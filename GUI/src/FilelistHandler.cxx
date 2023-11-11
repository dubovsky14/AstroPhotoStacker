#include "../headers/FilelistHandler.h"

#include <algorithm>

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

FileTypes string_to_filetype(const std::string& type)   {
    if (type == "FLAT")     {
        return FileTypes::FLAT;
    }
    else if (type == "LIGHT")   {
        return FileTypes::LIGHT;
    }
    else if (type == "DARK")    {
        return FileTypes::DARK;
    }
    else if (type == "BIAS")    {
        return FileTypes::BIAS;
    }
    else    {
        return FileTypes::UNKNOWN;
    }
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

void FilelistHandler::remove_file(const std::string& path, FileTypes type)  {
    std::vector<std::string> &files = m_filelist[type];

    while (true)  {
        auto it = std::find(files.begin(), files.end(), path);
        if (it == files.end())  {
            break;
        }
        files.erase(it);
    }
};

const std::vector<std::string>& FilelistHandler::get_files(FileTypes type)   {
    return m_filelist[type];
};