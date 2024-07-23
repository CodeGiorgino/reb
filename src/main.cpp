#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "config/config.hpp"
#include "context/context.hpp"
#include "lib/utils.hpp"
namespace fs = std::filesystem;

static reb::context::Context context{};

namespace bool_ext {
auto from_string(const std::string value) noexcept -> std::optional<bool> {
    if (value == "true")
        return true;
    else if (value == "false")
        return false;
    else
        return {};
}
}  // namespace bool_ext

namespace file_ext {
auto checksum(const fs::path filepath) -> uint32_t {
    uint32_t checksum = 0;
    unsigned shift = 0;

    std::ifstream stream(filepath);
    if (!stream.is_open()) REB_PANIC("cannot open file " << filepath.filename())

    for (uint32_t ch = stream.get(); stream; ch = stream.get()) {
        checksum += (ch << shift);
        shift += 8;

        if (shift == 32) shift = 0;
    }

    return checksum;
}
}  // namespace file_ext

auto _readConfig() -> void {
    const auto localConfig{fs::current_path() / ".reb"};
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository")

    std::ifstream stream(localConfig / "reb.config");
    if (!stream.is_open()) REB_PANIC("cannot open config file");

    size_t linePos{0};
    std::string line;
    bool inSection = false;
    while (std::getline(stream, line)) {
        linePos++;
        if (line.empty() || line[0] == '#') continue;

        std::smatch match{};
        if (std::regex_search(line, match, std::regex("^(.*):\\s*$"))) {
            inSection = match.str(1) == context.Params;
            continue;
        }

        else if (std::regex_search(
                     line, match,
                     std::regex("^\\s*(\\w*)\\s*=\\s*\"(.*)\"\\s*$"))) {
            if (!inSection) continue;

            const auto configValue =
                enum_ext::from_string<reb::config::ConfigValue>(match.str(1));

            if (!configValue.has_value()) {
                stream.close();
                REB_PANIC("unknown config value found in config file at line "
                          << linePos)
            }

            context.Config[std::to_underlying(configValue.value())] =
                match.str(2);
            continue;
        }

        else {
            stream.close();
            REB_PANIC("invalid line found in config file at line " << linePos)
        }
    };

    stream.close();

    bool isValid = false;
    for (const auto &configValue : context.Config)
        if (!configValue.empty()) {
            isValid = true;
            break;
        }

    if (!isValid) REB_PANIC("cannot find section in config file")

    context.Config[std::to_underlying(reb::config::ConfigValue::IGNORE)] =
        "\\.reb/.*;";

    if (!fs::exists(fs::current_path() / ".rebignore")) return;
    stream.open(fs::current_path() / ".rebignore");
    if (!stream.is_open()) REB_PANIC("cannot open .rebignore file")

    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        std::string wildcardPattern{line};
        wildcardPattern =
            std::regex_replace(wildcardPattern, std::regex("\\."), "\\.");
        wildcardPattern =
            std::regex_replace(wildcardPattern, std::regex("\\*"), ".*");
        wildcardPattern =
            std::regex_replace(wildcardPattern, std::regex("\\?"), ".");

        context.Config[std::to_underlying(reb::config::ConfigValue::IGNORE)] +=
            wildcardPattern + ";";
    }

    stream.close();
}

auto _getIgnoreList() -> std::vector<std::string> {
    auto ignoreList =
        context.Config[std::to_underlying(reb::config::ConfigValue::IGNORE)];
    std::vector<std::string> retval{};

    size_t pos;
    while ((pos = ignoreList.find(';')) != std::string::npos) {
        const auto part = ignoreList.substr(0, pos);
        ignoreList.erase(ignoreList.begin(), ignoreList.begin() + pos + 1);
        retval.push_back(fs::current_path() / part);
    };
    return retval;
}

auto _getSubRepositoriesPath() -> std::vector<std::string> {
    std::vector<std::string> retval{};
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (dirEntry.is_directory() && dirEntry.path().stem() == ".reb" &&
            dirEntry.path().parent_path() != fs::current_path())
            retval.push_back(dirEntry.path().parent_path() / "/.*");
    }

    return retval;
}

auto _writeHashToFile() -> void {
    const auto localConfig = fs::current_path() / ".reb";
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository")

    if (fs::exists(localConfig / "hash") && !fs::remove(localConfig / "hash"))
        REB_PANIC("cannot delete hash file")

    std::ofstream stream(localConfig / "hash");
    if (!stream.is_open()) REB_PANIC("cannot open output hash file")

    auto ignoreList = _getIgnoreList();
    for (const auto &ignorePath : _getSubRepositoriesPath())
        ignoreList.push_back(ignorePath);

    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        bool ignore = false;
        for (const auto &ignorePattern : ignoreList)
            if (std::regex_match(
                    dirEntry.path().string(),
                    std::regex(ignorePattern, std::regex::icase))) {
                ignore = true;
                break;
            }

        if (ignore ||
            !std::regex_match(dirEntry.path().extension().string(),
                              std::regex(context.Config[std::to_underlying(
                                             reb::config::ConfigValue::EXT)],
                                         std::regex::icase)))
            continue;

        const auto hash = file_ext::checksum(dirEntry.path());
        if (hash == 0) continue;

        stream << std::hex << std::uppercase << std::setw(8) << hash << " : "
               << dirEntry.path().string() << std::endl;
    }

    stream.close();
}

auto _getHashFromFile() -> std::unordered_map<std::string, std::string> {
    const auto localConfig{fs::current_path() / ".reb"};
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository")

    if (!fs::exists(localConfig / "hash")) return {};

    std::ifstream stream(localConfig / "hash");
    if (!stream.is_open()) REB_PANIC("cannot open hash file")

    std::unordered_map<std::string, std::string> retval{};

    auto linePos{0};
    std::string line{};
    while (std::getline(stream, line)) {
        linePos++;
        if (line.empty()) continue;

        std::smatch match{};
        if (!std::regex_search(line, match,
                               std::regex("^(.{8})\\s:\\s(.+)$"))) {
            stream.close();
            REB_PANIC("invalid line in hash file at line " << linePos)
        }

        retval.emplace(match[1], match[0]);
    };

    return retval;
}

auto Help(char **argv) -> void {
    if (*++argv) REB_ERROR("unexpected parameter provided")
    REB_USAGE
}

auto Init(char **argv) -> void {
    if (!*++argv) REB_PANIC("unexpected end of command")
    context.Params = *argv;

    fs::path modelFilePath{};
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(context.ConfigPath / "models"))
        if (const auto fileName{dirEntry.path().stem()};
            dirEntry.is_regular_file() && fileName == context.Params)
            modelFilePath = dirEntry.path();

    if (modelFilePath.empty()) REB_PANIC("config model not found")

    const auto localConfig{fs::current_path() / ".reb"};
    if (fs::exists(localConfig)) {
        REB_INFO("already a reb repository")

        char input{};
        while (true) {
            std::cout << "\33[2K\r"
                      << "Remove the old configuration? [Y/n]: ";
            std::cin.get(input);

            if (input == 'n')
                REB_PANIC("cannot delete the old configuration")
            else if (input == 'Y')
                break;
        };

        fs::remove_all(localConfig);
    }

    if (!fs::create_directory(localConfig))
        REB_PANIC("cannot create .reb folder")

    if (!fs::copy_file(modelFilePath, fs::current_path() / ".reb/reb.config"))
        REB_PANIC("cannot copy model")

    if (!fs::create_directory(localConfig / "snap"))
        REB_PANIC("cannot create snapshot folder")
}

auto Run(char **argv) -> void {
    if (!**++argv) REB_PANIC("unexpected end of command")

    context.Params = *argv;

    REB_INFO("reading config file")
    _readConfig();

    REB_INFO("building the project")

    const auto &fileExtension{
        context.Config[std::to_underlying(reb::config::ConfigValue::EXT)]};
    const auto &buildName{
        context.Config[std::to_underlying(reb::config::ConfigValue::EXT)]};
    const auto &compiler{
        context.Config[std::to_underlying(reb::config::ConfigValue::COMP)]};
    const auto &compilerFlags{
        context.Config[std::to_underlying(reb::config::ConfigValue::FLAGS)]};
    const auto sourcePath{fs::path(
        context.Config[std::to_underlying(reb::config::ConfigValue::SOURCE)])};
    const auto objectPath{fs::current_path() / "obj"};
    const auto buildPath{fs::path(
        context.Config[std::to_underlying(reb::config::ConfigValue::BUILD)])};
    const auto autoRun{
        bool_ext::from_string(context.Config[std::to_underlying(
                                  reb::config::ConfigValue::AUTO_RUN)])
            .value_or(true)};

    if (!fs::exists(sourcePath) || !fs::is_directory(sourcePath))
        REB_PANIC("cannot find source folder")

    if (!fs::exists(buildPath) && !fs::create_directory(buildPath))
        REB_PANIC("cannot create build folder")

    if (!fs::exists(objectPath) && !fs::create_directory(objectPath))
        REB_PANIC("cannot create object folder")

    auto returnCode{0};
    const auto hashList{_getHashFromFile()};

    auto ignoreList = _getIgnoreList();
    for (const auto &ignorePath : _getSubRepositoriesPath())
        ignoreList.push_back(ignorePath);

    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        bool ignore = false;
        for (const auto &ignorePattern : ignoreList)
            if (!fs::is_regular_file(dirEntry) ||
                !std::regex_match(
                    dirEntry.path().extension().string(),
                    std::regex(fileExtension, std::regex::icase)) ||
                std::regex_match(
                    dirEntry.path().string(),
                    std::regex(ignorePattern, std::regex::icase))) {
                ignore = true;
                break;
            }

        if (ignore) continue;

        std::stringstream hash;
        hash << std::hex << std::uppercase << std::setw(8)
             << file_ext::checksum(dirEntry.path());

        if (const auto &record = hashList.find(dirEntry.path().string());
            record != hashList.end() && record->second == hash.str()) {
            REB_INFO("skipping file " << dirEntry.path().filename())
            continue;
        }

        REB_INFO("compiling file " << dirEntry.path().filename())
        returnCode = system(std::string(compiler + " " + compilerFlags +
                                        " -c -o " + objectPath.string() + "/" +
                                        dirEntry.path().stem().string() +
                                        ".o " + dirEntry.path().string())
                                .c_str());

        if (returnCode != 0)
            REB_PANIC("cannot compile file " +
                      dirEntry.path().filename().string())
    }

    std::string objectList{};
    for (const auto &dirEntry : fs::directory_iterator(objectPath))
        if (fs::is_regular_file(dirEntry.path()) &&
            std::regex_match(dirEntry.path().string(),
                             std::regex("^.*\\.o$", std::regex::icase)))
            objectList += dirEntry.path().string() + " ";

    returnCode =
        system(std::string(compiler + " " + compilerFlags + " -o " +
                           (buildPath / buildName).string() + " " + objectList)
                   .c_str());

    if (returnCode != 0) REB_PANIC("cannot build project")

    REB_INFO("writing files' hash")
    _writeHashToFile();

    if (!autoRun) return;

    REB_INFO("executing project" << std::endl)
    char *nullArgs[] = {NULL};
    if (execvp((buildPath / buildName).string().c_str(), nullArgs) != 0)
        REB_PANIC("project execution ended with non zero exit code")
}

auto Clean(char **argv) -> void {
    if (*++argv) REB_PANIC("unexpected parameter provided")

    const auto localConfig{fs::current_path() / ".reb"};
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository")

    REB_INFO("cleaning the repository")
    fs::remove(localConfig / "hash");
    fs::remove_all(fs::current_path() / "build");
    fs::remove_all(fs::current_path() / "obj");
}

int main(int argc, char **argv) {
    (void)argc;

    context.ProgramName = *argv;
    if (!*++argv) REB_PANIC("unexpected end of command")

    context.Command = *argv;
    if (context.Command == "help")
        Help(argv);
    else if (context.Command == "init")
        Init(argv);
    else if (context.Command == "run")
        Run(argv);
    else if (context.Command == "clean")
        Clean(argv);
    else if (context.Command == "snap")
        REB_NOT_IMPLEMENTED("Snap()")

    REB_INFO("command completed")
    return 0;
}
