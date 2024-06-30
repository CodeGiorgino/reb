#include "main.hpp"

auto reb::GetConfigValueFromName(std::string name) -> ConfigValue {
    if (name == "GXX")
        return GXX;
    else if (name == "CXX")
        return CXX;
    else if (name == "CC")
        return CC;
    else if (name == "SOURCE")
        return SOURCE;
    else if (name == "OBJ")
        return OBJ;
    else if (name == "BUILD")
        return BUILD;
    else if (name == "KEEP_OBJ")
        return KEEP_OBJ;
    else if (name == "AUTO_RUN")
        return AUTO_RUN;
    else if (name == "IGNORE")
        return IGNORE;

    REB_PANIC("Config value name not defined: '" + name + "'");
}

auto reb::ReadConfig() -> void {
    std::ifstream configFile(Context.ConfigFilePath);
    if (!configFile.is_open())
        REB_PANIC("Cannot open config file: '" + Context.ConfigFilePath + "'")

    std::string line;
    while (getline(configFile, line)) {
        if (line.empty() || line.starts_with("#"))
            continue;
        else if (line.starts_with("- ") && line.substr(2) == (Context.Section + ":")) {
            while (getline(configFile, line)) {
                if (line.empty() || line.starts_with("#"))
                    continue;
                else if (line.starts_with("- "))
                    break;
                else if (!line.starts_with("    - "))
                    REB_PANIC("Invalid line in config file section '" + Context.Section + "': '" + line + "'")

                if (line.find('"') == std::string::npos ||
                    line.find_first_of('"') == line.find_last_of("'") ||
                    !line.ends_with('"'))
                    REB_PANIC("Invalid line in config file section '" + Context.Section + "': '" + line + "'")

                auto configValueName = line.substr(6, line.find_first_of('=') - 6);
                configValueName.erase(
                    configValueName.begin() + configValueName.find_first_of(' '),
                    configValueName.end());

                Context.Config[GetConfigValueFromName(configValueName)] =
                    line.substr(line.find_first_of('"') + 1);
                Context.Config[GetConfigValueFromName(configValueName)]
                    .erase(Context.Config[GetConfigValueFromName(configValueName)].length() - 1);
            };
        }
    };

    configFile.close();

    if (Context.Config[ConfigValue::GXX].empty())
        REB_PANIC("Unexpected section provided: '" + Context.Section + "'")
}

auto reb::GetIgnoreList() -> std::vector<std::string> {
    std::vector<std::string> ignoreList = {".reb", ""};
    for (const auto& ch : Context.Config[ConfigValue::IGNORE])
        if (ch == ' ')
            ignoreList.push_back("");
        else
            ignoreList[ignoreList.size() - 1] += ch;

    return ignoreList;
}

auto reb::Init(char** argv) -> void {
    if (!*++argv)
        REB_PANIC("Unexpected end of command: expected mode or section name")

    Context.Flag = std::string(*argv).starts_with("--") ? *argv++ : "--default"s;
    if (Context.Flag == "--default") {
        if (!*argv)
            REB_PANIC("Unexpected end of command: expected section name")

        Context.Section = *argv;

        if (fs::is_directory("./.reb")) {
            REB_INFO("Removing existing reb configuration")
            if (!fs::remove_all("./.reb"))
                REB_PANIC("Cannot remove configuration folder: './.reb'")
        }

        if (!fs::create_directory("./.reb"))
            REB_PANIC("Cannot create folder: './.reb'")

        REB_INFO("Reading global configuration file")
        ReadConfig();

        REB_INFO("Reading local configuration file")
        if (!fs::copy_file(Context.ConfigFilePath, "./.reb/reb.config"))
            REB_PANIC("Cannot copy config file: '" + Context.ConfigFilePath + "'")
        Context.ConfigFilePath = "./.reb/reb.config";
        ReadConfig();

        if (!fs::create_directory("./.reb/release"))
            REB_PANIC("Cannot create folder: './.reb/release'")

        const auto ignoreList = GetIgnoreList();
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

    else if (reb::Context.Flag == "--bare")
        REB_NOT_IMPLEMENTED("Init()::bare")

    else
        REB_PANIC(std::string("Invalid flag provided for command 'init': '" + reb::Context.Flag + "'"))
}

int main(int argc, char** argv) {
    (void)argc;

    reb::Context.ProgramName = *argv + ""s;
    if (!*++argv)
        REB_PANIC("Unexpected end of command: expected command name")

    reb::Context.Command = *argv;
    if (reb::Context.Command == "help")
        REB_NOT_IMPLEMENTED("Help()")
    else if (reb::Context.Command == "init")
        reb::Init(argv);
    else if (reb::Context.Command == "run")
        REB_NOT_IMPLEMENTED("Run()")
    else if (reb::Context.Command == "release")
        REB_NOT_IMPLEMENTED("Release()")

    return 0;
}