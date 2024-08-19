# REB - Rebuild Command Line Utility
REB is a small command line build tool which you can use to build your project and keep track of releases.

## Configuration
Before using REB, you need to execute `install.sh` to begin the installation

To create a local configuration you just need to cd into your project folder and initialise it
with `reb init <language>`. Now you can edit your model in `.reb/config.json`
```
/path/to/project
L .reb
  L config.json
```

> To add new languages, you can create a file under `~/.config/reb/models/<language>.json` 

### config.json
| Name | Description | Type |
| :--- | :---------- | :--- |
| compilation.command | the compilation command                          | `string      ` |
| compilation.flags   | the flags to append to the command               | `string array` |
| compilation.source  | the POSIX regex to match against the files' path | `string      ` |
| compilation.dest    | the destination folder                           | `string      ` |
| linking.command     | the linking command                              | `string      ` |
| linking.flags       | the flags to append to the command               | `string array` |
| linking.source      | the POSIX regex to match against the files' path | `string      ` |
| linking.dest        | the destination folder                           | `string      ` |
| linking.deps        | the dependencies to include                      | `string array` |
| linking.target      | the executable name                              | `string      ` |
| post compile        | the commands to execute after the compilation    | `string array` |

> Check the [assets](assets) folder for examples

## Usage
After you have initialised your project's folder as a reb repository, you can also define a `.rebignore` file
to exclude folders or files from the compilation, the hash calculation and the snapshots
```
/path/to/project
L .rebignore
L ...
```

Then you can run `reb run <model_name>` to start the compilation

### Commands
| Name | Description | Parameters |
| :--- | :---------- | :--------- |
| help  | Display usage informations                                         | `-              ` |
| init  | Initialise the current folder as a reb repository                  | `<language>     ` |
| run   | Run the model provided                                             | `<model_name>   ` |
| clean | Clean the repository                                               | `-              ` |
| snap  | Take a snapshot of the repository or revert to a previous snapshot | `[snapshot_path]` |
