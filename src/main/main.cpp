#include "main.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <regex>

#include "../config/config.hpp"
#include "../context/context.hpp"
#include "../lib/utils.hpp"
namespace fs = std::filesystem;

auto WriteHashToFile() -> void {
    const auto localConfig = fs::current_path() / ".reb";
    if (!fs::exists(localConfig)) REB_PANIC("not reb initialised")

    const auto hashFile = localConfig / "hash";
    if (fs::exists(hashFile)) fs::rename(hashFile, localConfig / "_hash");

    std::ofstream stream(hashFile, std::ios::binary);
    for (const auto &dirEntry :
         fs::recursive_directory_iterator(fs::current_path())) {
        bool skip = false;
        for (const auto &ignorePath : reb::config::GetIgnoreList()) {
            if (std::regex_search(dirEntry.path().string().erase(0, fs::current_path().string().length()),
                                 std::regex(ignorePath, std::regex::grep))) {
                skip = true;
                break;
            }
        }

        if (skip) continue;

        stream << std::hex << std::uppercase << std::setw(16)
               << fs::hash_value(dirEntry.path()) << " : " << dirEntry.path()
               << std::endl;
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

    if (modelFilePath.empty()) REB_PANIC("model not found")

    const auto localConfig = fs::current_path() / ".reb";
    if (fs::exists(localConfig)) fs::remove_all(localConfig);

    if (!fs::create_directory(localConfig))
        REB_PANIC("cannot create .reb folder")

    if (!fs::copy_file(modelFilePath, fs::current_path() / ".reb/reb.config"))
        REB_PANIC("cannot copy model")

    if (!fs::create_directory(localConfig / "snap"))
        REB_PANIC("cannot create snapshot folder")

    REB_INFO("writing files' hash");
    WriteHashToFile();
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
