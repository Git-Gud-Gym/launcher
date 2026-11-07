cmake_minimum_required(VERSION 3.20)
# Hub app version, released as app-v<version>. Each game module has its own version in
# src/games/<id>/module.json, released as <id>-v<version>. See CHANGELOG.md.
project(GitGudGym VERSION 0.3.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Static CRT so nothing depends on whatever redistributable is installed.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(OUT_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUT_DIR})

add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
add_compile_options(/utf-8)
include_directories(src/common)

# Writes generated/<name>/version.h for a target from a "x.y.z" string.
function(write_version_header name version)
    string(REPLACE "." ";" parts "${version}")
    list(GET parts 0 V_MAJOR)
    list(GET parts 1 V_MINOR)
    list(GET parts 2 V_PATCH)
    set(V_STRING "${version}")
    configure_file(${CMAKE_SOURCE_DIR}/src/common/version.h.in ${CMAKE_BINARY_DIR}/generated/${name}/version.h)
endfunction()

# --- Game modules: one DLL per game, built from the shared core plus that game's adapter ---
set(CORE_SOURCES
    src/core/game_adapter.cpp
    src/core/memory.cpp
    src/core/stats.cpp
    src/core/trainer.cpp
)

function(add_game_module id)
    set(dir ${CMAKE_SOURCE_DIR}/src/games/${id})
    file(READ ${dir}/module.json manifest)
    string(JSON version GET "${manifest}" version)
    write_version_header(${id} ${version})

    file(GLOB sources CONFIGURE_DEPENDS ${dir}/*.cpp)
    set(target module_${id})
    add_library(${target} SHARED ${sources} ${CORE_SOURCES} ${dir}/module.rc)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME gitgudgym-${id}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUT_DIR}/modules/${id}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUT_DIR}/modules/${id})
    target_include_directories(${target} PRIVATE ${dir} src/core ${CMAKE_BINARY_DIR}/generated/${id})
    target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4>)
    # bin\modules\<id>\ is a complete module the app picks up as a development install.
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dir}/module.json ${dir}/gitgudgym-${id}.ini
                ${OUT_DIR}/modules/${id})
endfunction()

add_game_module(eldenring)

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

# --- The hub app ---
write_version_header(app ${PROJECT_VERSION})
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
target_include_directories(app PRIVATE ${CMAKE_BINARY_DIR}/generated/app third_party/nlohmann)
target_compile_options(app PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4>)
target_link_libraries(app PRIVATE imgui d3d11 dxgi d3dcompiler dwmapi windowscodecs gdiplus winmm version
                                  winhttp bcrypt)
# Run elevated: attaching to games and launching them as admin both need it.
# -DGGG_REQUIRE_ADMIN=OFF builds a non-elevated app for UI work.
option(GGG_REQUIRE_ADMIN "Embed a requireAdministrator manifest in GitGudGym.exe" ON)
if(GGG_REQUIRE_ADMIN)
    target_link_options(app PRIVATE "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'")
endif()

add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUT_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/assets/logo_mark.png ${OUT_DIR}/assets/logo.png
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/assets/mixkit-gaming-lock-2848.wav
            ${OUT_DIR}/assets/notify.wav)
