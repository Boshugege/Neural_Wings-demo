#pragma once
#include "Engine/Network/Transport/INetworkTransport.h"
#include <memory>

/// The ONE place where platform-specific transport selection lives.
/// All other code uses INetworkTransport and never sees #if PLATFORM_*.
namespace TransportFactory
{

    inline std::unique_ptr<INetworkTransport> Create()
    {
#if defined(PLATFORM_DESKTOP)
        // Forward-declared; implemented in TransportFactory.cpp to avoid
        // leaking ENet/Windows headers into every translation unit.
        extern std::unique_ptr<INetworkTransport> CreateENetTransport();
        return CreateENetTransport();
#else
        // Web / other platforms get a no-op transport for now.
        // Replace with WebRTCTransport when ready.
        extern std::unique_ptr<INetworkTransport> CreateNullTransport();
        return CreateNullTransport();
#endif
    }

} // namespace TransportFactory
