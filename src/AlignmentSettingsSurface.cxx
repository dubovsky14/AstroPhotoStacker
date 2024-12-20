#include "../headers/AlignmentSettingsSurface.h"

#include <memory>
#include <mutex>

using namespace AstroPhotoStacker;
using namespace std;


std::unique_ptr<AlignmentSettingsSurface> AlignmentSettingsSurface::s_singleton_instance = nullptr;
std::mutex AlignmentSettingsSurface::s_initializer_mutex = {};

AlignmentSettingsSurface* AlignmentSettingsSurface::get_instance() {
    if (s_singleton_instance == nullptr) {
        std::scoped_lock lock(s_initializer_mutex);
        if (s_singleton_instance == nullptr) { // we need this check because another thread might have initialized the instance
            s_singleton_instance = std::unique_ptr<AlignmentSettingsSurface>(new AlignmentSettingsSurface());
        }
    }
    return s_singleton_instance.get();
}


float AlignmentSettingsSurface::get_contrast_threshold() const {
    return m_contrast_threshold;
}

void AlignmentSettingsSurface::set_contrast_threshold(float contrast_threshold) {
    m_contrast_threshold = contrast_threshold;
}


float AlignmentSettingsSurface::get_max_overlap_between_boxes() const {
    return m_max_overlap_between_boxes;
}

void AlignmentSettingsSurface::set_max_overlap_between_boxes(float max_overlap_between_boxes) {
    m_max_overlap_between_boxes = max_overlap_between_boxes;
}


int AlignmentSettingsSurface::get_number_of_boxes() const {
    return m_number_of_boxes;
}

void AlignmentSettingsSurface::set_number_of_boxes(int number_of_boxes) {
    m_number_of_boxes = number_of_boxes;
}

bool AlignmentSettingsSurface::get_regular_grid() const {
    return m_regular_grid;
};

void AlignmentSettingsSurface::set_regular_grid(bool regular_grid)  {
    m_regular_grid = regular_grid;
};