set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# https://github.com/messense/homebrew-macos-cross-toolchains
set(CMAKE_C_COMPILER   aarch64-unknown-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-unknown-linux-gnu-g++)
set(CMAKE_STRIP        aarch64-unknown-linux-gnu-strip)

if(NOT DEFINED TARGET_SYSROOT)
  set(TARGET_SYSROOT "" CACHE PATH "Path to target sysroot")
endif()

if(TARGET_SYSROOT)
  set(CMAKE_SYSROOT ${TARGET_SYSROOT})
  set(CMAKE_FIND_ROOT_PATH ${TARGET_SYSROOT})
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)