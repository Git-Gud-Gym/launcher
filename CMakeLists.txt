cmake_minimum_required(VERSION 3.20)
# Release version. Tag releases as eldenring-v<version> (see CHANGELOG.md).
project(GitGudGym VERSION 0.1.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Static CRT so neither binary depends on whatever redistributable is installed.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(OUT_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUT_DIR})

add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
add_compile_options(/utf-8)

configure_file(src/common/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h)
include_directories(${CMAKE_BINARY_DIR}/generated src/common)

# --- The mod, loaded into the game ---
add_library(mod SHARED
    src/mod/dllmain.cpp
    src/mod/config.cpp
    src/mod/memory.cpp
    src/mod/game.cpp
    src/mod/stats.cpp
    src/mod/trainer.cpp
    src/mod/mod.rc
)
set_target_properties(mod PROPERTIES OUTPUT_NAME gitgudgym)
target_compile_options(mod PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4>)

# --- Dear ImGui (vendored in third_party/imgui, v1.91.5) ---
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)

# --- The desktop app ---
add_executable(app WIN32
    src/app/main.cpp
    src/app/app.cpp
    src/app/ui.cpp
    src/app/overlay.cpp
    src/app/injector.cpp
    src/app/locator.cpp
    src/app/app.rc
)
set_target_properties(app PROPERTIES OUTPUT_NAME GitGudGym)
target_compile_options(app PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4>)
target_link_libraries(app PRIVATE imgui d3d11 dxgi d3dcompiler dwmapi windowscodecs gdiplus winmm version)
# Always run elevated: attaching to the game and launching it as admin both need it.
target_link_options(app PRIVATE "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'")

# Ship the default config and assets next to the binaries, but never clobber a user-edited config.
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_SOURCE_DIR}/config/gitgudgym.ini -DDST=${OUT_DIR}/gitgudgym.ini
            -P ${CMAKE_SOURCE_DIR}/cmake/copy_if_missing.cmake
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUT_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/assets/logo_mark.png ${OUT_DIR}/assets/logo.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/assets/mixkit-gaming-lock-2848.wav
            ${OUT_DIR}/assets/notify.wav)
