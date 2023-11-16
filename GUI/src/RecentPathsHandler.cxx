#include "../headers/RecentPathsHandler.h"

#include "../../headers/Common.h"

#include <fstream>
#include <iostream>


using namespace std;

RecentPathsHandler::RecentPathsHandler(const std::string &storage_folder)   :
    m_storage_path(storage_folder) {

};

std::string RecentPathsHandler::get_recent_file_path(FileTypes frame_type, const std::string &default_value)  const  {
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

void RecentPathsHandler::set_recent_file_path(FileTypes frame_type, const std::string &recent_path)     const {
    const std::string file_address = m_storage_path + "/" + m_frame_type_to_txt_file.at(frame_type);
    ofstream output_file(file_address);
    if (output_file.is_open())    {
        output_file << recent_path;
    }
    output_file.close();
};

void RecentPathsHandler::set_recent_file_path_from_file(FileTypes frame_type, const std::string &recent_file_path) const    {
    // drop everything behind last "/"
    const size_t last_slash_idx = recent_file_path.find_last_of("\\/");
    if (std::string::npos != last_slash_idx)    {
        const std::string recent_path = recent_file_path.substr(0, last_slash_idx);
        set_recent_file_path(frame_type, recent_path + "/");
    }
    else {
        set_recent_file_path(frame_type, "");
    }
};