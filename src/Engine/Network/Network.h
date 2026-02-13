#pragma once

// ── Network module umbrella header ──────────────────────────────
// NOTE: ENetTransport.h is intentionally NOT included here because
// it pulls in <enet/enet.h> → <winsock2.h> → <windows.h>, which
// conflicts with Raylib's CloseWindow / ShowCursor declarations.
// Only .cpp files that need the transport should include it directly.

#include "Engine/Network/NetTypes.h"
#include "Engine/Network/Protocol/MessageTypes.h"
#include "Engine/Network/Protocol/Messages.h"
#include "Engine/Network/Protocol/PacketSerializer.h"
#include "Engine/Network/Client/NetworkClient.h"
#include "Engine/Network/Sync/NetworkSyncComponent.h"
#include "Engine/Network/Sync/NetworkSyncSystem.h"
