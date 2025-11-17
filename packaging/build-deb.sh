#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"

if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TOOLS=ON \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_TESTS=OFF

echo "Building..."
cmake --build . -j$(nproc)

echo "Creating DEB package..."
cpack -G DEB

echo ""
echo "DEB packages created in build/ directory:"
ls -lh *.deb

echo ""
echo "To install:"
echo "  sudo dpkg -i libiubpatch0_*.deb"
echo "  sudo dpkg -i libiubpatch-dev_*.deb"
echo "  sudo dpkg -i iubpatch-tools_*.deb"
