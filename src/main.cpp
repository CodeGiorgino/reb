#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "config/config.hpp"
#include "context/context.hpp"
#include "lib/utils.hpp"
namespace fs = std::filesystem;

static reb::context::Context context{};

auto _readConfig() -> void {
    std::ifstream stream(fs::current_path() / ".reb/reb.config");
    if (!stream.is_open()) REB_PANIC("cannot open config file");

    std::string line;
    bool inSection = false;
    while (std::getline(stream, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::smatch match;
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
                REB_PANIC("unknown config value found in config file")
            }

            context.Config[std::to_underlying(configValue.value())] =
                match.str(2);
            continue;
        }

        else {
            stream.close();
            REB_PANIC("cannot parse config file")
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

    const auto hashFile = localConfig / "hash";
    if (fs::exists(hashFile)) fs::rename(hashFile, localConfig / "_hash");

    std::ofstream stream(hashFile);
    if (!stream.is_open()) REB_PANIC("cannot open output hash file")

    auto ignoreList = _getIgnoreList();
    for (const auto &ignorePath : _getSubRepositoriesPath())
        ignoreList.push_back(ignorePath);

    for (const auto &ignorePattern : ignoreList) REB_INFO(ignorePattern)

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

        if (ignore) continue;

        stream << std::hex << std::uppercase << std::setw(16)
               << fs::hash_value(dirEntry.path()) << " : "
               << dirEntry.path().string() << std::endl;
    }

    stream.close();
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
                REB_PANIC("cannot remove the old configuration")
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

    REB_INFO("writing files' hash")
    _writeHashToFile();

    REB_INFO("building the project")
    REB_NOT_IMPLEMENTED("Run()::build")
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
    else if (context.Command == "snap")
        REB_NOT_IMPLEMENTED("Snap()")

    REB_INFO("command completed")
    return 0;
}
