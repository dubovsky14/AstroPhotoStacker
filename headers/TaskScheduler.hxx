#include <mutex>
#include <future>
#include <vector>
#include <functional>
#include <stdexcept>
#include <atomic>

namespace AstroPhotoStacker {
    class TaskScheduler {
        public:
            /**
             * @brief Construct a new Task Scheduler object
             *
             * @param resource_limits The limits of the resources that can be used by the tasks executed in paralel. For example 1st element might by number of available CPU cores, 2nd element might be the amount of memory available, etc.
             */
            TaskScheduler(const std::vector<size_t> &resource_limits)   :
                m_resource_limits(resource_limits),
                m_resource_usage(resource_limits.size(), 0)   {

                    m_maintain_tasks_future = std::async(std::launch::async, [this] {
                        maintain_tasks();
                    });
            };

            TaskScheduler()                     = delete;
            TaskScheduler(const TaskScheduler&) = delete;

            void submit(const std::function<void()> &task, const std::vector<size_t> &resource_requirements)    {
                check_resources_limit_correctness(resource_requirements);

                std::scoped_lock lock(m_mutex);

                const bool resources_available = enough_resources(resource_requirements);
                if (resources_available) {
                    allocate_resources(resource_requirements);
                    m_futures_and_requirements.push_back({std::async(std::launch::async, task), resource_requirements});
                }
                else {
                    m_remaining_tasks_and_requirements.push_back({task, resource_requirements});
                }

            };

            void wait_for_tasks()   {
                while (m_running)   {
                    std::this_thread::sleep_for(std::chrono::microseconds(m_sleep_time));

                    std::scoped_lock lock(m_mutex);

                    if (m_futures_and_requirements.empty() && m_remaining_tasks_and_requirements.empty()) {
                        m_running = false;
                    }
                }
            };

        private:
            std::mutex m_mutex;
            std::vector<size_t> m_resource_limits;
            std::vector<size_t> m_resource_usage;
            std::vector<std::pair<std::future<void>, std::vector<size_t>>>      m_futures_and_requirements;
            std::vector<std::pair<std::function<void()>, std::vector<size_t>>>  m_remaining_tasks_and_requirements;
            std::atomic<bool>   m_running = true;
            int m_sleep_time = 100;
            std::future<void>   m_maintain_tasks_future;


            void maintain_tasks() {
                while (m_running)   {
                    std::this_thread::sleep_for(std::chrono::microseconds(m_sleep_time));

                    std::scoped_lock lock(m_mutex);

                    // check if any task finished
                    for (size_t i_task = 0; i_task < m_futures_and_requirements.size(); i_task++) {
                        if (m_futures_and_requirements[i_task].first.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                            release_resources(m_futures_and_requirements[i_task].second);
                            m_futures_and_requirements.erase(m_futures_and_requirements.begin() + i_task);
                            i_task--;
                        }
                    }

                    // check if any task can be executed now
                    for (size_t i_task = 0; i_task < m_remaining_tasks_and_requirements.size(); i_task++) {
                        auto &task = m_remaining_tasks_and_requirements[i_task].first;
                        auto &resource_requirements = m_remaining_tasks_and_requirements[i_task].second;
                        if (enough_resources(resource_requirements)) {
                            allocate_resources(resource_requirements);
                            m_futures_and_requirements.push_back({std::async(std::launch::async, task), resource_requirements});
                            m_remaining_tasks_and_requirements.erase(m_remaining_tasks_and_requirements.begin() + i_task);
                            i_task--;
                        }
                    }

                }
            };

            /**
             * @brief Check if there are enough resources to execute the task now
             *
             * @param resource_requirements The requirements of the resources for the task
             * @return true if there are enough resources to execute the task now
             */
            bool enough_resources(const std::vector<size_t> &resource_requirements) const   {
                for (size_t i = 0; i < resource_requirements.size(); i++) {
                    if (m_resource_usage[i] + resource_requirements[i] > m_resource_limits[i]) {
                        return false;
                    }
                }

                return true;
            };

            void allocate_resources(const std::vector<size_t> &resource_requirements) {
                for (size_t i = 0; i < resource_requirements.size(); i++) {
                    m_resource_usage[i] += resource_requirements[i];
                }
            };

            void release_resources(const std::vector<size_t> &resource_requirements) {
                for (size_t i = 0; i < resource_requirements.size(); i++) {
                    m_resource_usage[i] -= resource_requirements[i];
                }
            };

            void check_resources_limit_correctness(const std::vector<size_t> &resource_requirements) const {
                if (resource_requirements.size() != m_resource_limits.size()) {
                    throw std::runtime_error("The number of resource requirements must match the number of resource limits requirements");
                }

                for (size_t i = 0; i < resource_requirements.size(); i++) {
                    if (resource_requirements[i] > m_resource_limits[i]) {
                        throw std::runtime_error("The resource requirements exceed the resource limits");
                    }
                }
            };

    };
}