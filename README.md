# IUBPatchLib

[![CI/CD](https://github.com/advnpzn/IUBPatchLib/actions/workflows/ci.yml/badge.svg)](https://github.com/advnpzn/IUBPatchLib/actions/workflows/ci.yml)

A modern, cross-platform C++ library for applying binary patches in IPS, UPS, and BPS formats created with usage for ROM hacks in mind, but I believe it can be used for anything.

## Features

- Supports IPS, UPS, and BPS patch formats
- Clean, modern API with RAII, move semantics
- Works on Windows, Linux, macOS, and other POSIX systems
- Optional memory-mapped I/O for fast file processing
- Core library has no external dependencies

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Build Options

- `BUILD_SHARED_LIBS`: Build as shared library (default: ON)
- `BUILD_STATIC_LIBS` : Build static libraries (default: ON)
- `IUB_ENABLE_MMAP`: Enable memory-mapped I/O (default: ON)
- `BUILD_EXAMPLES`: Build example programs (default: OFF)
- `BUILD_TESTS`: Build unit tests (default: ON)
- `BUILD_BENCHMARKS` : Build benchmarks (default: OFF)
- `BUILD_TOOLS`: Build command-line tools (default: ON)

## Usage

### Using IUBPatchLib in Your CMake Project

This directory contains CMake modules for integrating IUBPatchLib into your project.

There are several ways to use IUBPatchLib in your CMake-based project:

### find_package (Recommended)

After installing IUBPatchLib system-wide, you can use `find_package`:

```cmake
cmake_minimum_required(VERSION 3.20)
project(myproj)

find_package(iubpatch REQUIRED)

add_executable(myapp main.cc)
target_link_libraries(myapp PRIVATE iubpatch::iubpatch)
```

### pkg-config

If you have pkg-config available:

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(IUBPATCH REQUIRED iubpatch)

add_executable(myapp main.cc)
target_link_libraries(myapp PRIVATE ${IUBPATCH_LIBRARIES})
target_include_directories(myapp PRIVATE ${IUBPATCH_INCLUDE_DIRS})
target_compile_options(myapp PRIVATE ${IUBPATCH_CFLAGS_OTHER})
```

### FetchContent

To include IUBPatchLib directly in your project:

```cmake
include(FetchContent)

FetchContent_Declare(
    iubpatch
    GIT_REPOSITORY https://github.com/advnpzn/IUBPatchLib.git
    GIT_TAG main
)

FetchContent_MakeAvailable(iubpatch)

add_executable(myapp main.cc)
target_link_libraries(myapp PRIVATE iubpatch::iubpatch)
```

## Example Usage in Code

```cpp
#include <iubpatch/apply.h>
#include <iostream>

int main() {
    auto result = iubpatch::apply_patch(
        "game.ips", 
        "game.rom", 
        "game_patched.rom"
    );
    
    if (result) {
        std::cout << "Patch applied!\n";
        return 0;
    } else {
        std::cerr << "Error: " << result.error().message << "\n";
        return 1;
    }
}
```

## CMake Variables

After `find_package(iubpatch)`, the following variables are available:

- `iubpatch_FOUND` - TRUE if found
- `iubpatch_VERSION` - Version string
- `iubpatch_INCLUDE_DIRS` - Include directories
- `iubpatch_LIBRARIES` - Libraries to link (iubpatch::iubpatch)
- `iubpatch_WITH_MMAP` - Whether mmap support is enabled

## Checking Version

```cmake
find_package(iubpatch 0.1.0 REQUIRED)

if(iubpatch_VERSION VERSION_LESS "0.2.0")
    message(STATUS "Using IUBPatchLib ${iubpatch_VERSION}")
endif()
```

### Library API

```cpp
#include "iubpatch/apply.h"

// Simple patch application
auto result = iubpatch::apply_patch("game.ips", "game.rom", "game_patched.rom");
if (result) {
    std::cout << "Patch applied successfully!\n";
} else {
    std::cerr << "Error: " << result.error().message << "\n";
}

// With options
iubpatch::PatchOptions options;
options.verify_checksums = true;
options.create_backup = true;
auto result = iubpatch::apply_patch("game.ups", "game.rom", "game_patched.rom", options);

// Get patch information
auto info = iubpatch::get_patch_info("game.bps");
if (info) {
    std::cout << "Format: " << iubpatch::format_to_string(info->format) << "\n";
    std::cout << "Source size: " << info->src_size << " bytes\n";
    std::cout << "Target size: " << info->target_size << " bytes\n";
}
```

### Command-Line Tool

```bash
# Apply a patch
iubpatch-cli apply game.ips game.rom game_patched.rom

# Get patch information
iubpatch-cli info game.bps

# Validate patch
iubpatch-cli validate game.ups game.rom

# Apply with options
iubpatch-cli apply game.bps game.rom game_patched.rom --backup --no-mmap
```

## License
Check this for [LICENSE](LICENSE).

## Contributing

Contributions welcome! Please create an issue or raise a PR. I will review and merge it given that the tests pass.

## References

- [IPS File Format](https://zerosoft.zophar.net/ips.php)
- [UPS Format Specification](https://www.romhacking.net/documents/392/)
- [BPS Format Specification](https://github.com/blakesmith/rombp/blob/master/docs/bps_spec.md)
