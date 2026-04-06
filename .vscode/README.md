# VS Code Configuration for createres

This folder contains VS Code workspace configuration for building and debugging createres on both Linux and Windows (MSYS64).

## Setup

### Linux
No special setup required. Open the workspace and run the build task.

### Windows with MSYS64

1. **Install MSYS64** from https://www.msys2.org/
   - Install MinGW64 development tools when prompted

2. **Configure VS Code**
   - The C++ IntelliSense is configured for MSYS64 by default
   - Update paths in `c_cpp_properties.json` if MSYS64 is installed in a different location

3. **Open in VS Code**
   ```bash
   code .
   ```

4. **Build**
   - Open Command Palette (Ctrl+Shift+P)
   - Run: `Tasks: Run Build Task`
   - Select "Build (Windows - MSYS64)"

## Files

- **settings.json** - Workspace settings (C++ standard, file exclusions)
- **c_cpp_properties.json** - IntelliSense configuration for Linux and Windows
- **tasks.json** - Build tasks for both platforms
- **launch.json** - Debug configurations for both platforms
- **extensions.json** - Recommended VS Code extensions

## Build Variants

The workspace includes multiple build configurations:

- `Build (Linux)` - Standard Linux build with make
- `Build (Windows - MSYS64)` - Debug build with Makefile.win32
- `Build Release (Windows - MSYS64)` - Release build with optimizations
- `Clean` - Remove all build artifacts
- `Run` - Build and run the tool

## Debugging

1. Set breakpoints in the code
2. Press F5 or use Run → Start Debugging
3. Select appropriate debug configuration for your platform

## Troubleshooting

### Build fails with "make: command not found" on Windows
- Ensure MSYS64 is properly installed
- Check that `make` is available in MSYS64
- You may need to run: `pacman -S mingw-w64-x86_64-make`

### IntelliSense errors with Windows includes
- Verify MSYS64 installation path in `c_cpp_properties.json`
- Restart VS Code after configuration changes
- Run: `C_Cpp: Reset IntelliSense Database` from Command Palette
