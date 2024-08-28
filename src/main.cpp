#include <bits/fs_fwd.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <optional>
#include <ostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "../deps/include/cppjson.hpp"
#include "../deps/include/parser.hpp"
#include "enviroment.hpp"
#include "utils.hpp"
namespace fs = std::filesystem;
using std::literals::string_literals::operator""s;
using enviroment = reb::enviroment::enviroment;

namespace file_ext {
auto checksum(const fs::path filepath) -> uint32_t {
    uint32_t checksum = 0;
    unsigned shift = 0;

    std::ifstream stream(filepath);
    if (!stream.is_open())
        REB_PANIC("cannot open file " << filepath.filename());

    for (uint32_t ch = stream.get(); stream; ch = stream.get()) {
        checksum += (ch << shift);
        shift += 8;

        if (shift == 32) shift = 0;
    }

    return checksum;
}
}  // namespace file_ext

auto regex_from_posix(const std::string &source) noexcept -> std::string {
    if (source.empty()) return source;
    auto retval = source;
    retval = std::regex_replace(retval, std::regex{"\\."}, "\\.");
    retval = std::regex_replace(retval, std::regex{"\\?"}, ".");
    retval = std::regex_replace(retval, std::regex{"\\*"}, ".*");
    return retval;
}

auto read_config(enviroment &env, bool from_model = true) -> void {
    /* reading the config file */
    const auto localConfig{fs::current_path() / ".reb"};
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository");

    std::ifstream stream(localConfig / "config.json");
    if (!stream.is_open()) REB_PANIC("cannot open config file");

    std::string configFile{}, line{};
    while (std::getline(stream, line)) {
        configFile += line;
    };

    stream.close();
    env.configJson = json::parser::deserialize(configFile);
    env.configJson["ignore"s] = json::array_t{};

    if (from_model) {
        /* reading the model */
        env.configJson = env.configJson[env.params];
        if (env.configJson.is_null()) REB_PANIC("the model is not defined");
    }

    /* reding the .rebignore file */
    env.configJson["ignore"s] = json::array_t{};
    if (fs::exists(fs::current_path() / ".rebignore")) {
        stream.open(fs::current_path() / ".rebignore");
        if (!stream.is_open()) REB_PANIC("cannot open .rebignore file");

        while (std::getline(stream, line)) {
            if (line.empty()) continue;
            env.configJson["ignore"s]
                << json::json_node{regex_from_posix(line)};
        }

        stream.close();
    }

    env.configJson["ignore"s] << json::json_node{"\\.git/.*"s}
                              << json::json_node{"\\.reb/snap/.*"s}
                              << json::json_node{"\\.reb/hash"s};

    /* get the ignorelist */
    std::vector<std::string> ignoreList{};
    for (const auto &entry : (json::array_t)env.configJson["ignore"s])
        ignoreList.push_back((std::string)entry);

    /* excluding the sub repositories */
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (dirEntry.is_directory() && dirEntry.path().stem() == ".reb" &&
            dirEntry.path().parent_path() != fs::current_path() &&
            std::none_of(
                ignoreList.begin(), ignoreList.end(),
                [dirEntry](const std::string &ignorePath) -> bool {
                    return std::regex_search(
                        dirEntry.path().string(),
                        std::regex(
                            (fs::current_path() / ignorePath).string(),
                            std::regex::icase | std::regex_constants::grep));
                }))
            env.configJson["ignore"s] << json::json_node{
                (dirEntry.path().parent_path() / "/.*").string()};
    }
}

auto write_hash(enviroment &env) -> void {
    const auto localConfig = fs::current_path() / ".reb";
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository");
    if (fs::exists(localConfig / "hash") && !fs::remove(localConfig / "hash"))
        REB_PANIC("cannot delete hash file");

    std::ofstream stream(localConfig / "hash");
    if (!stream.is_open()) REB_PANIC("cannot open output hash file");

    /* get the ignorelist */
    std::vector<std::string> ignoreList{};
    for (const auto &entry : (json::array_t)env.configJson["ignore"s])
        ignoreList.push_back((std::string)entry);

    /* writing files' hash */
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
    if (!fs::exists(localConfig)) REB_PANIC("not a reb repository");
    if (!fs::exists(localConfig / "hash")) return {};

    std::ifstream stream(localConfig / "hash");
    if (!stream.is_open()) REB_PANIC("cannot open hash file");

    std::unordered_map<std::string, std::string> retval{};

    /* reading the hash file */
    auto linePos{0};
    std::string line{};
    while (std::getline(stream, line)) {
        linePos++;
        if (line.empty()) continue;

        std::smatch match{};
        if (!std::regex_search(line, match,
                               std::regex{"^(.{8})\\s:\\s(.+)$"})) {
            stream.close();
            REB_PANIC("invalid line in hash file at line " << linePos);
        }

        retval.emplace(match[2], match[1]);
    };

    return retval;
}

auto command_help(char **argv) -> void {
    if (*++argv) REB_ERROR("unexpected parameter provided");
    REB_USAGE();
}

auto command_init(enviroment &env, char **argv) -> void {
    if (!*++argv) REB_PANIC("unexpected end of command");
    env.params = *argv;

    /* search for the model */
    fs::path modelFilePath{};
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(env.configPath / "models"))
        if (dirEntry.is_regular_file() &&
            dirEntry.path().filename() == env.params + ".json") {
            modelFilePath = dirEntry.path();
            break;
        }

    if (modelFilePath.empty()) REB_PANIC("config model not found");

    /* check for old config */
    const auto localConfig{fs::current_path() / ".reb"};
    if (fs::exists(localConfig)) {
        REB_INFO("already a reb repository");

        char input{};
        while (true) {
            std::cout << "\033[1G\033[K"
                      << "Remove the old configuration? [Y/n]: ";
            std::cin.get(input);

            if (input == 'n') {
                REB_INFO("keeping the current configuration");
                return;
            } else if (input == 'Y')
                break;
        };

        fs::remove_all(localConfig);
    }

    /* make the repository folder tree */
    if (!fs::create_directory(localConfig))
        REB_PANIC("cannot create .reb folder");

    if (!fs::copy_file(modelFilePath, localConfig / "config.json"))
        REB_PANIC("cannot copy model");

    if (!fs::create_directory(localConfig / "snap"))
        REB_PANIC("cannot create snapshot folder");
}

auto command_compile(enviroment &env) -> void {
    /* reading the model compilation section */
    auto &section = env.configJson["compilation"s];
    if (section.is_null())
        REB_PANIC("missing field 'compilation' in config file");
    if (section["source"s].is_null())
        REB_PANIC("missing field 'source' in config file");
    if (section["dest"s].is_null())
        REB_PANIC("missing field 'dest' in config file");
    if (section["command"s].is_null())
        REB_PANIC("missing field 'command' in config file");

    if (const auto path = (std::string)section["dest"s];
        !fs::exists(path) && !fs::create_directory(path))
        REB_PANIC("cannot create destination folder");

    /* get the hash list */
    const auto hashList{get_hash()};

    /* get the ignorelist */
    std::vector<std::string> ignoreList{};
    for (const auto &entry : (json::array_t)env.configJson["ignore"s])
        ignoreList.push_back((std::string)entry);

    const auto source = regex_from_posix((std::string)section["source"s]);
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (!fs::is_regular_file(dirEntry.path()) ||
            !std::regex_search(
                dirEntry.path().string(),
                std::regex((fs::current_path() / source).string(),
                           std::regex::icase | std::regex_constants::grep)) ||
            std::any_of(
                ignoreList.begin(), ignoreList.end(),
                [dirEntry](const std::string &ignorePath) -> bool {
                    return std::regex_search(
                        dirEntry.path().string(),
                        std::regex(
                            (fs::current_path() / ignorePath).string(),
                            std::regex::icase | std::regex_constants::grep));
                }))
            continue;

        std::stringstream hash;
        hash << std::hex << std::uppercase << std::setw(8)
             << file_ext::checksum(dirEntry.path());

        /* check if the file was edited */
        if (const auto &record = hashList.find(dirEntry.path().string());
            record != hashList.end() && record->second == hash.str()) {
            REB_INFO("skipping file " << dirEntry.path().filename() << std::endl
                                      << "-- reason (not modified)");
            continue;
        }

        /* compose the compilation command */
        REB_INFO("processing file " << dirEntry.path().filename());
        auto command = (std::string)section["command"s];
        if (!section["flags"s].is_null())
            for (const auto &entry : (json::array_t)section["flags"s])
                command += " " + (std::string)entry;

        command +=
            " -o " +
            fs::path((std::string)section["dest"s] / dirEntry.path().stem())
                .string() +
            ".o " + dirEntry.path().string();

        /* execute the compilation command */
        if (system(command.c_str()) != 0)
            REB_PANIC("cannot execute command on file "
                      << dirEntry.path().filename().string() << std::endl
                      << "-- section: (compilation)" << std::endl
                      << "-- command: (" << command << ")");
    }
}

auto command_link(enviroment &env) -> void {
    /* reading the model linking section */
    auto &section = env.configJson["linking"s];
    if (section.is_null()) REB_PANIC("missing field 'linking' in config file");
    if (section["source"s].is_null())
        REB_PANIC("missing field 'source' in config file");
    if (section["dest"s].is_null())
        REB_PANIC("missing field 'dest' in config file");
    if (section["command"s].is_null())
        REB_PANIC("missing field 'command' in config file");
    if (section["target"s].is_null())
        REB_PANIC("missing field 'target' in config file");

    if (const auto path = (std::string)section["dest"s];
        fs::exists(path) && !fs::remove_all(path))
        REB_PANIC("cannot delete destination folder");
    if (const auto path = (std::string)section["dest"s];
        !fs::create_directory(path))
        REB_PANIC("cannot create destination folder");

    /* get the ignorelist */
    std::vector<std::string> ignoreList{};
    for (const auto &entry : (json::array_t)env.configJson["ignore"s])
        ignoreList.push_back((std::string)entry);

    /* compose the linking command */
    auto command = (std::string)section["command"s];
    if (!section["flags"s].is_null())
        for (const auto &entry : (json::array_t)section["flags"s])
            command += " " + (std::string)entry;

    command += " -o " + fs::path(fs::path((std::string)section["dest"s]) /
                                 fs::path((std::string)section["target"s]))
                            .string();

    const auto source = regex_from_posix((std::string)section["source"s]);
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (!fs::is_regular_file(dirEntry.path()) ||
            !std::regex_search(
                dirEntry.path().string(),
                std::regex((fs::current_path() / source).string(),
                           std::regex::icase | std::regex_constants::grep)))
            continue;

        command += " " + dirEntry.path().string();
    }

    if (!section["deps"s].is_null())
        for (const auto &entry : (json::array_t)section["deps"s])
            command += " " + (std::string)entry;

    /* execute the linking command */
    if (system(command.c_str()) != 0)
        REB_PANIC("cannot execute command"
                  << std::endl
                  << "-- section: (linking)" << std::endl
                  << "-- command: (" << command << ")");
}

auto command_post_compile(enviroment &env) -> void {
    /* reading the post compile field */
    auto &section = env.configJson["post compile"s];
    if (section.is_null()) return;

    /* execute the commands */
    for (const auto &entry : (json::array_t)section) {
        if (const auto command = (std::string)entry;
            system(command.c_str()) != 0)
            REB_PANIC("cannot execute command"
                      << std::endl
                      << "-- section: (post compile)" << std::endl
                      << "-- command: (" << command << ")");
    }
}

auto command_run(enviroment &env, char **argv) -> void {
    if (!*++argv) REB_PANIC("unexpected end of command");

    env.params = *argv;

    REB_INFO("reading config file");
    read_config(env);

    REB_INFO("building the project");
    command_compile(env);
    command_link(env);
    command_post_compile(env);

    REB_INFO("writing files' hash");
    write_hash(env);
}

auto command_clean(enviroment &env, char **argv) -> void {
    if (*++argv) REB_PANIC("unexpected parameter provided");

    REB_INFO("reading config file");
    read_config(env, false);

    /* remove the hash file */
    const auto localConfig{fs::current_path() / ".reb"};
    REB_INFO("cleaning the repository");
    if (fs::exists(localConfig / "hash") && !fs::remove(localConfig / "hash"))
        REB_PANIC("cannot delete hash file");

    /* remove the destination folders */
    for (auto &entry : (json::object_t)env.configJson) {
        if (entry.second.try_get_value<json::object_t>() == std::nullopt)
            continue;

        auto &section = entry.second["compilation"s];
        if (section.is_null())
            REB_PANIC("missing field 'compilation' in config file");
        if (section["dest"s].is_null())
            REB_PANIC("missing field 'dest' in config file");

        auto dest = (std::string)section["dest"s];
        if (fs::exists(dest) && !fs::remove_all(dest))
            REB_PANIC("cannot delete folder '" << dest << "'");

        section = entry.second["linking"s];
        if (section.is_null())
            REB_PANIC("missing field 'linking' in config file");
        if (section["dest"s].is_null())
            REB_PANIC("missing field 'dest' in config file");

        dest = (std::string)section["dest"s];
        if (fs::exists(dest) && !fs::remove_all(dest))
            REB_PANIC("cannot delete folder '" << dest << "'");
    }
}

auto command_snap(enviroment &env, char **argv) -> void {
    REB_INFO("reading config file");
    read_config(env, false);

    const auto localConfig{fs::current_path() / ".reb"};

    /* get the ignorelist */
    std::vector<std::string> ignoreList{};
    for (const auto &entry : (json::array_t)env.configJson["ignore"s])
        ignoreList.push_back((std::string)entry);

    if (const auto snapshotName = *++argv; snapshotName) {
        /* revert to snapshot */
        if (const auto snapshotPath =
                fs::path{localConfig / "snap" / snapshotName};
            !fs::exists(snapshotPath) ||
            snapshotPath.parent_path().stem() != "snap")
            REB_PANIC("cannot find snapshot '" << snapshotName << "'");

        char input{};
        while (true) {
            std::cout << "\033[1G\033[K"
                      << "Revert to the snapshot '" << snapshotName
                      << "' (this operation cannot be undone)? [Y/n]: ";
            std::cin.get(input);

            if (input == 'n') {
                REB_INFO("keeping the current repository state");
                return;
            } else if (input == 'Y')
                break;
        };

        /* keep ignored files because not part of the snapshot */
        for (const auto &dirEntry :
             fs::recursive_directory_iterator(fs::current_path())) {
            if (std::none_of(ignoreList.begin(), ignoreList.end(),
                             [dirEntry](const std::string &ignorePath) -> bool {
                                 return std::regex_search(
                                     dirEntry.path().string(),
                                     std::regex(
                                         ignorePath,
                                         std::regex::icase |
                                             std::regex_constants::grep));
                             })) {
                fs::remove_all(dirEntry);
            }
        }

        /* copy the snapshot recursively */
        fs::copy(localConfig / "snap" / snapshotName, fs::current_path(),
                 fs::copy_options::recursive);
    }

    else {
        /* make snapshot */
        if (!fs::exists(localConfig / "snap") &&
            !fs::create_directory(localConfig / "snap"))
            REB_PANIC("cannot create snapshot folder");

        /* date as snapshot name */
        const auto timeNow = std::time(nullptr);
        const auto *timeInfo = std::localtime(&timeNow);

        std::stringstream newSnapshotName{};
        newSnapshotName << (timeInfo->tm_year + 1900) << std::setfill('0')
                        << std::setw(2) << (timeInfo->tm_mon + 1)
                        << std::setfill('0') << std::setw(2)
                        << (timeInfo->tm_mday) << "-" << std::setfill('0')
                        << std::setw(2) << timeInfo->tm_hour
                        << std::setfill('0') << std::setw(2)
                        << timeInfo->tm_min;
        const auto snapshotPath = localConfig / "snap" / newSnapshotName.str();

        if (fs::exists(snapshotPath) && !fs::remove_all(snapshotPath))
            REB_PANIC("cannot delete snapshot '" << newSnapshotName.str()
                                                 << "'");
        if (!fs::create_directory(snapshotPath))
            REB_PANIC("cannot create snapshot '" << newSnapshotName.str()
                                                 << "'");

        /* copy files */
        for (const auto &dirEntry :
             fs::recursive_directory_iterator(fs::current_path())) {
            if (std::none_of(ignoreList.begin(), ignoreList.end(),
                             [dirEntry](const std::string &ignorePath) -> bool {
                                 return std::regex_search(
                                     dirEntry.path().string(),
                                     std::regex(
                                         ignorePath,
                                         std::regex::icase |
                                             std::regex_constants::grep));
                             })) {
                if (fs::is_directory(dirEntry.path()))
                    fs::create_directory(
                        snapshotPath /
                        fs::relative(dirEntry.path(), fs::current_path()));
                else
                    fs::copy_file(
                        dirEntry.path(),
                        snapshotPath /
                            fs::relative(dirEntry.path(), fs::current_path()));
            }
        }
    }
}

int main(int argc, char **argv) {
    (void)argc;

    enviroment env{};
    env.programName = *argv;
    if (!*++argv) REB_PANIC("unexpected end of command");

    env.command = *argv;
    if (env.command == "help")
        command_help(argv);
    else if (env.command == "init")
        command_init(env, argv);
    else if (env.command == "run")
        command_run(env, argv);
    else if (env.command == "clean")
        command_clean(env, argv);
    else if (env.command == "snap")
        command_snap(env, argv);

    REB_INFO("command completed");
    return 0;
}
