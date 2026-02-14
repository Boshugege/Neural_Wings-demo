#pragma once
#include "Engine/Network/Transport/INetworkTransport.h"
#include <iostream>

/// No-op transport used on platforms where networking is not yet implemented
/// (e.g. WASM before WebRTC integration).  All operations silently succeed
/// so that business code never needs #if PLATFORM_*.
class NullTransport : public INetworkTransport
{
public:
    bool Connect(const std::string & /*host*/, uint16_t /*port*/) override
    {
        std::cout << "[NullTransport] Connect() called – networking unavailable on this platform\n";
        return false;
    }
    void Disconnect() override {}
    void Poll(uint32_t /*timeoutMs*/) override {}
    void Send(const uint8_t * /*data*/, size_t /*len*/, uint8_t /*channel*/) override {}

    bool IsConnected() const override { return false; }
    ConnectionState GetState() const override { return ConnectionState::Disconnected; }

    void SetOnConnect(OnConnectFn /*fn*/) override {}
    void SetOnDisconnect(OnDisconnectFn /*fn*/) override {}
    void SetOnReceive(OnReceiveFn /*fn*/) override {}
};
