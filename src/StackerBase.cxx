#include "../headers/StackerBase.h"
#include "../headers/ImageFilesInputOutput.h"

using namespace std;
using namespace AstroPhotoStacker;

StackerBase::StackerBase(int number_of_colors, int width, int height)   {
    m_number_of_colors = number_of_colors;
    m_width = width;
    m_height = height;
    m_stacked_image = vector<vector<double> >(m_number_of_colors, vector<double>(m_width*m_height, 0));
};


void StackerBase::set_memory_usage_limit(int memory_usage_limit_in_mb)  {
    m_memory_usage_limit_in_mb = memory_usage_limit_in_mb;
};

void StackerBase::add_alignment_text_file(const string &alignment_file_address) {
    m_alignment_file_address = alignment_file_address;
    m_photo_alignment_handler = make_unique<PhotoAlignmentHandler>();
    m_photo_alignment_handler->read_from_text_file(alignment_file_address);
};

void StackerBase::add_photo(const string &file_address) {
    m_files_to_stack.push_back(file_address);
};

void StackerBase::add_flat_frame(const string &file_address) {
    m_flat_frame_handler = make_unique<FlatFrameHandler>(file_address);
};

void StackerBase::save_stacked_photo(const string &file_address, int image_options) const {
    auto data_for_plotting = m_stacked_image;


    for (int color = 0; color < m_number_of_colors; color++) {
        const double max_value = *max_element(data_for_plotting[color].begin(), data_for_plotting[color].end());
        const double scale_factor = 65534 / max_value;
        for (int index = 0; index < m_width*m_height; index++) {
            data_for_plotting[color][index] *= scale_factor;
        }
    }

    const bool is_color_image = m_number_of_colors == 3;

    if (is_color_image) {
        // scale down green (we have 2 green channels)
        std::transform(data_for_plotting[1].begin(), data_for_plotting[1].end(), data_for_plotting[1].begin(), [](double value) { return value / 2; });

        // for some reason, the max of blue and red has to be 32767, not 65534
        std::transform(data_for_plotting[0].begin(), data_for_plotting[0].end(), data_for_plotting[0].begin(), [](double value) { return std::min<double>(value, 32767); });
        std::transform(data_for_plotting[2].begin(), data_for_plotting[2].end(), data_for_plotting[2].begin(), [](double value) { return std::min<double>(value, 32767); });

        crate_color_image(&data_for_plotting.at(0)[0], &data_for_plotting.at(1)[0], &data_for_plotting.at(2)[0] , m_width, m_height, file_address, image_options);
    }
    else {
        create_gray_scale_image(&data_for_plotting.at(0)[0], m_width, m_height, file_address, image_options);
    }

};

void StackerBase::stretch_stacked_photo(StretchingType stretching_type, unsigned int n_bits)  {
    if (m_image_stretcher == nullptr) {
        m_image_stretcher = make_unique<ImageStretcher>(&m_stacked_image);
    }
    m_image_stretcher->set_max_value( pow(2, n_bits) - 1. );
    m_image_stretcher->stretch_image(stretching_type);
};

void StackerBase::apply_black_point(double black_pixels_fraction)  {
    if (m_image_stretcher == nullptr) {
        m_image_stretcher = make_unique<ImageStretcher>(&m_stacked_image);
    }
    m_image_stretcher->apply_black_point(black_pixels_fraction);
};