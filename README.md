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
Before using REB, you need to execute `install.sh` to begin the installation

### Global
To create a global configuration you just need to edit the models in `~/.config/reb/models`
```
~
L .config
  L reb
    L models
      L model_name_1.config
      L model_name_2.config
      L ...
```

### Local
To create a local configuration you just need to cd into your project folder and initialise it
with `reb init <model_name>`. Now you can edit your model in `.reb/model_name.config`
```
/path/to/project
L .reb
  L model_name.config
```

> **Note:** the local config file will be merged into the global one on initialisation, overwriting the global properties

### Parameters
| Name         | Value type  | Description                                         |
| :----------- | :---------- | :-------------------------------------------------- |
| **COMP**     | string      | Compiler to use                                     |
| **FLAGS**    | string      | Compilation flags                                   |
| **SOURCE**   | string      | Source folder                                       |
| **BUILD**    | string      | Build output folder                                 |
| **AUTO_RUN** | true\|false | Whether to auto start the program after compilation |
| **IGNORE**   | string      | The list of folders and files to ignore             |

## Usage
TODO

### Commands
| Name    | Description                      | Parameters        |
| :------ | :------------------------------- | :---------------- |
| help    | Display usage informations       | -                 |
| init    | Initialise the project's folder  | `<model_name>`    |
| run     | Run the compilation              | `<section_name>`  |
| snap    | Save a snapshot of source folder | `[snapshot_name]` |
