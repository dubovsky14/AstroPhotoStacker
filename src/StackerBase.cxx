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
    double max_value = 0;
    for (int color = 0; color < m_number_of_colors; color++) {
        for (int index = 0; index < m_width*m_height; index++) {
            if (data_for_plotting[color][index] > max_value) {
                max_value = data_for_plotting[color][index];
            }
        }
    }

    const double scale_factor = 65534 / max_value;

    for (int color = 0; color < m_number_of_colors; color++) {
        for (int index = 0; index < m_width*m_height; index++) {
            data_for_plotting[color][index] *= scale_factor;
        }
    }

    crate_color_image(&data_for_plotting.at(0)[0], &data_for_plotting.at(1)[0], &data_for_plotting.at(2)[0] , m_width, m_height, file_address, image_options);
};