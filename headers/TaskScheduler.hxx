#pragma once

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
            };

            TaskScheduler()                     = delete;
            TaskScheduler(const TaskScheduler&) = delete;

            /**
             * @brief Submit a task to be executed in parallel
             *
             * @param task - the function to be executed
             * @param release_resources - the resources that will be used by the task
             * @param args - the arguments of the task
             */
            template<typename FunctionType, typename... Args>
            void submit(const FunctionType &task, const std::vector<size_t> &resource_requirements, const Args &...args)   {

                std::scoped_lock lock(m_mutex);

                const size_t this_task_id = m_last_task_id++;
                check_resources_limit_correctness(resource_requirements);
                std::function<void()> task_wrapped = [this, task, resource_requirements, this_task_id, args...]() {
                    try {
                        task(args...);
                        std::scoped_lock lock(m_mutex);
                        release_resources(resource_requirements);
                        submit_task_from_buffer();
                    }
                    catch(...) {
                        std::scoped_lock lock(m_mutex);
                        release_resources(resource_requirements);
                        submit_task_from_buffer();
                        throw;
                    }
                };

                const bool resources_available = enough_resources(resource_requirements);
                if (resources_available) {
                    allocate_resources(resource_requirements);
                    m_futures_and_requirements[this_task_id] = {std::async(std::launch::async, task_wrapped), resource_requirements};
                }
                else {
                    m_remaining_tasks_and_requirements[this_task_id] = {task_wrapped, resource_requirements};
                }

            };

            /**
             * @brief Wait for all tasks to finish
             *
             * @param sleep_time The time to sleep between checking if the tasks are finished
             */
            void wait_for_tasks(int sleep_time = 100) {
                while (true)   {
                    std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));

                    std::scoped_lock lock(m_mutex);

                    if (m_remaining_tasks_and_requirements.empty()) {
                        break;
                    }
                }
            };

        private:
            std::mutex m_mutex;
            std::vector<size_t> m_resource_limits;
            std::vector<size_t> m_resource_usage;
            std::map<size_t, std::pair<std::future<void>, std::vector<size_t>>>                 m_futures_and_requirements;
            std::unordered_map<size_t, std::pair<std::function<void()>, std::vector<size_t>>>   m_remaining_tasks_and_requirements;
            size_t m_last_task_id = 0;

            void submit_task_from_buffer() {
                // check if any task can be executed now
                std::vector<size_t> tasks_to_remove;
                for (const auto &[i_task, task_and_resource_requirements] : m_remaining_tasks_and_requirements) {

                    const std::function<void()> &task                   = task_and_resource_requirements.first;
                    const std::vector<size_t> &resource_requirements    = task_and_resource_requirements.second;

                    if (enough_resources(resource_requirements)) {
                        allocate_resources(resource_requirements);
                        m_futures_and_requirements[i_task] = {std::async(std::launch::async, task), resource_requirements};
                        tasks_to_remove.push_back(i_task);
                    }
                }

                for (size_t i_task : tasks_to_remove) {
                    m_remaining_tasks_and_requirements.erase(i_task);
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