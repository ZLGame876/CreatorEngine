#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace eng
{
    class GameObject;

    // Stable native identity shared with managed code. The upper 32 bits are a
    // generation counter, the lower 32 bits are a slot index plus one.
    class NativeHandleRegistry
    {
    public:
        using Handle = std::uint64_t;

        static NativeHandleRegistry& GetInstance();

        NativeHandleRegistry(const NativeHandleRegistry&) = delete;
        NativeHandleRegistry& operator=(const NativeHandleRegistry&) = delete;

        Handle Acquire(GameObject* object);
        void Release(Handle handle);
        GameObject* Resolve(Handle handle) const;
        bool IsValid(Handle handle) const;

    private:
        NativeHandleRegistry() = default;

        struct Entry
        {
            GameObject* object = nullptr;
            std::uint32_t generation = 1;
            std::uint32_t references = 0;
        };

        static Handle Encode(std::uint32_t index, std::uint32_t generation);
        static bool Decode(Handle handle, std::uint32_t& index, std::uint32_t& generation);

        mutable std::mutex m_Mutex;
        std::vector<Entry> m_Entries;
        std::vector<std::uint32_t> m_FreeIndices;
        std::unordered_map<GameObject*, std::uint32_t> m_ObjectIndices;
    };
}
