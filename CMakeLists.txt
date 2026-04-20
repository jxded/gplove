cmake_minimum_required(VERSION 3.20)
project(gplove VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # for clangd/lsp autocompletes

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2_TTF   REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_GFX   REQUIRED SDL2_gfx)

add_executable(gplove
    src/main.cpp
    src/renderer.cpp
    src/input.cpp
    src/trail.cpp
)

target_include_directories(gplove PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_GFX_INCLUDE_DIRS}
)

target_compile_options(gplove PRIVATE
    ${SDL2_CFLAGS_OTHER}
    -Wall -Wextra
)

target_link_libraries(gplove PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_GFX_LIBRARIES}
)

add_custom_command(TARGET gplove POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:gplove>/assets
    COMMENT "Copying assets to build directory"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET gplove POST_BUILD
        COMMAND ${CMAKE_STRIP} $<TARGET_FILE:gplove>
        COMMENT "Stripping binary")
endif()