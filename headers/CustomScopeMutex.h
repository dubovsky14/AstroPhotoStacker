#pragma once

#include <mutex>

namespace AstroPhotoStacker {

    /**
     * @brief A class that tries to lock a mutex in its constructor and unlocks it in its destructor.
    */
    class CustomScopeMutex {
        public:

            /**
             * @brief Construct a new CustomScopeMutex object
             *
             * @param mutex The mutex to lock
            */
            CustomScopeMutex(std::mutex *mutex) : m_mutex(mutex) {
                m_is_locked = m_mutex->try_lock();
            }

            CustomScopeMutex()                          = delete;
            CustomScopeMutex(const CustomScopeMutex&)   = delete;

            /**
             * @brief Check if the mutex is locked
             *
             * @return true if the mutex is locked
             * @return false if the mutex is not locked
            */
            bool is_locked() const {
                return m_is_locked;
            }

            /**
             * @brief CustomScopeMutex descturctor - unlock the mutex if locked before
            */
            ~CustomScopeMutex() {
                if (m_is_locked)   {
                    m_mutex->unlock();
                }
            }

        private:
            std::mutex *m_mutex;
            bool m_is_locked = false;
    };
}