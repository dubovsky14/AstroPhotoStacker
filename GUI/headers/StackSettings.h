#pragma once

#include <string>
#include <thread>
#include <vector>

/**
 * @brief Class responsible for storing settings for the stacking process.
*/
class StackSettings {
    public:
        StackSettings() = default;
        ~StackSettings() = default;

        void set_alignment_file(const std::string& alignment_file);
        const std::string& get_alignment_file() const;

        // alignment method
        void set_alignment_method(const std::string& alignment_method);
        const std::string& get_alignment_method() const;

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

        // cut-off average options
        void set_cut_off_tail_fraction(float cut_off_tail_fraction);
        float get_cut_off_tail_fraction() const;

        // hot pixel correction
        void set_hot_pixel_correction(bool hot_pixel_correction);
        bool use_hot_pixel_correction() const;

        // color interpolation
        void set_use_color_interpolation(bool use_color_interpolation);
        bool use_color_interpolation() const;

        // color stretching
        void set_apply_color_stretching(bool apply_color_stretching);
        bool apply_color_stretching() const;

    private:
        std::string m_alignment_file;
        std::string m_stacking_algorithm = "kappa-sigma mean";
        int m_n_cpus = get_max_threads();
        int m_max_memory = 8000;

        float m_kappa = 1.0;
        int   m_kappa_sigma_iter = 3;
        float m_cut_off_tail_fraction = 0.2;
        bool  m_hot_pixel_correction = false;
        bool  m_use_color_interpolation = true;
        bool  m_apply_color_stretching = false;

        std::string m_alignment_method = "stars";

        const std::vector<std::string> m_stacking_algorithms = {"kappa-sigma median", "kappa-sigma mean", "average", "median", "cut-off average", "maximum", "minimum", "best score"};

};