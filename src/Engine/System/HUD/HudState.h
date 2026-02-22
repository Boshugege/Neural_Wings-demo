#pragma once
#include <string>
#include <utility>

class HudState
{
public:
    HudState() : m_id(-1), m_name("none") {}
    HudState(int id, std::string name) : m_id(id), m_name(std::move(name)) {}

    operator int() const { return m_id; }
    operator std::string() const { return m_name; }

    bool operator==(const HudState &other) const { return m_id == other.m_id; }
    bool operator!=(const HudState &other) const { return m_id != other.m_id; }

    std::string GetName() const { return m_name; }

private:
    int m_id;
    std::string m_name;
};

static HudState HUD_STATE_NONE = {-1, "none"};
