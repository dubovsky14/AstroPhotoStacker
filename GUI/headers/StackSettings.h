#pragma once

#include <string>
#include <thread>

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

        // stacking algorithm
        //void set_stacking_algorithm(const std::string& stacking_algorithm);
        //const std::string& get_stacking_algorithm() const;

    private:
        std::string m_alignment_file;
        int m_n_cpus = get_max_threads();

};