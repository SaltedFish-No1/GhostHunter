cmake_minimum_required(VERSION 3.16)
project(GhostHunter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

# Layout binaries the way the original project ships them, so the relative
# resource paths baked into main.cpp ("../res/...") keep working.
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
    common/model/mesh.cpp
    common/model/model.cpp
)

set(GH_MAIN_SOURCES
    GhostHunter/src/main.cpp
    GhostHunter/src/glad.c
)

add_executable(GhostHunter ${GH_MAIN_SOURCES} ${GH_COMMON_SOURCES})

target_include_directories(GhostHunter PRIVATE
    ${CMAKE_SOURCE_DIR}/include       # GLFW, glad, KHR, glm, assimp, stb_image
    ${CMAKE_SOURCE_DIR}/common        # tools.h, player.h, ...
    ${CMAKE_SOURCE_DIR}/common/model  # model.h, mesh.h
)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
# OpenGL is provided by the platform.
find_package(OpenGL REQUIRED)
target_link_libraries(GhostHunter PRIVATE OpenGL::GL)

# Threads — used by Ghost animation threads, Camera::timer, etc.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(GhostHunter PRIVATE Threads::Threads)

if(WIN32)
    # On Windows, use the vendored binaries that already ship in the repo,
    # matching what GhostHunter.vcxproj + config/*.props do.
    set(GH_VENDOR_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

    target_link_libraries(GhostHunter PRIVATE
        ${GH_VENDOR_LIB_DIR}/glfw3.lib
        ${GH_VENDOR_LIB_DIR}/assimp-vc143-mt.lib
    )
    target_link_libraries(GhostHunter PRIVATE user32 gdi32 shell32)

    # Copy the assimp runtime next to the exe so it can be launched directly.
    add_custom_command(TARGET GhostHunter POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${GH_VENDOR_LIB_DIR}/assimp-vc143-mt.dll
            $<TARGET_FILE_DIR:GhostHunter>
    )

    # Visual Studio: set the debugger CWD so "../res/..." resolves.
    set_target_properties(GhostHunter PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:GhostHunter>
    )
else()
    # On Linux/macOS, assume system-installed dependencies.
    find_package(glfw3 REQUIRED)
    find_package(assimp REQUIRED)
    target_link_libraries(GhostHunter PRIVATE glfw assimp::assimp ${CMAKE_DL_LIBS})
endif()

# ---------------------------------------------------------------------------
# Resources
# ---------------------------------------------------------------------------
# main.cpp loads shaders/models via "../res/..." relative to the working
# directory. Mirror the original Excutable/ layout so the exe in
# build/bin/GhostHunter/ can find build/bin/res/ at runtime.
add_custom_command(TARGET GhostHunter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/res
        ${GH_RUNTIME_DIR}/res
)
