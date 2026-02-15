#include "ClientIdentity.h"
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ── Public ─────────────────────────────────────────────────────────

bool ClientIdentity::LoadOrGenerate(const std::string &filePath)
{
    if (LoadFromFile(filePath))
    {
        std::cout << "[ClientIdentity] Loaded UUID: " << GetUUIDString() << "\n";
        return true;
    }

    // First launch — generate and save
    Generate();
    std::cout << "[ClientIdentity] Generated new UUID: " << GetUUIDString() << "\n";

    if (!SaveToFile(filePath))
    {
        std::cerr << "[ClientIdentity] Warning: could not save identity to " << filePath << "\n";
    }

    return true;
}

std::string ClientIdentity::GetUUIDString() const
{
    return ToHexString(m_uuid);
}

// ── File I/O ───────────────────────────────────────────────────────

bool ClientIdentity::LoadFromFile(const std::string &filePath)
{
    std::ifstream in(filePath);
    if (!in.is_open())
        return false;

    try
    {
        json j;
        in >> j;

        std::string uuidStr = j.value("uuid", "");
        if (uuidStr.empty())
            return false;

        NetUUID loaded{};
        if (!FromHexString(uuidStr, loaded))
        {
            std::cerr << "[ClientIdentity] Invalid UUID format in file\n";
            return false;
        }

        if (loaded.IsNull())
            return false;

        m_uuid = loaded;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ClientIdentity] Failed to parse " << filePath << ": " << e.what() << "\n";
        return false;
    }
}

bool ClientIdentity::SaveToFile(const std::string &filePath) const
{
    json j;
    j["uuid"] = ToHexString(m_uuid);

    std::ofstream out(filePath);
    if (!out.is_open())
        return false;

    out << j.dump(4);
    return out.good();
}

// ── UUID v4 Generation ─────────────────────────────────────────────

void ClientIdentity::Generate()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;

    uint64_t hi = dist(gen);
    uint64_t lo = dist(gen);

    std::memcpy(m_uuid.bytes, &hi, 8);
    std::memcpy(m_uuid.bytes + 8, &lo, 8);

    // Set version 4 (bits 48–51 = 0100)
    m_uuid.bytes[6] = (m_uuid.bytes[6] & 0x0F) | 0x40;
    // Set variant 1 (bits 64–65 = 10)
    m_uuid.bytes[8] = (m_uuid.bytes[8] & 0x3F) | 0x80;
}

// ── Hex string conversion ──────────────────────────────────────────

std::string ClientIdentity::ToHexString(const NetUUID &uuid)
{
    // Format: 8-4-4-4-12  (36 chars)
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');

    for (int i = 0; i < 16; ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
            ss << '-';
        ss << std::setw(2) << static_cast<unsigned>(uuid.bytes[i]);
    }
    return ss.str();
}

bool ClientIdentity::FromHexString(const std::string &str, NetUUID &out)
{
    // Accept with or without dashes
    std::string hex;
    hex.reserve(32);
    for (char c : str)
    {
        if (c == '-')
            continue;
        hex += c;
    }

    if (hex.size() != 32)
        return false;

    for (int i = 0; i < 16; ++i)
    {
        unsigned val = 0;
        auto result = std::sscanf(hex.c_str() + i * 2, "%2x", &val);
        if (result != 1)
            return false;
        out.bytes[i] = static_cast<uint8_t>(val);
    }

    return true;
}
