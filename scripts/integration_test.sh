#!/bin/bash

# Get the directory of the script
SCRIPT_DIR="$(dirname "$0")"

# Convert to an absolute path
ABSOLUTE_PATH="$(realpath "$SCRIPT_DIR")"
#
#BUILD_SCRIPT="${ABSOLUTE_PATH}/build.sh"
#
#chmod +x ${BUILD_SCRIPT}
#${BUILD_SCRIPT}

build_dir="build"
bin="./network"
data_path="../data/Somedata.txt"
cd ${ABSOLUTE_PATH}/../${build_dir}
#echo ${ABSOLUTE_PATH}/../build
#echo Run server and a few client simultaniously

#server
set -x
${bin} 127.0.0.1 1235 &
# Sleep for 100 milliseconds (0.1 seconds)
sleep 0.1
#clients
${bin} 127.0.0.1 1235 ${data_path} & ${bin} 127.0.0.1 1235 ${data_path}
