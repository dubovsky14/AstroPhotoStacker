#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/Common.h"

#include <fstream>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;

void PhotoAlignmentHandler::read_from_text_file(const std::string &alignment_file_address) {
    ifstream alignment_file(alignment_file_address);
    string line;
    while (getline(alignment_file, line)) {
        strip_string(&line);
        if (line.empty()) continue;
        if (line[0] == '#') continue; // Ignore comments (lines starting with #)

        vector<string> elements = split_and_strip_string(line, "|");
        if (starts_with(line, c_reference_file_header)) {
            if (elements.size() != 2) {
                throw runtime_error("Invalid reference file header.");
            }
            m_reference_file_address = elements[1];
            continue;
        }

        if (elements.size() != 6) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        if (!string_is_float(elements[1]) || !string_is_float(elements[2]) || !string_is_float(elements[3]) || !string_is_float(elements[4]) || !string_is_float(elements[5])) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        m_file_addresses.push_back(elements[0]);
        m_shift_x.push_back(stof(elements[1]));
        m_shift_y.push_back(stof(elements[2]));
        m_rotation_center_x.push_back(stof(elements[3]));
        m_rotation_center_y.push_back(stof(elements[4]));
        m_rotation.push_back(stof(elements[5]));
    }
    alignment_file.close();
}

void PhotoAlignmentHandler::save_to_text_file(const std::string &alignment_file_address)   {
    ofstream alignment_file(alignment_file_address);
    alignment_file << c_reference_file_header << " | " << m_reference_file_address << endl;
    alignment_file << "# File address | shift_x | shift_y | rotation_center_x | rotation_center_y | rotation" << endl;
    for (unsigned int i = 0; i < m_file_addresses.size(); i++) {
        alignment_file  << m_file_addresses[i]
                        << " | " << m_shift_x[i]
                        << " | " << m_shift_y[i]
                        << " | " << m_rotation_center_x[i]
                        << " | " << m_rotation_center_y[i]
                        << " | " << m_rotation[i] << endl;
    }
    alignment_file.close();
};

void PhotoAlignmentHandler::align_files(const std::string &reference_file_address, const std::vector<std::string> &files) {
    m_reference_photo_handler = make_unique<ReferencePhotoHandler>(reference_file_address, 0.0005);
    m_reference_file_address = reference_file_address;
    for (const std::string &file : files)   {
        float shift_x, shift_y, rot_center_x, rot_center_y, rotation;
        if (m_reference_photo_handler->plate_solve(file, &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation)) {
            m_file_addresses.push_back(file);
            m_shift_x.push_back(shift_x);
            m_shift_y.push_back(shift_y);
            m_rotation_center_x.push_back(rot_center_x);
            m_rotation_center_y.push_back(rot_center_y);
            m_rotation.push_back(rotation);
        }
    }
};

void PhotoAlignmentHandler::align_all_files_in_folder(const std::string &reference_file_address, const std::string &raw_files_folder) {
    vector<string> files;
    for (const auto & entry : filesystem::directory_iterator(raw_files_folder)) {
        if (entry.path().extension() == ".txt") {
            continue;
        }
        files.push_back(entry.path());
    }
    sort(files.begin(), files.end());
    align_files(reference_file_address, files);
}

void PhotoAlignmentHandler::reset() {
    m_file_addresses.clear();
    m_shift_x.clear();
    m_shift_y.clear();
    m_rotation_center_x.clear();
    m_rotation_center_y.clear();
    m_rotation.clear();
    m_reference_photo_handler = nullptr;
}

void PhotoAlignmentHandler::get_alignment_parameters(const std::string &file_address, float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const    {
    for (unsigned int i = 0; i < m_file_addresses.size(); i++) {
        if (m_file_addresses[i] == file_address) {
            *shift_x = m_shift_x[i];
            *shift_y = m_shift_y[i];
            *rot_center_x = m_rotation_center_x[i];
            *rot_center_y = m_rotation_center_y[i];
            *rotation = m_rotation[i];
            return;
        }
    }
    throw runtime_error("File not found in alignment file: " + file_address);
}