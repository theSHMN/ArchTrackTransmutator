#!/bin/bash
set -e

echo "Cleaning build directory..."
rm -rf build/macos-debug

echo "Configuring project..."
cmake --preset macos-debug

echo "Building project..."
cmake --build --preset build-macos-debug

echo "Running executable..."
./build/macos-debug/ArchTrackTransmutator