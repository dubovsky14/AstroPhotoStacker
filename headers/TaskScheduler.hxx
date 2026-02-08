#pragma once

#include <mutex>
#include <future>
#include <vector>
#include <functional>
#include <stdexcept>
#include <atomic>
#include <thread>
#include <map>
#include <iostream>
#include <memory>

namespace AstroPhotoStacker {
    class TaskScheduler {
        public:
            /**
             * @brief Construct a new Task Scheduler object
             *
             * @param resource_limits The limits of the resources that can be used by the tasks executed in paralel. For example 1st element might by number of available CPU cores, 2nd element might be the amount of memory available, etc.
             * @param ignore_exceptions_in_tasks If true, the TaskScheduler will ignore exceptions thrown in tasks and will continue executing remaining tasks. Otherwise, "wait_for_tasks" will rethrow the first exception occuring in the tasks.
             */
            TaskScheduler(const std::vector<size_t> &resource_limits, bool ignore_exceptions_in_tasks = false)   :
                m_resource_limits(resource_limits),
                m_resource_usage(resource_limits.size(), 0),
                m_ignore_exceptions_in_tasks(ignore_exceptions_in_tasks)   {
            };

            ~TaskScheduler()     {
                if (!*m_active_exception) {
                    m_ignore_exceptions_in_tasks = true; // otherwise we might get exceptions in destructor if there are still tasks running
                    wait_for_tasks();
                }
            }

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
            size_t submit(const FunctionType &task, const std::vector<size_t> &resource_requirements, const Args &...args)   {
                m_n_tasks_remaining++;
                std::scoped_lock lock(m_mutex);

                const size_t this_task_id = m_last_task_id++;
                check_resources_limit_correctness(resource_requirements);
                std::shared_ptr<std::atomic<bool>> active_exception = m_active_exception;
                std::function<void()> task_wrapped = [this, task, resource_requirements, this_task_id, active_exception, args...]() {
                    try {
                        task(args...);
                        // if there was an exception in another task, we don't want to start new tasks and we cannot touch member variables - the destructor of TaskSchedulermight have already been called
                        if (*active_exception) {
                            return;
                        }
                        std::scoped_lock lock(m_mutex);
                        release_resources(resource_requirements);
                        m_n_tasks_remaining--;

                        submit_task_from_buffer();
                    }
                    catch(...) {
                        std::scoped_lock lock(m_mutex);

                        // if there was an exception in another task, we just need to terminate task - using any memeber varaible is unsafe - the destructor of TaskSchedulermight have already been called
                        if (*active_exception) {
                            return;
                        }
                        release_resources(resource_requirements);
                        m_n_tasks_remaining--;
                        if (m_ignore_exceptions_in_tasks) {
                            submit_task_from_buffer();
                            return;
                        }
                        *active_exception = true;
                        m_task_with_active_exception = this_task_id;
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
                return this_task_id;
            };

            /**
             * @brief Wait for all tasks to finish
             *
             * @param sleep_time_ms The time to sleep between checking if the tasks are finished, in milliseconds
             */
            void wait_for_tasks(int sleep_time_ms = 100) {
                while (true)   {
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));

                    std::scoped_lock lock(m_mutex);

                    if (!m_ignore_exceptions_in_tasks && *m_active_exception) {
                        try {
                            m_futures_and_requirements.at(m_task_with_active_exception).first.get();
                        }
                        catch (...) {
                            m_ignore_exceptions_in_tasks = true; // otherwise we would get another exception while handling this one
                            throw;
                        }
                    }

                    if (m_n_tasks_remaining == 0)   {
                        break;
                    }
                }
            };

            /**
             * @brief Wait for all tasks to finish
             *
             * @param exception_handler The function that will be called when an exception is thrown in any of the tasks. This function can be used to wrap the exception (with additional information) of trigger something.
             * @param sleep_time_ms The time to sleep between checking if the tasks are finished, in milliseconds
             */
            void wait_for_tasks_and_modify_exceptions(std::function<void(size_t)> exception_handler, int sleep_time_ms = 100) {
                while (true)   {
                    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_ms));

                    std::scoped_lock lock(m_mutex);

                    if (!m_ignore_exceptions_in_tasks && *m_active_exception) {
                        try {
                            m_futures_and_requirements.at(m_task_with_active_exception).first.get();
                        }
                        catch (...) {
                            m_ignore_exceptions_in_tasks = true; // otherwise we would get another exception while handling this one
                            const size_t task_id = m_task_with_active_exception;
                            exception_handler(task_id);
                        }
                    }

                    if (m_n_tasks_remaining == 0)   {
                        break;
                    }
                }
            };

            /**
             * @brief Get the number of tasks that are still running or waiting to be executed
             * @return size_t The number of tasks that are still running or waiting to be executed
             */
            size_t get_tasks_remaining() const {
                return m_n_tasks_remaining;
            };

        private:
            std::mutex m_mutex;

            // this must outlive the TaskScheduler object in case of exception
            std::shared_ptr<std::atomic<bool>> m_active_exception = std::make_shared<std::atomic<bool>>(false);
            size_t m_task_with_active_exception = 0;

            std::vector<size_t> m_resource_limits;
            std::vector<size_t> m_resource_usage;
            bool m_ignore_exceptions_in_tasks;
            std::map<size_t, std::pair<std::future<void>, std::vector<size_t>>>                 m_futures_and_requirements;
            std::unordered_map<size_t, std::pair<std::function<void()>, std::vector<size_t>>>   m_remaining_tasks_and_requirements;
            size_t m_last_task_id = 0;
            std::atomic<size_t> m_n_tasks_remaining = 0;

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