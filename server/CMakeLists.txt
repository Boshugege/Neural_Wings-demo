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
# Server uses nbnet directly (NBN_IMPL is in GameServer.cpp).
# No shared transport .cpp files are linked – only protocol headers.
set(SERVER_SOURCES
    main.cpp
    GameServer.cpp
    nbnet_server_impl.c
)

add_executable(${PROJECT_NAME} ${SERVER_SOURCES})

# ── nbnet ────────────────────────────────────────────────────────
set(NBNET_ROOT "${CMAKE_SOURCE_DIR}/third_party/nbnet")

if(NOT EXISTS "${NBNET_ROOT}/nbnet.h")
    message(FATAL_ERROR
        "nbnet not found!  Please clone/download nbnet into:\n"
        "  ${NBNET_ROOT}\n"
        "  (needs nbnet.h and net_drivers/ folder)\n"
        "  https://github.com/nathhB/nbnet")
endif()

# ── Include paths ────────────────────────────────────────────────
# Server code can #include "Engine/Network/..." for shared protocol headers.
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NBNET_ROOT}
)

# On Windows, nbnet UDP driver needs ws2_32.
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm)
endif()

# TODO: When adding WebRTC server driver, link additional dependencies here
# (e.g. OpenSSL, libdatachannel, etc. depending on nbnet's webrtc_c driver).
