# CMake and Conan based build environment

## Prerequisites

Building boxes requires two tools to be installed:

- [Conan](https://conan.io), a package manager for C/C++, is used to resolve required library and tool dependencies
  - libunistring
  - pcre2
  - ncurses (official Conan 2 version should be available soon)
  - flex (library and tool)
  - bison (tool)
  - cmocka (for testing)
- [CMake](https://cmake.org) is used for building.  
  Make sure that a C compiler and the tools for the underlying build environment (e.g., [GNU
  Make](https://www.gnu.org/software/make/) or [Ninja](https://ninja-build.org)) are installed.

If Conan is used for the first time, start with the creation of a default
[profile](https://docs.conan.io/2.0/reference/config_files/profiles.html):

```sh
conan profile detect
```

## Building boxes

It is recommended to use the generated presets for building. For a debug build, follow these steps:

```sh
export BUILD_TYPE="Debug"
export BUILD_PRESET="conan-debug"

conan install . --output-folder=./build/${BUILD_TYPE} \
    --settings build_type=${BUILD_TYPE} \
    --build missing

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --preset ${BUILD_PRESET}
cmake --build --preset ${BUILD_PRESET}

ctest --verbose --preset ${BUILD_PRESET}
```

Release build is similar:

```sh
export BUILD_TYPE="Release"
export BUILD_PRESET="conan-release"

conan install . --output-folder=./build/${BUILD_TYPE} \
    --settings build_type=${BUILD_TYPE} \
    --build missing

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON --preset ${BUILD_PRESET}
cmake --build --preset ${BUILD_PRESET}

ctest --verbose --preset ${BUILD_PRESET}
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
