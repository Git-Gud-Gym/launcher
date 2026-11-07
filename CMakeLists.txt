cmake_minimum_required(VERSION 3.20)
# Git Gud Gym launcher. Released as v<version> (see CHANGELOG.md).
project(GitGudGym VERSION 0.3.2 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Static CRT so nothing depends on whatever redistributable is installed.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(OUT_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUT_DIR})
add_compile_options(/utf-8)

# The launcher <-> module contract (shared memory, file locations, version resources) comes from the
# SDK. To build against a local SDK checkout: build.bat -DFETCHCONTENT_SOURCE_DIR_GGG_SDK=..\sdk
include(FetchContent)
FetchContent_Declare(ggg_sdk
    GIT_REPOSITORY https://github.com/Git-Gud-Gym/sdk.git
    GIT_TAG v0.1.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(ggg_sdk)

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

# --- The app ---
ggg_write_version_header(app ${PROJECT_VERSION})
add_executable(app WIN32
    src/app/main.cpp
    src/app/app.cpp
    src/app/catalog.cpp
    src/app/http.cpp
    src/app/modules.cpp
    src/app/ui.cpp
    src/app/overlay.cpp
    src/app/injector.cpp
    src/app/locator.cpp
    src/app/app.rc
)
set_target_properties(app PROPERTIES OUTPUT_NAME GitGudGym)
# assets/ is on the include path so app.rc can embed the logo and the notification sound.
target_include_directories(app PRIVATE ${CMAKE_BINARY_DIR}/generated/app third_party/nlohmann assets)
target_compile_options(app PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4>)
target_link_libraries(app PRIVATE ggg::contract imgui d3d11 dxgi d3dcompiler dwmapi windowscodecs gdiplus winmm
                                  version winhttp bcrypt shlwapi)

# Run elevated: attaching to games and launching them as admin both need it.
# -DGGG_REQUIRE_ADMIN=OFF builds a non-elevated app for UI work.
option(GGG_REQUIRE_ADMIN "Embed a requireAdministrator manifest in GitGudGym.exe" ON)
if(GGG_REQUIRE_ADMIN)
    target_link_options(app PRIVATE "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'")
endif()

