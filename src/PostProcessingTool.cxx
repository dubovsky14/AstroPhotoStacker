#include "../headers/PostProcessingTool.h"

using namespace std;
using namespace AstroPhotoStacker;


void PostProcessingTool::set_apply_sharpening(bool apply_sharpening) {
    m_apply_sharpening = apply_sharpening;
};

bool PostProcessingTool::get_apply_sharpening() const {
    return m_apply_sharpening;
};

void PostProcessingTool::set_kernel_size(int kernel_size) {
    m_kernel_size = kernel_size;
};

int PostProcessingTool::get_kernel_size() const {
    return m_kernel_size;
};

void PostProcessingTool::set_gauss_width(float gauss_width) {
    m_gauss_width = gauss_width;
};

float PostProcessingTool::get_gauss_width() const {
    return m_gauss_width;
};

void PostProcessingTool::set_center_value(float center_value) {
    m_center_value = center_value;
};

float PostProcessingTool::get_center_value() const {
    return m_center_value;
};

void PostProcessingTool::set_apply_rgb_alignment(bool apply_rgb_alignment) {
    m_apply_rgb_alignment = apply_rgb_alignment;
};

bool PostProcessingTool::get_apply_rgb_alignment() const {
    return m_apply_rgb_alignment;
};

void PostProcessingTool::set_rgb_alignment_parameters(const std::pair<float,float> &shift_red, const std::pair<float,float> &shift_blue) {
    m_shift_red = shift_red;
    m_shift_blue = shift_blue;
};

void PostProcessingTool::set_shift_red(const std::pair<float,float> &shift_red) {
    m_shift_red = shift_red;
};

std::pair<float,float> PostProcessingTool::get_shift_red() const {
    return m_shift_red;
};

void PostProcessingTool::set_shift_blue(const std::pair<float,float> &shift_blue) {
    m_shift_blue = shift_blue;
};

std::pair<float,float> PostProcessingTool::get_shift_blue() const {
    return m_shift_blue;
};

void PostProcessingTool::set_use_auto_rgb_alignment(bool use_auto_rgb_alignment) {
    m_use_auto_rgb_alignment = use_auto_rgb_alignment;
};

bool PostProcessingTool::get_use_auto_rgb_alignment() const {
    return m_use_auto_rgb_alignment;
};