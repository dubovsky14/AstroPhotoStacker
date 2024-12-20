#pragma once

#include <memory>
#include <mutex>

namespace AstroPhotoStacker {
    class AlignmentSettingsSurface {
        public:
            static AlignmentSettingsSurface *get_instance();

            float get_contrast_threshold() const;

            void set_contrast_threshold(float contrast_threshold);

            float get_max_overlap_between_boxes() const;

            void set_max_overlap_between_boxes(float max_overlap_between_boxes);

            int get_number_of_boxes() const;

            void set_number_of_boxes(int number_of_boxes);

            bool get_regular_grid() const;

            void set_regular_grid(bool regular_grid);

        private:
            static std::unique_ptr<AlignmentSettingsSurface> s_singleton_instance;

            AlignmentSettingsSurface() = default;
            AlignmentSettingsSurface(const AlignmentSettingsSurface&) = delete;
            AlignmentSettingsSurface& operator=(const AlignmentSettingsSurface&) = delete;

            static std::mutex s_initializer_mutex;

            float m_contrast_threshold        = 0.4;
            float m_max_overlap_between_boxes = 0.3;
            int   m_number_of_boxes           = 100;
            bool  m_regular_grid              = true;

    };
}