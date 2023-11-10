#include "../headers/FilelistHandler.h"


std::string to_string(FileTypes type)   {
    switch (type)   {
        case FileTypes::FLAT:
            return "FLAT";
        case FileTypes::LIGHT:
            return "LIGHT";
        case FileTypes::DARK:
            return "DARK";
        case FileTypes::BIAS:
            return "BIAS";
        case FileTypes::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
};

FilelistHandler::FilelistHandler()   {
    m_filelist[FileTypes::FLAT]     = std::vector<std::string>();
    m_filelist[FileTypes::LIGHT]    = std::vector<std::string>();
    m_filelist[FileTypes::DARK]     = std::vector<std::string>();
    m_filelist[FileTypes::BIAS]     = std::vector<std::string>();
    m_filelist[FileTypes::UNKNOWN]  = std::vector<std::string>();
};

void FilelistHandler::add_file(const std::string& path, FileTypes type)  {
    m_filelist[type].push_back(path);
};

const std::vector<std::string>& FilelistHandler::get_files(FileTypes type)   {
    return m_filelist[type];
};