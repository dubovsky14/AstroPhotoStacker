#pragma once

#include "../headers/InputFrame.h"
#include "../headers/PixelType.h"

#include <vector>
#include <map>
#include <atomic>

namespace AstroPhotoStacker {
    struct FrameClusterInfo {
        std::vector< std::vector<std::tuple<int, int> > > clusters;
        std::vector<bool> clusters_selected;
        std::vector<float> clusters_excentricity;

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

            FrameClusterInfo get_cluster_info(const InputFrame &frame)  const;

            void recalculate_clusters(const InputFrame &frame, float cluster_fraction_threshold, bool buffer_brightness = false);

            void recalculate_clusters(const std::vector<InputFrame> &frames, float cluster_fraction_threshold);

            void clear_buffer();

            void clear_clusters();

            static void keep_clusters_with_at_least_n_pixels(std::vector< std::vector<std::tuple<int, int> > > *clusters, int min_n_pixels);

        private:
            std::map<InputFrame, FrameClusterInfo> m_frame_clusters_map;
            unsigned int m_n_cpus = 1;
            std::atomic<int> m_tasks_processed = 0;

            std::vector<PixelType>  m_brightness_buffer;
            int                     m_brightness_buffer_width = 0;
            int                     m_brightness_buffer_height = 0;
            InputFrame              m_frame_in_brightness_buffer;
    };
}