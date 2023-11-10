#pragma once

#include <string>
#include <thread>
#include <vector>

class StackSettings {
    public:
        StackSettings() = default;
        ~StackSettings() = default;

        void set_alignment_file(const std::string& alignment_file);
        const std::string& get_alignment_file() const;

        // nCPU
        int get_max_threads() const;
        void set_n_cpus(int n_cpus);
        int get_n_cpus() const;

        // max memory
        void set_max_memory(int max_memory);
        int get_max_memory() const;

        // stacking algorithm
        const std::vector<std::string>& get_stacking_algorithms() const;
        void set_stacking_algorithm(const std::string& stacking_algorithm);
        const std::string& get_stacking_algorithm() const;

    private:
        std::string m_alignment_file;
        std::string m_stacking_algorithm;
        int m_n_cpus = get_max_threads();
        int m_max_memory = 8000;

        const std::vector<std::string> m_stacking_algorithms = {"average", "median", "kappa-sigma mean", "kappa-sigma median"};

};