cmake_minimum_required(VERSION 3.11)

project(Neural_Wings-demo)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    # /FS allows concurrent cl.exe processes to share the same PDB
    # (VS generators enable parallel builds by default).
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Od /Zi /EHsc /FS")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /DNDEBUG /EHsc /FS")
endif()


include(FetchContent)
FetchContent_Declare(
    raylib
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/raylib-master"
)

FetchContent_Declare(
    raygui
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/raygui-master"
)

FetchContent_Declare(
    nlohmann_json
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-develop"
)

FetchContent_Declare(
    enet
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/enet-master"
)

set(ULTRALIGHT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ultralight")

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib raygui nlohmann_json enet)

file(GLOB_RECURSE SOURCES "src/*.cpp")

if(EMSCRIPTEN)
    list(FILTER SOURCES EXCLUDE REGEX "src/Engine/UI/UltralightLayer.cpp")
else()
    list(FILTER SOURCES EXCLUDE REGEX "src/Engine/UI/WebLayer.cpp")
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

if(MSVC)
    # Ensure PDB is shared safely when cl.exe runs in parallel.
    target_compile_options(${PROJECT_NAME} PRIVATE /FS)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${raygui_SOURCE_DIR}/src
)


if(EMSCRIPTEN)
    set(PLATFORM "Web")
    add_definitions(-DPLATFORM_WEB)

else()
    set(PLATFORM "Desktop")
    add_definitions(-DPLATFORM_DESKTOP)
endif()

message(STATUS "当前平台: ${PLATFORM}")


if(PLATFORM STREQUAL "Web")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

    target_link_libraries(${PROJECT_NAME} PRIVATE
        raylib
        nlohmann_json::nlohmann_json
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        "-sUSE_GLFW=3"
        "-sASYNCIFY"
        "--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/src/shell.html"
        "--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets@assets"
    )

    # Try to find npm to build the Vue UI automatically for web builds.
    find_program(NPM_EXECUTABLE npm)
    if(NPM_EXECUTABLE)
        add_custom_target(WebUI
            COMMAND ${NPM_EXECUTABLE} ci
            COMMAND ${NPM_EXECUTABLE} run build
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ui
            COMMENT "Building Vue UI into ui/dist"
        )
        add_dependencies(${PROJECT_NAME} WebUI)
    else()
        message(STATUS "npm not found: skipping automatic UI build for Web. Run 'npm ci' and 'npm run build' in ui/ manually.")
    endif()

    # After building the Emscripten HTML, copy the generated ui/dist into the web build output
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/ui/dist"
        "${CMAKE_BINARY_DIR}/ui/dist"
    )

else()

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${ULTRALIGHT_ROOT}/include)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        raylib
        enet
        nlohmann_json::nlohmann_json
        "${ULTRALIGHT_ROOT}/lib/Ultralight.lib"
        "${ULTRALIGHT_ROOT}/lib/UltralightCore.lib"
        "${ULTRALIGHT_ROOT}/lib/WebCore.lib"
        "${ULTRALIGHT_ROOT}/lib/AppCore.lib"
    )



    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ULTRALIGHT_ROOT}/bin"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ULTRALIGHT_ROOT}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources"
    )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/ui"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/ui"
    )
endif()
