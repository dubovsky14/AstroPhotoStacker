#pragma once

#include <memory>
#include <mutex>

namespace AstroPhotoStacker {
    class AlignmentSettingsSurface {
        public:
            static AlignmentSettingsSurface *get_instance();

            float get_maximal_allowed_distance_in_pixels() const;

            void set_maximal_allowed_distance_in_pixels(float distance);

            bool use_sift_detector() const;

            void set_use_sift_detector(bool use);

            float get_match_distance_threshold() const;

            void set_match_distance_threshold(float threshold);

        private:
            static std::unique_ptr<AlignmentSettingsSurface> s_singleton_instance;

            AlignmentSettingsSurface() = default;
            AlignmentSettingsSurface(const AlignmentSettingsSurface&) = delete;
            AlignmentSettingsSurface& operator=(const AlignmentSettingsSurface&) = delete;

            static std::mutex s_initializer_mutex;

            float m_maximal_allowed_distance_in_pixels = 20.0f;

            float m_match_distance_threshold = 200.0f;

            bool m_use_sift_detector = false;

    };
}