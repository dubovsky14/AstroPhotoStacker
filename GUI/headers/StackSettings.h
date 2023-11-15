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
        bool is_kappa_sigma() const;

        // kappa-sigma options
        void  set_kappa(float kappa_sigma);
        float get_kappa() const;

        void set_kappa_sigma_iter(int kappa_sigma_iter);
        int  get_kappa_sigma_iter() const;

        // hot pixel correction
        void set_hot_pixel_correction(bool hot_pixel_correction);
        bool get_hot_pixel_correction() const;

    private:
        std::string m_alignment_file;
        std::string m_stacking_algorithm = "average";
        int m_n_cpus = get_max_threads();
        int m_max_memory = 8000;

        float m_kappa = 1.0;
        int   m_kappa_sigma_iter = 3;
        bool  m_hot_pixel_correction = false;

        const std::vector<std::string> m_stacking_algorithms = {"kappa-sigma median", "kappa-sigma mean", "average", "median", };

};