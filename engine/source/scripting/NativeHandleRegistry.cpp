#include "scripting/NativeHandleRegistry.h"

#include "core/GameObject.h"

namespace eng
{
    NativeHandleRegistry& NativeHandleRegistry::GetInstance()
    {
        static NativeHandleRegistry registry;
        return registry;
    }

    NativeHandleRegistry::Handle NativeHandleRegistry::Acquire(GameObject* object)
    {
        if (!object)
        {
            return 0;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);
        const auto existing = m_ObjectIndices.find(object);
        if (existing != m_ObjectIndices.end())
        {
            Entry& entry = m_Entries[existing->second];
            ++entry.references;
            return Encode(existing->second, entry.generation);
        }

        std::uint32_t index = 0;
        if (!m_FreeIndices.empty())
        {
            index = m_FreeIndices.back();
            m_FreeIndices.pop_back();
            Entry& entry = m_Entries[index];
            entry.object = object;
            entry.references = 1;
        }
        else
        {
            index = static_cast<std::uint32_t>(m_Entries.size());
            m_Entries.push_back(Entry{object, 1, 1});
        }

        m_ObjectIndices.emplace(object, index);
        return Encode(index, m_Entries[index].generation);
    }

    void NativeHandleRegistry::Release(Handle handle)
    {
        std::uint32_t index = 0;
        std::uint32_t generation = 0;
        if (!Decode(handle, index, generation))
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);
        if (index >= m_Entries.size())
        {
            return;
        }

        Entry& entry = m_Entries[index];
        if (!entry.object || entry.generation != generation || entry.references == 0)
        {
            return;
        }

        --entry.references;
        if (entry.references != 0)
        {
            return;
        }

        m_ObjectIndices.erase(entry.object);
        entry.object = nullptr;
        entry.generation = entry.generation == UINT32_MAX ? 1 : entry.generation + 1;
        m_FreeIndices.push_back(index);
    }

    GameObject* NativeHandleRegistry::Resolve(Handle handle) const
    {
        std::uint32_t index = 0;
        std::uint32_t generation = 0;
        if (!Decode(handle, index, generation))
        {
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);
        if (index >= m_Entries.size())
        {
            return nullptr;
        }

        const Entry& entry = m_Entries[index];
        return entry.object && entry.generation == generation && entry.references != 0
            ? entry.object
            : nullptr;
    }

    bool NativeHandleRegistry::IsValid(Handle handle) const
    {
        return Resolve(handle) != nullptr;
    }

    NativeHandleRegistry::Handle NativeHandleRegistry::Encode(
        std::uint32_t index, std::uint32_t generation)
    {
        return (static_cast<Handle>(generation) << 32u) |
               static_cast<Handle>(index + 1u);
    }

    bool NativeHandleRegistry::Decode(
        Handle handle, std::uint32_t& index, std::uint32_t& generation)
    {
        if (handle == 0)
        {
            return false;
        }

        const std::uint32_t encodedIndex = static_cast<std::uint32_t>(handle & 0xffffffffu);
        generation = static_cast<std::uint32_t>(handle >> 32u);
        if (encodedIndex == 0 || generation == 0)
        {
            return false;
        }

        index = encodedIndex - 1u;
        return true;
    }
}
