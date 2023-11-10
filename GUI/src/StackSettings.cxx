#include "../headers/StackSettings.h"
#include <stdexcept>
#include <algorithm>

void StackSettings::set_alignment_file(const std::string& alignment_file)       {
    m_alignment_file = alignment_file;
};

const std::string& StackSettings::get_alignment_file() const  {
    return m_alignment_file;
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
        throw std::invalid_argument("Stacking algorithm not found");
    }
};

const std::string& StackSettings::get_stacking_algorithm() const       {
    return m_stacking_algorithm;
};

bool StackSettings::is_kappa_sigma() const     {
    return m_stacking_algorithm == "kappa-sigma mean" || m_stacking_algorithm == "kappa-sigma median";
};

void  StackSettings::set_kappa(float kappa_sigma)       {
    m_kappa = kappa_sigma;
};

float StackSettings::get_kappa() const  {
    return m_kappa;
};

void StackSettings::set_kappa_sigma_iter(int kappa_sigma_iter)  {
    m_kappa_sigma_iter = kappa_sigma_iter;
};

int  StackSettings::get_kappa_sigma_iter() const        {
    return m_kappa_sigma_iter;
};