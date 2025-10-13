#pragma once

#include "../headers/CustomSharedMutex.h"

#include <string>
#include <map>
#include <shared_mutex>
#include <functional>

namespace AstroPhotoStacker {
    template<typename Key, typename Value>
    class ThreadSafeCacheSystem {
        public:
            ThreadSafeCacheSystem() = default;

            Value get(const Key &key, std::function<Value()> value_loader) {
                {
                    // Lock the mutex in shared (read-only) mode
                    SharedMutexRAIIShared scoped_lock(&m_mutex);

                    // Check if the value is already in the cache
                    auto it = m_cache.find(key);
                    if (it != m_cache.end()) {
                        return it->second;
                    }
                }

                // Value is not yet in the cache, so we need to load it
                Value value = value_loader();

                // Lock the mutex in exclusive (write) mode
                SharedMutexRAIIExclusive scoped_lock(&m_mutex);
                m_cache[key] = value;

                return value;
            }

        private:
            std::map<Key, Value>    m_cache;
            std::shared_mutex       m_mutex;
    };
}