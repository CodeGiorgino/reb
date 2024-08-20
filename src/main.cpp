#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <istream>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../deps/include/cppjson.hpp"
#include "../deps/include/parser.hpp"
#include "context/context.hpp"
#include "lib/utils.hpp"
namespace fs = std::filesystem;
using std::literals::string_literals::operator""s;

static reb::context::Context context{};

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

auto get_subrepos_path() -> std::vector<std::string> {
    std::vector<std::string> retval{};

    return retval;
}

auto regex_from_posix(const std::string &source) noexcept -> std::string {
    if (source.empty()) return source;
    auto retval = source;
    retval = std::regex_replace(retval, std::regex{"\\."}, "\\.");
    retval = std::regex_replace(retval, std::regex{"\\?"}, ".");
    retval = std::regex_replace(retval, std::regex{"\\*"}, ".*");
    return retval;
}

auto read_config(bool from_model = true) -> void {
    const auto localConfig{fs::current_path() / ".reb"};
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository")

    std::ifstream stream(localConfig / "config.json");
    if (!stream.is_open()) REB_PANIC("cannot open config file");

    std::string configFile{}, line{};
    while (std::getline(stream, line)) {
        configFile += line;
    };

    stream.close();
    context.Config = json::parser::deserialize(configFile);
    if (!from_model) return;

    context.Config = context.Config[context.Params];
    if (context.Config.is_null()) REB_PANIC("the model is not defined")

    if (!fs::exists(fs::current_path() / ".rebignore")) return;
    stream.open(fs::current_path() / ".rebignore");
    if (!stream.is_open()) REB_PANIC("cannot open .rebignore file")

    context.Config["ignore"s] = json::array_t{};
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        context.Config["ignore"s] << json::json_node{regex_from_posix(line)};
    }

    stream.close();
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (dirEntry.is_directory() && dirEntry.path().stem() == ".reb" &&
            dirEntry.path().parent_path() != fs::current_path())
            context.Config["ignore"s]
                << json::json_node{dirEntry.path().parent_path() / "/*"};
    }
}

auto write_hash() -> void {
    const auto localConfig = fs::current_path() / ".reb";
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository")
    if (fs::exists(localConfig / "hash") && !fs::remove(localConfig / "hash"))
        REB_PANIC("cannot delete hash file")

    std::ofstream stream(localConfig / "hash");
    if (!stream.is_open()) REB_PANIC("cannot open output hash file")

    std::vector<std::string> ignoreList{};
    if (!context.Config["ignore"s].is_null())
        for (const auto &entry : (json::array_t)context.Config["ignore"s])
            ignoreList.push_back((std::string)entry);

    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (std::any_of(ignoreList.begin(), ignoreList.end(),
                        [dirEntry](const std::string &ignorePath) {
                            return std::regex_search(
                                dirEntry.path().string(),
                                std::regex(ignorePath,
                                           std::regex_constants::icase |
                                               std::regex_constants::grep));
                        }))
            continue;

        const auto hash = file_ext::checksum(dirEntry.path());
        if (hash == 0) continue;

        stream << std::hex << std::uppercase << std::setw(8) << hash << " : "
               << dirEntry.path().string() << std::endl;
    }

    stream.close();
}

auto get_hash() -> std::unordered_map<std::string, std::string> {
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
                               std::regex{"^(.{8})\\s:\\s(.+)$"})) {
            stream.close();
            REB_PANIC("invalid line in hash file at line " << linePos)
        }

        retval.emplace(match[2], match[1]);
    };

    return retval;
}

auto command_help(char **argv) -> void {
    if (*++argv) REB_ERROR("unexpected parameter provided")
    REB_USAGE
}

auto command_init(char **argv) -> void {
    if (!*++argv) REB_PANIC("unexpected end of command")
    context.Params = *argv;

    fs::path modelFilePath{};
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(context.ConfigPath / "models"))
        if (const auto fileName{dirEntry.path().string()};
            !dirEntry.is_directory() &&
            fileName.ends_with(context.Params + ".json")) {
            modelFilePath = dirEntry.path();
            break;
        }

    if (modelFilePath.empty()) REB_PANIC("config model not found")

    const auto localConfig{fs::current_path() / ".reb"};
    if (fs::exists(localConfig)) {
        REB_INFO("already a reb repository")

        char input{};
        while (true) {
            std::cout << "\033[1G\033[K"
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

    if (!fs::copy_file(modelFilePath, localConfig / "config.json"))
        REB_PANIC("cannot copy model")

    if (!fs::create_directory(localConfig / "snap"))
        REB_PANIC("cannot create snapshot folder")
}

auto command_compile() -> void {
    auto &section = context.Config["compilation"s];
    if (section.is_null())
        REB_PANIC("missing field 'compilation' in config file")
    if (section["source"s].is_null())
        REB_PANIC("missing field 'source' in config file")
    if (section["dest"s].is_null())
        REB_PANIC("missing field 'dest' in config file")
    if (section["command"s].is_null())
        REB_PANIC("missing field 'command' in config file")

    if (const auto path = (std::string)section["dest"s];
        !fs::exists(path) && !fs::create_directory(path))
        REB_PANIC("cannot create destination folder")

    const auto hashList{get_hash()};
    std::vector<std::string> ignoreList{};
    if (!context.Config["ignore"s].is_null())
        for (const auto &entry : (json::array_t)context.Config["ignore"s])
            ignoreList.push_back((std::string)entry);

    const auto source = regex_from_posix((std::string)section["source"s]);
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (fs::is_directory(dirEntry) ||
            !std::regex_search(
                dirEntry.path().string(),
                std::regex(source,
                           std::regex::icase | std::regex_constants::grep)) ||
            std::any_of(ignoreList.begin(), ignoreList.end(),
                        [dirEntry](const std::string &ignorePath) -> bool {
                            return std::regex_search(
                                dirEntry.path().string(),
                                std::regex(ignorePath,
                                           std::regex::icase |
                                               std::regex_constants::grep));
                        }))
            continue;

        std::stringstream hash;
        hash << std::hex << std::uppercase << std::setw(8)
             << file_ext::checksum(dirEntry.path());

        if (const auto &record = hashList.find(dirEntry.path().string());
            record != hashList.end() && record->second == hash.str()) {
            REB_INFO("skipping file " << dirEntry.path().filename() << std::endl
                                      << "-- reason (not modified)")
            continue;
        }

        REB_INFO("processing file " << dirEntry.path().filename())
        auto command = (std::string)section["command"s];
        if (!section["flags"s].is_null())
            for (const auto &entry : (json::array_t)section["flags"s])
                command += " " + (std::string)entry;

        command +=
            " -o " +
            fs::path((std::string)section["dest"s] / dirEntry.path().stem())
                .string() +
            ".o " + dirEntry.path().string();
        if (system(command.c_str()) != 0)
            REB_PANIC("cannot execute command on file "
                      << dirEntry.path().filename().string() << std::endl
                      << "-- section: (compilation)" << std::endl
                      << "-- command: (" << command << ")")
    }
}

auto command_link() -> void {
    auto &section = context.Config["linking"s];
    if (section.is_null()) REB_PANIC("missing field 'linking' in config file")
    if (section["source"s].is_null())
        REB_PANIC("missing field 'source' in config file")
    if (section["dest"s].is_null())
        REB_PANIC("missing field 'dest' in config file")
    if (section["command"s].is_null())
        REB_PANIC("missing field 'command' in config file")
    if (section["target"s].is_null())
        REB_PANIC("missing field 'target' in config file")

    if (const auto path = (std::string)section["dest"s];
        fs::exists(path) && !fs::remove_all(path))
        REB_PANIC("cannot delete destination folder")
    if (const auto path = (std::string)section["dest"s];
        !fs::create_directory(path))
        REB_PANIC("cannot create destination folder")

    std::vector<std::string> ignoreList{};
    if (!context.Config["ignore"s].is_null())
        for (const auto &entry : (json::array_t)context.Config["ignore"s])
            ignoreList.push_back((std::string)entry);

    auto command = (std::string)section["command"s];
    if (!section["flags"s].is_null())
        for (const auto &entry : (json::array_t)section["flags"s])
            command += " " + (std::string)entry;

    command += " -o " + fs::path(fs::path((std::string)section["dest"s]) /
                                 fs::path((std::string)section["target"s]))
                            .string();

    const auto source = regex_from_posix((std::string)section["source"s]);
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path()))
        if (!dirEntry.is_directory() &&
            std::regex_search(
                dirEntry.path().string(),
                std::regex{source, std::regex_constants::icase |
                                       std::regex_constants::grep}))
            command += " " + dirEntry.path().string();

    if (!section["deps"s].is_null())
        for (const auto &entry : (json::array_t)section["deps"s])
            command += " " + (std::string)entry;

    if (system(command.c_str()) != 0)
        REB_PANIC("cannot execute command" << std::endl
                                           << "-- section: (linking)"
                                           << std::endl
                                           << "-- command: (" << command << ")")
}

auto command_post_compile() -> void {
    auto &section = context.Config["post compile"s];
    if (section.is_null()) return;

    for (const auto &entry : (json::array_t)section) {
        if (const auto command = (std::string)entry;
            system(command.c_str()) != 0)
            REB_PANIC("cannot execute command"
                      << std::endl
                      << "-- section: (post compile)" << std::endl
                      << "-- command: (" << command << ")")
    }
}

auto command_run(char **argv) -> void {
    if (!*++argv) REB_PANIC("unexpected end of command")

    context.Params = *argv;

    REB_INFO("reading config file")
    read_config();

    REB_INFO("building the project")
    command_compile();
    command_link();
    command_post_compile();

    REB_INFO("writing files' hash")
    write_hash();
}

auto command_clean(char **argv) -> void {
    if (*++argv) REB_PANIC("unexpected parameter provided")

    REB_INFO("reading config file")
    read_config(false);

    const auto localConfig{fs::current_path() / ".reb"};
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository")

    REB_INFO("cleaning the repository")
    if (fs::exists(localConfig / "hash") && !fs::remove(localConfig / "hash"))
        REB_PANIC("cannot delete hash file")

    for (auto &entry : (json::object_t)context.Config) {
        auto &section = entry.second["compilation"s];
        if (section.is_null())
            REB_PANIC("missing field 'compilation' in config file")
        if (section["dest"s].is_null())
            REB_PANIC("missing field 'dest' in config file")

        auto dest = (std::string)section["dest"s];
        if (fs::exists(dest) && !fs::remove_all(dest))
            REB_PANIC("cannot delete folder '" << dest << "'")

        section = entry.second["linking"s];
        if (section.is_null())
            REB_PANIC("missing field 'linking' in config file")
        if (section["dest"s].is_null())
            REB_PANIC("missing field 'dest' in config file")

        dest = (std::string)section["dest"s];
        if (fs::exists(dest) && !fs::remove_all(dest))
            REB_PANIC("cannot delete folder '" << dest << "'")
    }
}

int main(int argc, char **argv) {
    (void)argc;

    context.ProgramName = *argv;
    if (!*++argv) REB_PANIC("unexpected end of command")

    context.Command = *argv;
    if (context.Command == "help")
        command_help(argv);
    else if (context.Command == "init")
        command_init(argv);
    else if (context.Command == "run")
        command_run(argv);
    else if (context.Command == "clean")
        command_clean(argv);
    else if (context.Command == "snap")
        REB_NOT_IMPLEMENTED("Snap()")

    REB_INFO("command completed")
    return 0;
}
