#pragma once
#include "Engine/Network/NetTypes.h"
#include <string>

/// Manages a persistent client UUID stored on disk.
/// On first launch, generates a UUID v4 and saves it.
/// On subsequent launches, loads the existing UUID.
class ClientIdentity
{
public:
    /// Load or generate identity from the given file path.
    /// Returns true if a valid UUID is available after the call.
    bool LoadOrGenerate(const std::string &filePath = "assets/config/client_identity.json");

    /// Get the current UUID (valid after LoadOrGenerate).
    const NetUUID &GetUUID() const { return m_uuid; }

    /// Get the UUID as a hex string (e.g. "550e8400-e29b-41d4-a716-446655440000").
    std::string GetUUIDString() const;

private:
    bool LoadFromFile(const std::string &filePath);
    bool SaveToFile(const std::string &filePath) const;
    void Generate();

    /// Convert 16 bytes to/from a 36-char UUID string.
    static std::string ToHexString(const NetUUID &uuid);
    static bool FromHexString(const std::string &str, NetUUID &out);

    NetUUID m_uuid{};
};
