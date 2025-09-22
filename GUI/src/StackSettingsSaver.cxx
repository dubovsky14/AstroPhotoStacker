#include "../headers/StackSettingsSaver.h"
#include "../../headers/Common.h"

#include <fstream>
#include <iostream>

using namespace std;

StackSettingsSaver::StackSettingsSaver(const std::string &filename) : AstroPhotoStacker::StackSettings() {
    m_filename = filename;
    load();
};

StackSettingsSaver::~StackSettingsSaver() {
    save();
};

void StackSettingsSaver::save() {
    ofstream file(m_filename);

    if (file.is_open()) {
        file << m_dict_string_n_cpus << m_separator << get_n_cpus() << endl;
        file << m_dict_string_max_memory << m_separator << get_max_memory() << endl;
        file << m_dict_string_stacking_algorithm << m_separator << get_stacking_algorithm() << endl;
        file << m_dict_string_hot_pixel_correction << m_separator << use_hot_pixel_correction() << endl;
        file << m_dict_string_use_color_interpolation << m_separator << use_color_interpolation() << endl;
        file << m_dict_string_apply_color_stretching << m_separator << apply_color_stretching() << endl;
    }
    file.close();
};

void StackSettingsSaver::load() {

    // Set default values
    set_n_cpus((get_max_threads()+1)/2);


    ifstream file(m_filename);
    string line;
    if (file.is_open()) {
        while (getline(file, line)) {
            vector<string> tokens = AstroPhotoStacker::split_string(line, m_separator);
            if (tokens.size() != 2) {
                continue;
            }
            const string &key = tokens[0];
            const string &value = tokens[1];

            if (key == m_dict_string_n_cpus) {
                set_n_cpus(stoi(value));
            }
            else if (key == m_dict_string_max_memory) {
                set_max_memory(stoi(value));
            }
            else if (key == m_dict_string_stacking_algorithm) {
                set_stacking_algorithm(value);
            }
            else if (key == m_dict_string_hot_pixel_correction) {
                set_hot_pixel_correction(value == "1");
            }
            else if (key == m_dict_string_use_color_interpolation) {
                set_use_color_interpolation(value == "1");
            }
            else if (key == m_dict_string_apply_color_stretching) {
                set_apply_color_stretching(value == "1");
            }
            else {
                cout << "Unknown key: " << key << endl;
            }
        }
    }
    file.close();
};

