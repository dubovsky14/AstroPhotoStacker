#pragma once

#include "../../headers/InputFrame.h"

#include <string>
#include <vector>


/**
 * @brief Class responsible for grouping photos based on their time stamps - it can be used to group photos taken in a burst mode for a timelaps video
 */
class  PhotoGroupingTool {

    struct PhotoInfo {
        AstroPhotoStacker::InputFrame input_frame;
        int unix_timestamp;
        float score;
    };

    public:
        PhotoGroupingTool() = default;

        /**
         * @brief Add a file to the list of photos, which will be later grouped
         *
         * @param input_frame information about the input frame
         * @param unix_timestamp unix timestamp of the photo
         * @param score score of the photo (e.g. alignment score)
         */
        void add_file(const AstroPhotoStacker::InputFrame &input_frame, int unix_timestamp, float score);

        void define_maximum_time_difference_in_group(int time_interval);

        void set_group_by_files(bool group_by_files) {
            m_group_by_files = group_by_files;
        };

        bool get_group_by_files() const {
            return m_group_by_files;
        };

        void run_grouping();

        std::vector<std::vector<AstroPhotoStacker::InputFrame>> get_groups() const;

        const std::vector<std::vector<size_t>>& get_groups_indices() const;

    private:
        std::vector<PhotoInfo> m_photos;
        int m_time_interval = 0;
        std::vector<std::vector<size_t>> m_groups;

        bool m_group_by_files = false;
};