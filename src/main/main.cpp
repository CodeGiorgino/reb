#include "main.hpp"
#include "../config/config.hpp"
#include "../context/context.hpp"
#include "../lib/utils.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
namespace fs = std::filesystem;

auto Init(char** argv) -> void {
    if (!*++argv)
        REB_PANIC("Unexpected end of command: expected mode or section name")

    reb::context::Context.Flag = std::string(*argv).starts_with("--") ? *argv++ : "--default";
    if (reb::context::Context.Flag == "--default") {
        if (!*argv)
            REB_PANIC("Unexpected end of command: expected section name")

        reb::context::Context.Section = *argv;

        if (fs::is_directory("./.reb")) {
            REB_INFO("Removing existing reb configuration")
            if (!fs::remove_all("./.reb"))
                REB_PANIC("Cannot remove configuration folder: './.reb'")
        }

        if (!fs::create_directory("./.reb"))
            REB_PANIC("Cannot create folder: './.reb'")

        REB_INFO("Reading global configuration file")
        reb::config::ReadConfig();

        REB_INFO("Reading local configuration file")
        if (!fs::copy_file(reb::context::Context.ConfigFilePath, "./.reb/reb.config"))
            REB_PANIC("Cannot copy config file: '" + reb::context::Context.ConfigFilePath + "'")
        reb::context::Context.ConfigFilePath = "./.reb/reb.config";
        reb::config::ReadConfig();

        if (!fs::create_directory("./.reb/release"))
            REB_PANIC("Cannot create folder: './.reb/release'")

        const auto ignoreList = reb::config::GetIgnoreList();
        std::ofstream objectsFile("./.reb/objects", std::ios::app);
        if (!objectsFile.is_open())
            REB_PANIC("Cannot open objects file: './.reb/objects")

        for (const auto& fsEntry :
             fs::recursive_directory_iterator(fs::current_path())) {
            bool ignore = false;
            for (const auto& toIngnore : ignoreList) {
                if (fsEntry.path().string().starts_with(fs::current_path().string() + "/" + toIngnore)) {
                    ignore = true;
                    break;
                }
            }

            if (ignore) continue;
            if (fs::is_regular_file(fsEntry)) {
                objectsFile << std::left << std::setw(20) << fs::hash_value(fsEntry) << " | " << fsEntry.path().string() << "\n";
            }
        }

        objectsFile.close();
    }

    else if (reb::context::Context.Flag == "--bare")
        REB_NOT_IMPLEMENTED("Init()::bare")

    else
        REB_PANIC(std::string("Invalid flag provided for command 'init': '" + reb::context::Context.Flag + "'"))
}

int main(int argc, char** argv) {
    (void)argc;

    reb::context::Context.ProgramName = *argv;
    if (!*++argv)
        REB_PANIC("Unexpected end of command: expected command name")

    reb::context::Context.Command = *argv;
    if (reb::context::Context.Command == "help")
        REB_NOT_IMPLEMENTED("Help()")
    else if (reb::context::Context.Command == "init")
        Init(argv);
    else if (reb::context::Context.Command == "run")
        REB_NOT_IMPLEMENTED("Run()")
    else if (reb::context::Context.Command == "snap")
        REB_NOT_IMPLEMENTED("Snap()")

    return 0;
}
