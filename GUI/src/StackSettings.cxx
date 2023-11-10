#include "../headers/StackSettings.h"

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