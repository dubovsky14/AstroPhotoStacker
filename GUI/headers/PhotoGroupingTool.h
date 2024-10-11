#pragma once

#include <string>
#include <vector>


/**
 * @brief Class responsible for grouping photos based on their time stamps - it can be used to group photos taken in a burst mode for a timelaps video
 */
class  PhotoGroupingTool {

    struct PhotoInfo {
        std::string file_address;
        int unix_timestamp;
        float score;
    };

    public:
        PhotoGroupingTool() = default;

        /**
         * @brief Add a file to the list of photos, which will be later grouped
         *
         * @param file_address address of the file
         * @param unix_timestamp unix timestamp of the photo
         * @param score score of the photo (e.g. alignment score)
         */
        void add_file(const std::string &file_address, int unix_timestamp, float score);

        void define_maximum_time_difference_in_group(int time_interval);

        void run_grouping();

        std::vector<std::vector<std::string>> get_groups() const;

    private:
        std::vector<PhotoInfo> m_photos;
        int m_time_interval = 0;
        std::vector<std::vector<PhotoInfo>> m_groups;
};