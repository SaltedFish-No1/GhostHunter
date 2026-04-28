cmake_minimum_required(VERSION 3.16)
project(GhostHunter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
# On Windows the repo ships prebuilt glfw3 / assimp binaries that match
# v143 toolset. On Linux/macOS we fetch sources via FetchContent so the
# project builds with no external setup. Both behaviours can be overridden.
if(WIN32)
    set(_GH_VENDOR_DEFAULT ON)
else()
    set(_GH_VENDOR_DEFAULT OFF)
endif()
option(GHOSTHUNTER_USE_VENDORED_DEPS
    "Use the prebuilt glfw3/assimp binaries shipped under lib/ (Windows-only)"
    ${_GH_VENDOR_DEFAULT})
option(GHOSTHUNTER_FETCH_DEPS
    "Fetch glfw/assimp from source via FetchContent if not vendored"
    ON)

# Lay out the runtime tree the way the original project shipped it,
# so the relative resource paths baked into main.cpp ("../res/...")
# keep working when the exe is run from its own directory.
set(GH_RUNTIME_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GH_RUNTIME_DIR}/GhostHunter)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
set(GH_COMMON_SOURCES
    common/admin.cpp
    common/camera.cpp
    common/ghost.cpp
    common/player.cpp
    common/shaders.cpp
    common/tools.cpp
    common/World.cpp
    common/model/mesh.cpp
    common/model/model.cpp
)

set(GH_MAIN_SOURCES
    GhostHunter/src/main.cpp
    GhostHunter/src/glad.c
)

add_executable(GhostHunter ${GH_MAIN_SOURCES} ${GH_COMMON_SOURCES})

target_include_directories(GhostHunter PRIVATE
    ${CMAKE_SOURCE_DIR}/include       # vendored glad/KHR/glm/stb_image (always)
    ${CMAKE_SOURCE_DIR}/common        # tools.h, player.h, ...
    ${CMAKE_SOURCE_DIR}/common/model  # model.h, mesh.h
)

# Be permissive on warnings — this is legacy student code; cleanup is a
# separate refactor stage. Just don't fail the build on Linux/macOS.
if(MSVC)
    target_compile_options(GhostHunter PRIVATE /W3 /wd4244 /wd4267 /wd4305)
    target_compile_definitions(GhostHunter PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
else()
    target_compile_options(GhostHunter PRIVATE
        -Wno-narrowing
        -Wno-deprecated-declarations
        -Wno-write-strings
    )
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(GhostHunter PRIVATE OpenGL::GL)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(GhostHunter PRIVATE Threads::Threads)

# dlopen/dlsym for glad on POSIX
if(NOT WIN32)
    target_link_libraries(GhostHunter PRIVATE ${CMAKE_DL_LIBS})
endif()

if(GHOSTHUNTER_USE_VENDORED_DEPS)
    if(NOT WIN32)
        message(FATAL_ERROR
            "GHOSTHUNTER_USE_VENDORED_DEPS=ON is only valid on Windows; "
            "the lib/ folder ships .lib/.dll files for MSVC v143.")
    endif()
    set(GH_VENDOR_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
    target_include_directories(GhostHunter PRIVATE
        ${CMAKE_SOURCE_DIR}/include/GLFW/..
        ${CMAKE_SOURCE_DIR}/include/assimp/..
    )
    target_link_libraries(GhostHunter PRIVATE
        ${GH_VENDOR_LIB_DIR}/glfw3.lib
        ${GH_VENDOR_LIB_DIR}/assimp-vc143-mt.lib
    )
    target_link_libraries(GhostHunter PRIVATE user32 gdi32 shell32)

    add_custom_command(TARGET GhostHunter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GH_VENDOR_LIB_DIR}/assimp-vc143-mt.dll
            $<TARGET_FILE_DIR:GhostHunter>
    )

    set_target_properties(GhostHunter PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:GhostHunter>
    )
else()
    # Try system packages first; fall back to FetchContent.
    find_package(glfw3 3.3 QUIET CONFIG)
    find_package(assimp QUIET CONFIG)

    if(NOT glfw3_FOUND OR NOT assimp_FOUND)
        if(NOT GHOSTHUNTER_FETCH_DEPS)
            message(FATAL_ERROR
                "glfw3 and/or assimp not found and GHOSTHUNTER_FETCH_DEPS=OFF. "
                "Install them via your package manager or enable fetching.")
        endif()
        include(FetchContent)

        if(NOT glfw3_FOUND)
            message(STATUS "GhostHunter: fetching GLFW from source")
            set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
            set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
            FetchContent_Declare(glfw
                GIT_REPOSITORY https://github.com/glfw/glfw.git
                GIT_TAG 3.4
                GIT_SHALLOW TRUE
            )
            FetchContent_MakeAvailable(glfw)
        endif()

        if(NOT assimp_FOUND)
            message(STATUS "GhostHunter: fetching assimp from source")
            # GhostHunter only consumes OBJ at runtime, so strip every other
            # importer/exporter to keep the build small and dodge unrelated
            # compile failures (e.g. glTF2Exporter on GCC 13).
            set(ASSIMP_BUILD_TESTS                    OFF CACHE BOOL "" FORCE)
            set(ASSIMP_INSTALL                        OFF CACHE BOOL "" FORCE)
            set(ASSIMP_BUILD_ASSIMP_TOOLS             OFF CACHE BOOL "" FORCE)
            set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
            set(ASSIMP_BUILD_OBJ_IMPORTER             ON  CACHE BOOL "" FORCE)
            set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
            set(ASSIMP_NO_EXPORT                      ON  CACHE BOOL "" FORCE)
            set(ASSIMP_WARNINGS_AS_ERRORS             OFF CACHE BOOL "" FORCE)
            set(BUILD_SHARED_LIBS                     OFF CACHE BOOL "" FORCE)
            FetchContent_Declare(assimp
                GIT_REPOSITORY https://github.com/assimp/assimp.git
                GIT_TAG v5.4.3
                GIT_SHALLOW TRUE
            )
            FetchContent_MakeAvailable(assimp)
        endif()
    endif()

    target_link_libraries(GhostHunter PRIVATE glfw assimp::assimp)

    # The repo's include/ folder ships GLFW 3.3.8 / older assimp headers
    # alongside the always-needed glad/KHR/glm/stb_image. Place the linked
    # targets' include directories BEFORE that path so we don't accidentally
    # compile against vendored headers while linking against fetched/system
    # binaries.
    target_include_directories(GhostHunter BEFORE PRIVATE
        $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:assimp::assimp,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

# ---------------------------------------------------------------------------
# Resources
# ---------------------------------------------------------------------------
# main.cpp loads shaders/models via "../res/..." relative to the exe's
# directory (we chdir there on startup; see common/tools.cpp). Stage them
# under build/bin/res/ so build/bin/GhostHunter/GhostHunter[.exe] can find
# them after the chdir.
add_custom_command(TARGET GhostHunter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        ${GH_RUNTIME_DIR}/res
)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "GhostHunter configuration:")
message(STATUS "  Platform           : ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Compiler           : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Build type         : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Use vendored deps  : ${GHOSTHUNTER_USE_VENDORED_DEPS}")
message(STATUS "  Fetch missing deps : ${GHOSTHUNTER_FETCH_DEPS}")
