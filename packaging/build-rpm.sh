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

echo "Creating RPM package..."
cpack -G RPM

echo ""
echo "RPM packages created in build/ directory:"
ls -lh *.rpm

echo ""
echo "To install:"
echo "  sudo rpm -i iubpatch-*.rpm"
echo "  sudo rpm -i iubpatch-devel-*.rpm"
echo "  sudo rpm -i iubpatch-tools-*.rpm"
