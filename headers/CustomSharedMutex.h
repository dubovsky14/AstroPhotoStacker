#pragma once

#include <shared_mutex>

namespace AstroPhotoStacker {

    /**
     * RAII class for shared mutex. Constructor locks the mutex in shared mode (read mode),
     * and destructor unlocks it.
     */
    class   SharedMutexRAIIShared  {
        public:
            SharedMutexRAIIShared(std::shared_mutex *mutex) : m_mutex(mutex) {
                m_mutex->lock_shared();
            }

            ~SharedMutexRAIIShared() {
                m_mutex->unlock_shared();
            }

        private:
            std::shared_mutex *m_mutex;

    };

    /**
     * RAII class for shared mutex. Constructor locks the mutex in exclusive mode (write mode),
     * and destructor unlocks it.
     */
    class   SharedMutexRAIIExclusive  {
        public:
            SharedMutexRAIIExclusive(std::shared_mutex *mutex) : m_mutex(mutex) {
                m_mutex->lock();
            }

            ~SharedMutexRAIIExclusive() {
                m_mutex->unlock();
            }

        private:
            std::shared_mutex *m_mutex;
    };
}