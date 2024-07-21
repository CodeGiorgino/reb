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
      L c.config
      L cpp.config
      L ...
```

### Local
To create a local configuration you just need to cd into your project folder and initialise it
with `reb init <language>`. Now you can edit your model in `.reb/language.config`
```
/path/to/project
L .reb
  L reb.config
```

> **Note:** the local config file will be merged into the global one on initialisation, overwriting the global properties

### Parameters
| Name          | Value type  | Description                                         |
| :------------ | :---------- | :-------------------------------------------------- |
| **EXT**       | string      | The extension of the files to compile               |
| **NAME**      | string      | The final build name                                |
| **COMP**      | string      | Compiler to use                                     |
| **FLAGS**     | string      | Compilation flags                                   |
| **SOURCE**    | string      | Source folder                                       |
| **BUILD**     | string      | Build output folder                                 |
| **AUTO_RUN**  | true\|false | Whether to auto start the program after compilation |
| **RECURSIVE** | true\|false | Whether to auto compile the sub repositories        |

## Usage
TODO

### Commands
| Name  | Description                                                     | Parameters        |
| :-----| :-------------------------------------------------------------- | :---------------- |
| help  | Display usage informations                                      | -                 |
| init  | Initialise the current folder as a reb repository               | `<language>`      |
| run   | Run the model provided                                          | `<model_name>`    |
| clean | Clean the repository                                            | -                 |
| snap  | Take a snapshot of the repository or revert to a taken snapshot | `[snapshot_path]` |
