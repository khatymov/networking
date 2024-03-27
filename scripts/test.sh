#!/bin/bash

# Get the directory where the script is located
script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Change to the project root directory
cd "${script_dir}/.."

if [ -f "CMakeUserPresets.json" ]; then
  rm "CMakeUserPresets.json" || exit $?
fi

build_dir="build"

if [ -d "${build_dir}" ]; then
  rm -rf "${build_dir}" || exit $?
fi

# Install dependencies
conan install . --output-folder="${build_dir}" --build=missing || exit $?

cd "${build_dir}" || exit $?

# Build
cmake ../tests/ -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release || exit $?
cmake --build . --config Release || exit $?

# Run all unit tests
./run_unit_tests || exit $?

# Get coverage and html report
lcov --directory . --capture --output-file coverage.info || exit $?
lcov --remove coverage.info '/opt/*' '/usr/*' '*/tests/*' 'build/*' --output-file coverage.info || exit $?
lcov --list coverage.info || exit $?
genhtml coverage.info --output-directory out || exit $?

# google-chrome-stable out/index.html

# Check for memory leaks
valgrind --leak-check=full ./run_unit_tests
