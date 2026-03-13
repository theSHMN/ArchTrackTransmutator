#!/bin/bash
set -e

echo "Cleaning build directory..."
rm -rf build

echo "Configuring project..."
cmake -S . -B build

echo "Building project..."
cmake --build build

echo "Running executable..."
./build/ArchTrackTransmutator