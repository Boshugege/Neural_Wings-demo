#pragma once
#include "IEvent.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>
#include <typeindex>
#include <atomic>

using Subscription_ID = size_t;
class EventManager
{
public:
    using EventCallback = std::function<void(const IEvent &)>;

    struct Subscription
    {
        Subscription_ID id;
        EventCallback callback;
    };

    EventManager() : m_nextID(1) {}

    template <typename T>
    Subscription_ID Subscribe(std::function<void(const T &)> callback)
    {
        Subscription_ID id = m_nextID++;
        std::type_index typeIndex = typeid(T);
        auto wrapper = [callback](const IEvent &event)
        {
            callback(static_cast<const T &>(event));
        };
        m_subscribers[typeIndex].push_back({id, wrapper});
        m_idToType.emplace(id, typeIndex);
        return id;
    }

    void Unsubscribe(Subscription_ID id)
    {
        auto itType = m_idToType.find(id);
        if (itType == m_idToType.end())
            return;
        std::type_index typeIndex = itType->second;
        auto &list = m_subscribers[typeIndex];

        for (auto it = list.begin(); it != list.end(); ++it)
        {
            if (it->id == id)
            {
                list.erase(it);
                break;
            }
        }
        m_idToType.erase(itType);
    }

    void Emit(const IEvent &event)
    {
        auto it = m_subscribers.find(event.GetTypeIndex());
        if (it != m_subscribers.end())
        {
            for (auto &sub : it->second)
                sub.callback(event);
        }
    }

private:
    std::atomic<Subscription_ID> m_nextID;
    std::unordered_map<std::type_index, std::vector<Subscription>> m_subscribers;
    std::unordered_map<Subscription_ID, std::type_index> m_idToType;
};