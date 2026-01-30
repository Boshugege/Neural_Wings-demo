#pragma once
#include <string>
#include <typeindex>

class IEvent
{
public:
    virtual ~IEvent() = default;
    virtual const std::type_index GetTypeIndex() const = 0;

    virtual const std::string GetName() const = 0;
};
#define EVENT_TYPE(type)                                                                    \
    static const std::type_index GetStaticType() { return std::type_index(typeid(type)); }  \
    virtual const std::type_index GetTypeIndex() const override { return GetStaticType(); } \
    virtual const std::string GetName() const override { return #type; }