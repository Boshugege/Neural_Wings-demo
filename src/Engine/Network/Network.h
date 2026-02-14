#pragma once

// ── Network module umbrella header ──────────────────────────────
// NOTE: Transport headers (NBNetTransport.h, nbnet.h) are intentionally
// NOT included here.  Only .cpp files that need the transport should
// include them directly to avoid header pollution.

#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/MessageTypes.h"
#include "Engine/Network/Protocol/Messages.h"
#include "Engine/Network/Protocol/PacketSerializer.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Network/Sync/NetworkSyncComponent.h"
#include "Engine/Network/Sync/NetworkSyncSystem.h"
