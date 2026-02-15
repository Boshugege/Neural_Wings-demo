#pragma once
#include "Engine/Core/Components/IComponent.h"
#include "Engine/Network/NetTypes.h"

/// Attach this component to any GameObject whose transform
/// should be synchronised over the network.
class NetworkSyncComponent : public IComponent
{
public:
    NetworkSyncComponent() = default;
    explicit NetworkSyncComponent(NetObjectID netID, bool isLocal = false)
        : netObjectID(netID), isLocalPlayer(isLocal) {}

    /// Unique network object identifier (assigned by server or locally).
    NetObjectID netObjectID = INVALID_NET_OBJECT_ID;

    /// True  → we own this object and SEND its position to the server.
    /// False → this is a remote player; we RECEIVE its position.
    bool isLocalPlayer = false;

    /// Owner client id – filled once ServerWelcome is received.
    ClientID ownerClientID = INVALID_CLIENT_ID;
};
