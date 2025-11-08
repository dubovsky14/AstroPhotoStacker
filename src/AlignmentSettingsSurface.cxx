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


float AlignmentSettingsSurface::get_maximal_allowed_distance_in_pixels() const {
    return m_maximal_allowed_distance_in_pixels;
};

void AlignmentSettingsSurface::set_maximal_allowed_distance_in_pixels(float distance) {
    m_maximal_allowed_distance_in_pixels = distance;
};

bool AlignmentSettingsSurface::use_sift_detector() const {
    return m_use_sift_detector;
};

void AlignmentSettingsSurface::set_use_sift_detector(bool use) {
    m_use_sift_detector = use;
};