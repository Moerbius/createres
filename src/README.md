createres
=========

Game resource packer with Snappy compression

## Building

### Linux

To compile on Linux, use the Makefile in the repository root:

```sh
make
```

The binary will be written to `build/createres`.

### Windows

To compile on Windows, use Makefile.win32:

```sh
make -f Makefile.win32
```

Or with mingw32-make:

```sh
mingw32-make -f Makefile.win32
```

The binary will be written to `build/createres.exe`.

For MSVC builds, compile with `-DWIN32_CONFIG` flag added to your compiler options.

## Platform Configuration

This repository vendors the current upstream Google Snappy sources in `src/snappy/`.

Platform-specific headers are automatically selected at compile time:

- **Linux**: Uses `config-linux.h` and `snappy-stubs-public-linux.h`
  - Features: Full mmap support, sys/uio.h, builtin functions
- **Windows**: Uses `config-win32.h` and `snappy-stubs-public-win32.h`
  - Features: Windows.h support, custom iovec struct

## Usage

```createres < option > < option >```

Options:
- `-r <resource name>` - Resource file name
- `-f <folder name>` - Folder containing files to pack
- `-c` - Enable compression using Snappy
- `-l` - List all files in resource
- `-u <file name>` - Unpack file from resource
- `-h` - Show help

## Examples

Pack files without compression:
```sh
createres -r resources.dat -f DATA_FOLDER
```

Pack files with compression:
```sh
createres -r resources.dat -c -f DATA_FOLDER
```

List files in resource:
```sh
createres -r resources.dat -l
```

Unpack a file:
```sh
createres -r resources.dat -u myfile.txt
```
