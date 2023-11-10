#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>

enum class FileTypes    {
    FLAT,
    LIGHT,
    DARK,
    BIAS,
    UNKNOWN
};

std::string to_string(FileTypes type);

class FilelistHandler   {
    public:
        FilelistHandler();
        ~FilelistHandler() = default;

        void add_file(const std::string& path, FileTypes type);

        const std::vector<std::string> &get_files(FileTypes type);

    private:
        std::map<FileTypes, std::vector<std::string>> m_filelist;

};