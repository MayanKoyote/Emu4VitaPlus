cmake_minimum_required(VERSION 3.12)

set(PUBLIC_PKG_FILES "")
collect_files(PUBLIC_PKG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/pkg)
SET(PUBLIC_PKG_FILES "${PUBLIC_PKG_FILES}" CACHE INTERNAL "PUBLIC_PKG_FILES")

add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.h

    COMMAND python export_share.py
    COMMAND mv language_define.h ${CMAKE_CURRENT_SOURCE_DIR}/source
    COMMAND mv language_define.cpp ${CMAKE_CURRENT_SOURCE_DIR}/source

    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../script

    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/language.json
    ${CMAKE_CURRENT_SOURCE_DIR}/../script/export_share.py
)

add_custom_target(
    LANG_SHARE
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.h)

add_compile_definitions(_APP_VER_STR="${APP_VER}")

file(GLOB SRC source/*.cpp)

if(NOT ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp IN_LIST SRC)
    list(APPEND SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/language_define.cpp)
endif()

include_directories(
    ../deps/libretro-common/include
    ../deps/7-Zip/C
    ../deps/lz4/lib)

add_library(share STATIC ${SRC})
add_dependencies(share LANG_SHARE)