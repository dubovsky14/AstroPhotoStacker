#include "../headers/StackerBase.h"
#include "../headers/ImageFilesInputOutput.h"

using namespace std;
using namespace AstroPhotoStacker;

StackerBase::StackerBase(int number_of_colors, int width, int height, bool interpolate_colors)   {
    m_number_of_colors = number_of_colors;
    m_width = width;
    m_height = height;
    m_stacked_image = vector<vector<double> >(m_number_of_colors, vector<double>(m_width*m_height, 0));
    m_interpolate_colors = interpolate_colors;
};


void StackerBase::set_memory_usage_limit(int memory_usage_limit_in_mb)  {
    m_memory_usage_limit_in_mb = memory_usage_limit_in_mb;
};

void StackerBase::add_alignment_text_file(const string &alignment_file_address) {
    m_alignment_file_address = alignment_file_address;
    m_photo_alignment_handler = make_unique<PhotoAlignmentHandler>();
    m_photo_alignment_handler->read_from_text_file(alignment_file_address);
};

void StackerBase::add_alignment_info(const std::string &file_address, float x_shift, float y_shift, float rotation_center_x, float rotation_center_y, float rotation, float ranking) {
    if (m_photo_alignment_handler == nullptr) {
        m_photo_alignment_handler = make_unique<PhotoAlignmentHandler>();
    }
    m_photo_alignment_handler->add_alignment_info(file_address, x_shift, y_shift, rotation_center_x, rotation_center_y, rotation, ranking);
};

void StackerBase::add_photo(const string &file_address, bool apply_alignment) {
    m_files_to_stack.push_back(file_address);
    m_apply_alignment.push_back(apply_alignment);
};

void StackerBase::add_flat_frame(const string &file_address) {
    m_flat_frame_handler = make_unique<FlatFrameHandler>(file_address);
};

void StackerBase::register_hot_pixels_file(const std::string &hot_pixels_file)  {
    m_hot_pixel_identifier = make_unique<HotPixelIdentifier>();
    m_hot_pixel_identifier->load_hot_pixels_from_file(hot_pixels_file);
};

void StackerBase::save_stacked_photo(const string &file_address, int image_options) const {
    auto data_for_plotting = m_stacked_image;

    const unsigned int max_value_output = pow(2, get_output_bit_depth(image_options)) -1;
    for (int color = 0; color < m_number_of_colors; color++) {
        const double max_value_input = *max_element(data_for_plotting[color].begin(), data_for_plotting[color].end());
        const double scale_factor = max_value_output / max_value_input;
        for (int index = 0; index < m_width*m_height; index++) {
            data_for_plotting[color][index] *= scale_factor;
        }
    }
    const bool color_image_source = m_number_of_colors == 3;
    const bool color_image_target = ((image_options >> 3) == (3-1));

    if (color_image_source) {
        // scale down green (we have 2 green channels)
        std::transform(data_for_plotting[1].begin(), data_for_plotting[1].end(), data_for_plotting[1].begin(), [](double value) { return value / 2; });

        // for some reason, the max of blue and red has to be 32767, not 65534
        std::transform(data_for_plotting[0].begin(), data_for_plotting[0].end(), data_for_plotting[0].begin(), [max_value_output](double value) { return std::min<double>(value, max_value_output/2 + 1); });
        std::transform(data_for_plotting[2].begin(), data_for_plotting[2].end(), data_for_plotting[2].begin(), [max_value_output](double value) { return std::min<double>(value, max_value_output/2 + 1); });

        if (color_image_target) {
            crate_color_image(&data_for_plotting.at(0)[0], &data_for_plotting.at(1)[0], &data_for_plotting.at(2)[0] , m_width, m_height, file_address, image_options);
        }
        else {
            std::vector<double> data_for_plotting_merged(m_width*m_height, 0.);
            for (int index = 0; index < m_width*m_height; index++) {
                data_for_plotting_merged[index] = (data_for_plotting[0][index] + data_for_plotting[1][index] + data_for_plotting[2][index]) / 3;
            }
            create_gray_scale_image(&data_for_plotting_merged.at(0), m_width, m_height, file_address, image_options);
        }
    }
    else {
        create_gray_scale_image(&data_for_plotting.at(0)[0], m_width, m_height, file_address, image_options);
    }

};

void StackerBase::fix_empty_pixels()    {
    auto get_average_from_pixels_around = [](const vector<double> &color_channel, int width, int pixel_index) {
        double sum = 0;
        int n_pixels = 0;
        for (int i_shift_y = -1; i_shift_y <= 1; i_shift_y++) {
            for (int i_shift_x = -1; i_shift_x <= 1; i_shift_x++) {
                const int index = pixel_index + i_shift_x + i_shift_y*width;
                if (index >= 0 && index < static_cast<int>(color_channel.size()) && color_channel[index] != c_empty_pixel_value) {
                    sum += color_channel[index];
                    n_pixels++;
                }
            }
        }
        return n_pixels != 0 ? sum/n_pixels : 0.;
    };

    for (vector<double> &color : m_stacked_image) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            if (color[i_pixel] == c_empty_pixel_value) {
                color[i_pixel] = get_average_from_pixels_around(color, m_width, i_pixel);
            }
        }
    }
};

int StackerBase::get_output_bit_depth(int open_cv_image_type)    {
    const int bit_depth_mask = 0b111;
    const int bit_depth_code = open_cv_image_type & bit_depth_mask;
    if (bit_depth_code == CV_8U) {
        return 8;
    }
    else if (bit_depth_code == CV_16U) {
        return 16;
    }
    else if (bit_depth_code == CV_8S) {
        return 7;
    }
    else if (bit_depth_code == CV_16S) {
        return 15;
    }
    else if (bit_depth_code == CV_32S) {
        return 32;
    }
    else if (bit_depth_code == CV_32F) {
        return 32;
    }
    else if (bit_depth_code == CV_64F) {
        return 64;
    }
    else if (bit_depth_code == CV_16F) {
        return 16;
    }
    else    {
        throw runtime_error("Unsupported image type");
    }
};

void StackerBase::set_hot_pixels(const std::vector<std::tuple<int, int> > &hot_pixels)  {
    if (m_hot_pixel_identifier == nullptr) {
        m_hot_pixel_identifier = make_unique<HotPixelIdentifier>();
    }
    m_hot_pixel_identifier->set_hot_pixels(hot_pixels);
};

const std::atomic<int>& StackerBase::get_tasks_processed() const    {
    return m_n_tasks_processed;
};

const std::vector<std::vector<double> >& StackerBase::get_stacked_image() const {
    return m_stacked_image;
};