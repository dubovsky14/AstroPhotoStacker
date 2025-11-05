#include "../headers/StackerBase.h"
#include "../headers/ImageFilesInputOutput.h"

#include "../headers/AlignmentResultDummy.h"

using namespace std;
using namespace AstroPhotoStacker;

StackerBase::StackerBase(int number_of_colors, int width, int height, bool interpolate_colors)   {
    m_number_of_colors = number_of_colors;
    m_width = width;
    m_height = height;
    m_interpolate_colors = interpolate_colors;
};

void StackerBase::set_memory_usage_limit(int memory_usage_limit_in_mb)  {
    m_memory_usage_limit_in_mb = memory_usage_limit_in_mb;
};

void StackerBase::add_alignment_text_file(const string &alignment_file_address) {
    m_photo_alignment_handler = make_unique<PhotoAlignmentHandler>();
    m_photo_alignment_handler->read_from_text_file(alignment_file_address);
};

void StackerBase::add_alignment_info(const InputFrame &input_frame, const AlignmentResultBase &alignment_result) {
    if (m_photo_alignment_handler == nullptr) {
        m_photo_alignment_handler = make_unique<PhotoAlignmentHandler>();
    }
    m_photo_alignment_handler->add_alignment_info(input_frame, alignment_result);
};

void StackerBase::add_photo(const InputFrame &input_frame,
                            const std::vector<std::shared_ptr<const CalibrationFrameBase> > &calibration_frame_handlers,
                            bool apply_alignment) {
    m_frames_to_stack.push_back(input_frame);
    m_calibration_frame_handlers.push_back(calibration_frame_handlers);
    m_apply_alignment.push_back(apply_alignment);
};

void StackerBase::register_hot_pixels_file(const std::string &hot_pixels_file)  {
    m_hot_pixel_identifier = make_unique<HotPixelIdentifier>();
    m_hot_pixel_identifier->load_hot_pixels_from_file(hot_pixels_file);
};

void StackerBase::save_stacked_photo(const string &file_address, int image_options) const {
    save_stacked_photo(file_address, m_stacked_image, m_width, m_height, image_options);
};

void StackerBase::save_stacked_photo(const std::string &file_address, const std::vector<std::vector<double> > &stacked_image, int width, int height, int image_options)   {
    std::vector<std::vector<double> > data_for_plotting = stacked_image;

    const unsigned int max_value_output = pow(2, get_output_bit_depth(image_options)) -1;
    double max_value_input = 0;
    for (unsigned int color = 0; color < stacked_image.size(); color++) {
        const double max_value_color = *max_element(data_for_plotting[color].begin(), data_for_plotting[color].end());
        if (max_value_color > max_value_input) {
            max_value_input = max_value_color;
        }
    }

    const double scale_factor = 0.5 * max_value_output / max_value_input;
    for (unsigned int color = 0; color < stacked_image.size(); color++) {
        for (int index = 0; index < width*height; index++) {
            data_for_plotting[color][index] *= scale_factor;
        }
    }
    const bool color_image_source = stacked_image.size() == 3;
    const bool color_image_target = ((image_options >> 3) == (3-1));

    if (color_image_source) {
        if (color_image_target) {
            create_color_image(&data_for_plotting.at(0)[0], &data_for_plotting.at(1)[0], &data_for_plotting.at(2)[0] , width, height, file_address, image_options);
        }
        else {
            std::vector<double> data_for_plotting_merged(width*height, 0.);
            for (int index = 0; index < width*height; index++) {
                data_for_plotting_merged[index] = (data_for_plotting[0][index] + data_for_plotting[1][index] + data_for_plotting[2][index]) / 3;
            }
            create_gray_scale_image(&data_for_plotting_merged.at(0), width, height, file_address, image_options);
        }
    }
    else {
        create_gray_scale_image(&data_for_plotting.at(0)[0], width, height, file_address, image_options);
    }
};

void StackerBase::set_number_of_cpu_threads(unsigned int n_cpu) {
    m_n_cpu = n_cpu;
};

void StackerBase::calculate_stacked_photo()  {
    m_stacked_image = vector<vector<double> >(m_number_of_colors, vector<double>(m_width*m_height, c_empty_pixel_value));
    calculate_stacked_photo_internal();
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

CalibratedPhotoHandler StackerBase::get_calibrated_photo(unsigned int i_file, int y_min, int y_max) const    {
    const InputFrame &input_frame = m_frames_to_stack[i_file];
    const bool apply_alignment = m_apply_alignment[i_file];
    unique_ptr<AlignmentResultBase> alignment_result = apply_alignment ? m_photo_alignment_handler->get_alignment_parameters(input_frame) : nullptr;

    CalibratedPhotoHandler calibrated_photo(input_frame, m_interpolate_colors);
    if (alignment_result != nullptr) {
        calibrated_photo.define_alignment(*alignment_result);
    }
    else {
        calibrated_photo.define_alignment(AlignmentResultDummy());
    }

    calibrated_photo.limit_y_range(y_min, y_max);
    if (m_hot_pixel_identifier != nullptr)  {
        calibrated_photo.register_hot_pixel_identifier(m_hot_pixel_identifier.get());
    }
    for (const std::shared_ptr<const CalibrationFrameBase> &calibration_frame : m_calibration_frame_handlers.at(i_file)) {
        calibrated_photo.register_calibration_frame(calibration_frame);
    }
    calibrated_photo.calibrate();
    return calibrated_photo;
};

std::vector<std::string> StackerBase::get_additional_setting_keys() const {
    std::vector<std::string> keys;
    for (const auto &pair : m_additional_settings) {
        keys.push_back(pair.first);
    }
    return keys;
};

void StackerBase::set_additional_setting(const std::string &name, double value) {
    if (m_additional_settings.find(name) == m_additional_settings.end()) {
        throw std::runtime_error("Setting not found: " + name);
    }
    m_additional_settings.at(name).set_value(value);
};

AdditionalStackerSetting StackerBase::get_additional_setting(const std::string &name) const {
    if (m_additional_settings.find(name) != m_additional_settings.end()) {
        return m_additional_settings.at(name);
    }
    throw std::runtime_error("Setting not found: " + name);
};

void StackerBase::configure_stacker(std::map<std::string, double> settings)  {
    for (const auto &pair : settings) {
        set_additional_setting(pair.first, pair.second);
    }
};