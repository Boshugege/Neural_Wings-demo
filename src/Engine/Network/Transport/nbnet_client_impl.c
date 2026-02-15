// ────────────────────────────────────────────────────────────────────
// nbnet client implementation – compiled as C
//
// This .c file compiles the nbnet implementation (NBNET_IMPL) and the
// platform-appropriate driver.  It MUST be compiled as C (not C++) because
// nbnet uses C99 designated initializers that MSVC rejects in C++17.
//
// The corresponding C++ wrapper (NBNetTransport.cpp) includes nbnet.h
// inside extern "C" { } for declaration-only access.
// ────────────────────────────────────────────────────────────────────

#include <stdio.h>

// nbnet logging – redirect to printf (Raylib's TraceLog is C++ in our build)
#define NBN_LogInfo(...)    do { printf("[nbnet INFO] ");    printf(__VA_ARGS__); printf("\n"); } while(0)
#define NBN_LogError(...)   do { fprintf(stderr, "[nbnet ERROR] "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define NBN_LogWarning(...) do { printf("[nbnet WARN] ");    printf(__VA_ARGS__); printf("\n"); } while(0)
#define NBN_LogDebug(...)   (void)0
#define NBN_LogTrace(...)   (void)0

#define NBNET_IMPL

#include <nbnet.h>

#if defined(PLATFORM_WEB)
#include <net_drivers/webrtc.h>
#else
#include <net_drivers/udp.h>
#endif
