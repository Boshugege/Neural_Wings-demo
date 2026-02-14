#include "Engine/Network/Transport/TransportFactory.h"
#include "Engine/Network/Transport/NullTransport.h"

// Platform-specific includes are isolated here — the ONLY .cpp
// that conditionally pulls in ENet headers.
#if defined(PLATFORM_DESKTOP)
#include "Engine/Network/Transport/ENetTransport.h"
#endif

namespace TransportFactory
{

#if defined(PLATFORM_DESKTOP)
    std::unique_ptr<INetworkTransport> CreateENetTransport()
    {
        return std::make_unique<ENetTransport>();
    }
#endif

    std::unique_ptr<INetworkTransport> CreateNullTransport()
    {
        return std::make_unique<NullTransport>();
    }

} // namespace TransportFactory
