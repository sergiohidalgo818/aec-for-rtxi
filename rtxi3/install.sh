#!/bin/sh

cmake -B build
cmake --build build/
sudo cmake --install ./build

PLUGIN_SRC_DIR="/usr/local/bin/rtxi_modules"

echo "Plugins have been installed to: $PLUGIN_SRC_DIR"
