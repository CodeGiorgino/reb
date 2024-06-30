Table of contents:
- [REB - Rebuild Command Line Utility](#reb---rebuild-command-line-utility)
  - [Configuration](#configuration)
    - [Global](#global)
    - [Local](#local)
    - [Parameters](#parameters)
  - [Usage](#usage)
    - [Commands](#commands)

<br>
<br>

# REB - Rebuild Command Line Utility
REB is a small command line build tool which you can use to build your project and keep track of releases.

## Configuration
Before configuring REB you need to include the executable in your lib folder .
```bash
./build.sh
ln -s ./build/reb /usr/lib/reb
```

### Global
To create a global configuration you just need to create the folder `~/.config/reb` and initialise 
the folder with `reb init --bare`. Now you can edit your global config file to match your preferences.

### Local
To create a local configuration you just need to cd into your project folder and initialise
the folder with `reb init [flag] <section>`. Now you can edit your local config file to match your preferences.
> **Note:** the local config file will be merged into the global one on initialisation, overwriting the global properties

### Parameters
| Name         | Value type  | Description                                                           |
| :----------- | :---------- | :-------------------------------------------------------------------- |
| **GXX**      | string      | Compiler                                                              |
| **CXX**      | string      | Compilation command                                                   |
| **CC**       | string      | Dependencies                                                          |
| **SOURCE**   | string      | Source folder                                                         |
| **OBJ**      | string      | Object output folder                                                  |
| **BUILD**    | string      | Build output folder                                                   |
| **KEEP_OBJ** | true\|false | Whether to keep or not the object files after compilation             |
| **AUTO_RUN** | true\|false | Whether to auto start the program after                   compilation |
| **IGNORE**   | string      | The list of folders and files to ignore                               |

## Usage
TODO

### Commands
| Name    | Description                               | Parameters                        |
| :------ | :---------------------------------------- | :-------------------------------- |
| help    | Display usage informations                | -                                 |
| init    | Initialise the project's folder           | \[--default\] \<section\>, --bare |
| run     | Run the compilation                       | \<section\>                       |
| release | Like run but also make a version snapshot | \<section\> \<version\>           |