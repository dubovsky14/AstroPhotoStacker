#include "../headers/RecentPathsHandler.h"

#include "../../headers/Common.h"

#include <fstream>
#include <iostream>
#include <filesystem>


using namespace std;

RecentPathsHandler::RecentPathsHandler(const std::string &storage_folder)   :
    m_storage_path(storage_folder) {

};

std::string RecentPathsHandler::get_recent_file_path(AstroPhotoStacker::FrameType frame_type, const std::string &default_value)  const  {
    const std::string file_address = m_storage_path + "/" + m_frame_type_to_txt_file.at(frame_type);
    string line;
    ifstream input_file (file_address);
    if (input_file.is_open())    {
        while ( getline (input_file,line) )        {
            AstroPhotoStacker::strip_string(&line);
            if (line.length() != 0) {
                input_file.close();
                return line;
            }
        }
        input_file.close();
    }
    return default_value;
};

void RecentPathsHandler::set_recent_file_path(AstroPhotoStacker::FrameType frame_type, const std::string &recent_path, bool reduce_to_folder_only)     const {
    const std::string file_address = m_storage_path + "/" + m_frame_type_to_txt_file.at(frame_type);
    ofstream output_file(file_address);
    if (output_file.is_open())    {
        output_file << get_reduced_path(recent_path, reduce_to_folder_only);
    }
    output_file.close();
};

void RecentPathsHandler::set_recent_file_path_from_file(AstroPhotoStacker::FrameType frame_type, const std::string &recent_file_path) const    {
    // drop everything behind last "/"
    const size_t last_slash_idx = recent_file_path.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)    {
        const std::string recent_path = recent_file_path.substr(0, last_slash_idx);
        set_recent_file_path(frame_type, recent_path + "/", false);
    }
    else {
        set_recent_file_path(frame_type, "", false);
    }
};


std::string RecentPathsHandler::get_recent_file_path(RecentPathSettings recent_path_setting, const std::string &default_value)  const {
    const std::string file_address = m_storage_path + "/" + m_recent_path_setting_to_txt_file.at(recent_path_setting);
    string line;
    ifstream input_file (file_address);
    if (input_file.is_open())    {
        while ( getline (input_file,line) )        {
            AstroPhotoStacker::strip_string(&line);
            if (line.length() != 0) {
                input_file.close();
                return line;
            }
        }
        input_file.close();
    }
    return default_value;
};

void RecentPathsHandler::set_recent_file_path(RecentPathSettings recent_path_setting, const std::string &recent_path, bool reduce_to_folder_only) const {
    const std::string file_address = m_storage_path + "/" + m_recent_path_setting_to_txt_file.at(recent_path_setting);
    ofstream output_file(file_address);
    if (output_file.is_open())    {
        output_file << get_reduced_path(recent_path, reduce_to_folder_only);
    }
    output_file.close();
};

std::string RecentPathsHandler::get_reduced_path(const std::string &path, bool reduce_to_folder_only) {
    if (!reduce_to_folder_only) {
        return path;
    }
    std::filesystem::path fs_path(path);
    if (fs_path.has_filename()) {
        return fs_path.parent_path().string() + "/";
    }
    return path;
};