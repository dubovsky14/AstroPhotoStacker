#pragma once

#include "../headers/FilelistHandler.h"

#include <string>
#include <map>

/**
 * @brief Class for handling addresses of the most recently used folders for each image type. It stores its data in text files, so that the paths are not lost after closing the application.
*/
class RecentPathsHandler    {
    public:
        /**
         * @brief Construct a new Recent Paths Handler object
         *
         * @param storage_folder path to the folder where the text files with the paths are stored
        */
        RecentPathsHandler(const std::string &storage_folder);

        /**
         * @brief Get the recent file path
         *
         * @param frame_type type of the frame
         * @param default_value default value to be returned if the path is not found
         * @return std::string path to the most recently used folder
        */
        std::string get_recent_file_path(FileTypes frame_type, const std::string &default_value = "")  const;

        /**
         * @brief Set the recent file path
         *
         * @param frame_type type of the frame
         * @param recent_path path to the most recently used folder
        */
        void set_recent_file_path(FileTypes frame_type, const std::string &recent_path) const;

        /**
         * @brief Set the recent file path from file
         *
         * @param frame_type type of the frame
         * @param recent_file_path path to the most recently use file
        */
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
