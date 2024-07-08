#include "main.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <vector>

#include "../config/config.hpp"
#include "../context/context.hpp"
#include "../lib/utils.hpp"
namespace fs = std::filesystem;

auto _getSubRepositoriesPath() -> std::vector<fs::path> {
    std::vector<fs::path> retval{};
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (dirEntry.is_directory() && dirEntry.path().stem() == ".reb" &&
            dirEntry.path().parent_path() != fs::current_path())
            retval.push_back(dirEntry.path().parent_path());
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

    auto ignoreList = reb::config::GetIgnoreList();
    for (const auto &ignorePath : _getSubRepositoriesPath())
        ignoreList.push_back(ignorePath);

    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        if (std::find(ignoreList.begin(), ignoreList.end(),
                      dirEntry.path().parent_path()) != ignoreList.end() ||
            std::find(ignoreList.begin(), ignoreList.end(), dirEntry.path()) !=
                ignoreList.end())
            continue;

        stream << std::hex << std::uppercase << std::setw(16)
               << fs::hash_value(dirEntry.path()) << " : "
               << dirEntry.path().string() << std::endl;
    }

    stream.close();
}

auto Init(char **argv) -> void {
    if (!*++argv) REB_PANIC("unexpected end of command")

    reb::context::Context.Params = *argv;

    fs::path modelFilePath{};
    for (const auto &dirEntry : fs::recursive_directory_iterator(
             reb::context::Context.ConfigPath / "models"))
        if (const auto fileName{dirEntry.path().stem()};
            dirEntry.is_regular_file() &&
            fileName == reb::context::Context.Params)
            modelFilePath = dirEntry.path();

    if (modelFilePath.empty()) REB_PANIC("config model not found")

    const auto localConfig = fs::current_path() / ".reb";
    if (fs::exists(localConfig)) {
        REB_INFO("already a reb repository")

        char input;
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

    REB_INFO("writing files' hash");
    _writeHashToFile();
}

int main(int argc, char **argv) {
    (void)argc;

    reb::context::Context.ProgramName = *argv;
    if (!*++argv) REB_PANIC("unexpected end of command")

    reb::context::Context.Command = *argv;
    if (reb::context::Context.Command == "help")
        REB_NOT_IMPLEMENTED("Help()")
    else if (reb::context::Context.Command == "init")
        Init(argv);
    else if (reb::context::Context.Command == "run")
        REB_NOT_IMPLEMENTED("Run()")
    else if (reb::context::Context.Command == "snap")
        REB_NOT_IMPLEMENTED("Snap()")

    REB_INFO("command completed")
    return 0;
}
