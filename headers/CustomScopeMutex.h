#pragma once

#include <mutex>

namespace AstroPhotoStacker {
    class CustomScopeMutex {
        public:
            CustomScopeMutex(std::mutex *mutex) : m_mutex(mutex) {
                m_is_locked = m_mutex->try_lock();
            }

            CustomScopeMutex()                          = delete;
            CustomScopeMutex(const CustomScopeMutex&)   = delete;

            bool is_locked() const {
                return m_is_locked;
            }

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