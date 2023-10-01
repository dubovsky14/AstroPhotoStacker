#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/Common.h"

#include <fstream>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;

void PhotoAlignmentHandler::ReadFromTextFile(const std::string &alignment_file_address) {
    ifstream alignment_file(alignment_file_address);
    string line;
    while (getline(alignment_file, line)) {
        if (line[0] == '#') continue; // Ignore comments (lines starting with #)
        if (StartsWith(line, c_reference_file_header)) {
            m_reference_file_address = line.substr(c_reference_file_header.length()+3);
        }
        if (line.empty()) continue;
        vector<string> tokens;
        string token;
        istringstream token_stream(line);
        while (getline(token_stream, token, '|')) {
            tokens.push_back(token);
        }
        m_file_addresses.push_back(tokens[0]);
        m_shift_x.push_back(stof(tokens[1]));
        m_shift_y.push_back(stof(tokens[2]));
        m_rotation_center_x.push_back(stof(tokens[3]));
        m_rotation_center_y.push_back(stof(tokens[4]));
        m_rotation.push_back(stof(tokens[5]));
    }
    alignment_file.close();
}

void PhotoAlignmentHandler::SaveToTextFile(const std::string &alignment_file_address)   {
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

void PhotoAlignmentHandler::AlignFiles(const std::string &reference_file_address, const std::vector<std::string> &files) {
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

void PhotoAlignmentHandler::AlignAllFilesInFolder(const std::string &reference_file_address, const std::string &raw_files_folder) {
    vector<string> files;
    for (const auto & entry : filesystem::directory_iterator(raw_files_folder)) {
        if (entry.path().extension() == ".txt") {
            continue;
        }
        files.push_back(entry.path());
    }
    sort(files.begin(), files.end());
    AlignFiles(reference_file_address, files);
}

void PhotoAlignmentHandler::Reset() {
    m_file_addresses.clear();
    m_shift_x.clear();
    m_shift_y.clear();
    m_rotation_center_x.clear();
    m_rotation_center_y.clear();
    m_rotation.clear();
    m_reference_photo_handler = nullptr;
}