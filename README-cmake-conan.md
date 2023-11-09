# CMake and Conan based build environment

## Prerequisites

Building boxes requires two tools to be installed:

- [Conan](https://conan.io), a package manager for C/C++, is used to resolve required library and tool dependencies
  - libunistring
  - pcre2
  - ncurses
  - flex (library and tool)
  - bison (tool)
- [CMake](https://cmake.org) is used for building.  
  Make sure that a C compiler and the tools for the underlying build environment (e.g., [GNU
  Make](https://www.gnu.org/software/make/) or [Ninja](https://ninja-build.org)) are installed.

If Conan is used for the first time, start with the creation of a default
[profile](https://docs.conan.io/2.0/reference/config_files/profiles.html):

```sh
conan profile detect
```

## Building boxes

It is recommended to use the generated presets for building:

```sh
export BUILD_TYPE="Debug"

conan install . --output-folder=./build/${BUILD_TYPE} \
    --settings build_type=${BUILD_TYPE} \
    --build missing

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --preset conan-debug
cmake --build --preset conan-debug

ctest --verbose --preset conan-debug
```

Of course the "classic" way works, too:

```sh
export BUILD_TYPE="Debug"

conan install . --output-folder=./build/${BUILD_TYPE} \
    --settings build_type=${BUILD_TYPE} \
    --build missing

cmake -S ./ -B ./build/${BUILD_TYPE} \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

cmake --build ./build/${BUILD_TYPE} \
    --config ${BUILD_TYPE}

ctest --test-dir ./build/${BUILD_TYPE} --verbose
```
