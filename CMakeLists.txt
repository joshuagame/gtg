cmake_minimum_required(VERSION 3.25)

project(
    gtg
    VERSION 0.1.0
    DESCRIPTION "Guerra Termonucleare Globale"
    LANGUAGES C
)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not supported. Use a CMake preset.")
endif()

include(CTest)
include(FetchContent)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(gtg_enable_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX)
    elseif(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
        target_compile_options(
            ${target_name}
            PRIVATE
                -Wall
                -Wextra
                -Wpedantic
                -Wconversion
                -Wshadow
                -Werror
        )
    endif()
endfunction()

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    # Immutable commit referenced by the raylib 6.0 tag.
    GIT_TAG dbc56a87da87d973a9c5baa4e7438a9d20121d28
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

# Apple Clang 21 reports a tautological overflow guard in raylib 6.0's bundled
# stb_vorbis. Keep project warnings strict while isolating this upstream warning
# to the third-party target that owns it.
if(TARGET raylib AND CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang")
    target_compile_options(raylib PRIVATE -Wno-tautological-compare)
endif()

add_library(
    gtg_core
    STATIC
        src/app/app.c
        src/app/app_config.c
        src/resources/resources.c
)
target_include_directories(gtg_core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(gtg_core PRIVATE c_std_17)
set_target_properties(
    gtg_core
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
)
gtg_enable_warnings(gtg_core)

add_executable(gtg src/main.c src/app/app_raylib.c)
target_include_directories(gtg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(gtg PRIVATE c_std_17)
set_target_properties(
    gtg
    PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF
)
target_link_libraries(gtg PRIVATE gtg_core raylib)
gtg_enable_warnings(gtg)

if(BUILD_TESTING)
    add_executable(gtg_tests src/tests/test_main.c)
    target_include_directories(gtg_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_compile_features(gtg_tests PRIVATE c_std_17)
    set_target_properties(
        gtg_tests
        PROPERTIES
            C_STANDARD 17
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS OFF
    )
    target_link_libraries(gtg_tests PRIVATE gtg_core)
    gtg_enable_warnings(gtg_tests)

    add_test(NAME gtg.lifecycle COMMAND gtg_tests)
endif()
