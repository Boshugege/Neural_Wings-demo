cmake_minimum_required(VERSION 3.11)
project(NeuralWings_Server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/utf-8")
    set(CMAKE_CXX_FLAGS_DEBUG   "/MDd /Od /Z7 /EHsc /FS")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /DNDEBUG /EHsc /FS")
endif()

# ── Sources ──────────────────────────────────────────────────────
set(SERVER_SOURCES
    main.cpp
    GameServer.cpp

    # Shared engine network code (Transport layer)
    ${CMAKE_SOURCE_DIR}/src/Engine/Network/Transport/ENetTransport.cpp
)

add_executable(${PROJECT_NAME} ${SERVER_SOURCES})

# ── Include paths ────────────────────────────────────────────────
# Server code can #include "Engine/Network/..." just like the client.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${enet_SOURCE_DIR}/include
)

# ── Link libraries ──────────────────────────────────────────────
# Server only needs ENet (no raylib, no rendering).
target_link_libraries(${PROJECT_NAME} PRIVATE
    enet
)

# On Windows, ENet requires ws2_32 and winmm.
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm)
endif()
