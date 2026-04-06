# createres

Game resource packer with optional Snappy compression.

## Features

- Pack a folder into a single resource file
- List files stored in a resource
- Unpack a file from a resource
- Optional compression with Snappy (`-c`)
- Cross-platform setup for Linux and Windows (MSYS64)

## Project Layout

- `src/main.cpp` - CLI entry point
- `src/Resource.cpp` - pack/list/unpack implementation
- `src/snappy/` - vendored Snappy sources and platform config headers
- `.vscode/` - build/debug tasks for Linux and Windows/MSYS64

## Build

### Linux

```sh
make clean && make
```

Output binary:

- `build/createres`

### Windows (MSYS64)

```sh
make -f Makefile.win32 clean && make -f Makefile.win32
```

Output binary:

- `build/createres.exe`

## Usage

```sh
createres <options>
```

Options:

- `-h` show help
- `-r <resource name>` resource file name
- `-f <folder name>` folder containing files to pack
- `-u <file name>` unpack a single file from the resource
- `-l` list all files in the resource
- `-c` enable compression when packing

## Examples

Pack without compression:

```sh
./build/createres -r resources.dat -f DATA
```

Pack with compression:

```sh
./build/createres -r resources.dat -c -f DATA
```

List files:

```sh
./build/createres -r resources.dat -l
```

Unpack one file:

```sh
./build/createres -r resources.dat -u image1.bmp
```

## VS Code

This repository includes VS Code workspace configs in `.vscode/`:

- Build tasks for Linux and Windows (MSYS64)
- Debug launch configs for both platforms
- IntelliSense profiles for Linux and MSYS64

Open the workspace and run `Tasks: Run Build Task` to build.
