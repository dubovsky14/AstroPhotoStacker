#pragma once

#include "../headers/InputFrame.h"
#include "../headers/PixelType.h"

#include "../headers/FilelistHandler.h"
#include "../headers/CalibrationFrameBase.h"

#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <memory>

namespace AstroPhotoStacker {
    struct FrameAndGroup    {
        InputFrame  input_frame;
        int         group_number = 0;

        // needed to use FrameAndGroup as a key in a map
        bool operator<(const FrameAndGroup &other) const {
            if (group_number != other.group_number) {
                return group_number < other.group_number;
            }
            return input_frame < other.input_frame;
        };

        bool operator>(const FrameAndGroup &other) const {
            if (group_number != other.group_number) {
                return group_number > other.group_number;
            }
            return input_frame > other.input_frame;
        };

        bool operator==(const FrameAndGroup &other) const {
            return group_number == other.group_number && input_frame == other.input_frame;
        };

        bool operator!=(const FrameAndGroup &other) const {
            return !(*this == other);
        };
    };

    struct FrameClusterInfo {
        std::vector< std::vector<std::tuple<int, int> > > clusters;
        std::vector<bool> clusters_selected;
        std::vector<float> clusters_excentricity;
        std::vector<float> clusters_correlation;

        float cluster_fraction_threshold = 0.0;
    };


    class MeteorShowerStackingTool  {
        public:
            MeteorShowerStackingTool() = default;
            ~MeteorShowerStackingTool() = default;

            void set_n_cpus(int n_cpus) {
                m_n_cpus = n_cpus;
            };

            /**
             * @brief Get the number of tasks processed so far
             *
             * @return const std::atomic<int>& - number of tasks processed so far
            */
            const std::atomic<int>& get_tasks_processed() const {return m_tasks_processed;};

            FrameClusterInfo get_cluster_info(const FrameAndGroup &frame)  const;

            void set_cluster_selected(const FrameAndGroup &frame, size_t cluster_id, bool selected);

            void recalculate_clusters(const FrameAndGroup &frame, float cluster_fraction_threshold, bool buffer_brightness = false);

            void recalculate_clusters(const std::vector<FrameAndGroup> &frames, float cluster_fraction_threshold);

            void clear_buffer();

            void clear_clusters();

            static void keep_clusters_with_at_least_n_pixels(std::vector< std::vector<std::tuple<int, int> > > *clusters, int min_n_pixels);

            const std::vector<std::vector<float>> &get_stacked_image(int *width, int *height);

            void stack_frames(const FilelistHandler &filelist_handler, const FrameAndGroup &background_frame);

        private:
            std::map<FrameAndGroup, FrameClusterInfo> m_frame_clusters_map;
            unsigned int m_n_cpus = 1;
            std::atomic<int> m_tasks_processed = 0;

            std::vector<PixelType>  m_brightness_buffer;
            int                     m_brightness_buffer_width = 0;
            int                     m_brightness_buffer_height = 0;
            FrameAndGroup           m_frame_in_brightness_buffer;

            std::vector<std::vector<float>> m_stacked_result_data;
            int                             m_stacked_result_width = 0;
            int                             m_stacked_result_height = 0;
            std::mutex                      m_stacking_mutex;

            void process_one_frame(FrameAndGroup frame, const FilelistHandler &filelist_handler, const std::vector<std::shared_ptr<const CalibrationFrameBase>> &calibration_frames);

            static std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase>>>   get_calibration_frames_map(const FilelistHandler &filelist_handler);

    };
}