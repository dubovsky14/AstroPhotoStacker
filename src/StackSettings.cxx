#include "../headers/StackSettings.h"
#include "../headers/StackerFactory.h"

#include <stdexcept>
#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


const std::vector<std::string> StackSettings::m_stacking_algorithms({"kappa-sigma median", "kappa-sigma mean", "average", "median", "cut-off average", "maximum", "minimum", "best score", "center", "quantil"});

void StackSettings::set_alignment_frame(const AstroPhotoStacker::InputFrame& alignment_frame)       {
    m_alignment_frame = alignment_frame;
};

const AstroPhotoStacker::InputFrame& StackSettings::get_alignment_frame() const  {
    return m_alignment_frame;
};


void StackSettings::set_alignment_method(const std::string& alignment_method)   {
    m_alignment_method = alignment_method;
};

const std::string& StackSettings::get_alignment_method() const  {
    return m_alignment_method;
};

int StackSettings::get_max_threads() const     {
    return std::thread::hardware_concurrency();
};

void StackSettings::set_n_cpus(int n_cpus)      {
    m_n_cpus = n_cpus;
};

int  StackSettings::get_n_cpus() const  {
    return m_n_cpus;
};

void StackSettings::set_max_memory(int max_memory)      {
    m_max_memory = max_memory;
};

int StackSettings::get_max_memory() const      {
    return m_max_memory;
};

const std::vector<std::string>& StackSettings::get_stacking_algorithms() const {
    return m_stacking_algorithms;
};

void StackSettings::set_stacking_algorithm(const std::string& stacking_algorithm)      {
    if (std::find(m_stacking_algorithms.begin(), m_stacking_algorithms.end(), stacking_algorithm) != m_stacking_algorithms.end())    {
        m_stacking_algorithm = stacking_algorithm;
    }
    else    {
        throw std::invalid_argument("Stacking algorithm not found: \'" + stacking_algorithm + "\'");
    }
};

const std::string& StackSettings::get_stacking_algorithm() const       {
    return m_stacking_algorithm;
};

void StackSettings::set_hot_pixel_correction(bool hot_pixel_correction)    {
    m_hot_pixel_correction = hot_pixel_correction;
};

bool StackSettings::use_hot_pixel_correction() const   {
    return m_hot_pixel_correction;
};

void StackSettings::set_use_color_interpolation(bool use_color_interpolation)    {
    m_use_color_interpolation = use_color_interpolation;
};

bool StackSettings::use_color_interpolation() const   {
    return m_use_color_interpolation;
};

void StackSettings::set_apply_color_stretching(bool apply_color_stretching)    {
    m_apply_color_stretching = apply_color_stretching;
};

bool StackSettings::apply_color_stretching() const   {
    return m_apply_color_stretching;
};

std::vector<AdditionalStackerSetting> StackSettings::get_algorithm_specific_settings_defaults() const {
    std::unique_ptr<StackerBase> stacker = create_stacker(m_stacking_algorithm, m_n_cpus, 3, 2, m_use_color_interpolation);
    vector<string> keys = stacker->get_additional_setting_keys();
    std::vector<AdditionalStackerSetting> settings;
    for (const auto &key : keys) {
        settings.push_back(stacker->get_additional_setting(key));
    }
    return settings;
}