#pragma once

#include "../headers/FilelistHandler.h"

#include <string>
#include <map>


class RecentPathsHandler    {
    public:
        RecentPathsHandler(const std::string &storage_folder = "../data/recent_paths/");

        std::string get_recent_file_path(FileTypes frame_type, const std::string &default_value = "")  const;

        void set_recent_file_path(FileTypes frame_type, const std::string &recent_path) const;

        void set_recent_file_path_from_file(FileTypes frame_type, const std::string &recent_file_path) const;

    private:
        std::string m_storage_path;

        std::map<FileTypes, std::string> m_frame_type_to_txt_file = std::map<FileTypes, std::string>({
            {FileTypes::FLAT ,       "recent_flat_paths.txt"},
            {FileTypes::LIGHT ,      "recent_lights_paths.txt"},
            {FileTypes::DARK ,       "recent_dark_paths.txt"},
            {FileTypes::BIAS ,       "recent_bias_paths.txt"},
            {FileTypes::UNKNOWN ,    "recent_unknown_paths.txt"},
        });
};
