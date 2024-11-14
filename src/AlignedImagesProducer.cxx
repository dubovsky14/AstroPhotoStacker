#include "../headers/AlignedImagesProducer.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/MetadataReader.h"
#include "../headers/raw_file_reader.h"
#include "../headers/Common.h"
#include "../headers/TimeLapseVideoCreator.h"
#include "../headers/StackerFactory.h"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

AlignedImagesProducer::AlignedImagesProducer(int n_cpu) :
    m_n_cpu(n_cpu)  {
};

void AlignedImagesProducer::limit_output_image_size(int top_left_corner_x, int top_left_corner_y, int width, int height) {
    m_top_left_corner_x = top_left_corner_x;
    m_top_left_corner_y = top_left_corner_y;
    m_width = width;
    m_height = height;
};

void AlignedImagesProducer::add_image(const InputFrame &input_frame, const FileAlignmentInformation &alignment_info) {
    m_frames_to_align.push_back(input_frame);
    m_alignment_info.push_back(alignment_info);
};


void AlignedImagesProducer::add_image_group_to_stack(const std::vector<InputFrame> &input_frames, const std::vector<FileAlignmentInformation> &alignment_info, const StackSettings &stack_settings) {
    GroupToStack group_to_stack;
    group_to_stack.input_frames     = input_frames;
    group_to_stack.alignment_info   = alignment_info;
    group_to_stack.stack_settings   = stack_settings;
    m_groups_to_stack.push_back(group_to_stack);
};


void AlignedImagesProducer::add_calibration_frame_handler(std::shared_ptr<const CalibrationFrameBase> calibration_frame_handler) {
    m_calibration_frame_handlers.push_back(calibration_frame_handler);
};

void AlignedImagesProducer::set_datetime_position(float x_frac, float y_frac)   {
    m_datetime_pos_frac_x = x_frac;
    m_datetime_pos_frac_y = y_frac;
};

void AlignedImagesProducer::produce_aligned_images(const std::string &output_folder_address)  {
    if (m_frames_to_align.size() == 0 && m_groups_to_stack.size() == 0) {
        return;
    }

    const int n_frames = m_frames_to_align.size();
    const int n_cpu = min(m_n_cpu, n_frames);

    cout << "Going to produce aligned images\n";
    cout << "Timestamp offset: " << m_timestamp_offset << endl;

    m_n_tasks_processed = 0;
    m_output_adresses_and_unix_times.clear();
    thread_pool pool(n_cpu);

    // add individual frames
    for (int i_file = 0; i_file < n_frames; i_file++) {
        const string output_file_address = output_folder_address + "/" + get_output_file_name(m_frames_to_align[i_file]);
        const FileAlignmentInformation alignment_info = m_alignment_info[i_file];
        auto submit_alignment = [this, i_file, output_file_address, alignment_info]() {
            produce_aligned_image(m_frames_to_align[i_file], output_file_address, alignment_info);
        };
        pool.submit(submit_alignment);
    }

    // add groups
    for (GroupToStack &group_to_stack : m_groups_to_stack) {
        const string output_file_address = output_folder_address + "/" + get_output_file_name(group_to_stack.input_frames[0]);

        produce_aligned_image(group_to_stack, output_file_address);
    }

    pool.wait_for_tasks();
    produce_video(output_folder_address + "/video.avi");
};

const std::atomic<int>& AlignedImagesProducer::get_tasks_processed() const {
    return m_n_tasks_processed;
};

int AlignedImagesProducer::get_tasks_total() const {
    return m_frames_to_align.size() + m_groups_to_stack.size();
};

string AlignedImagesProducer::get_output_file_name(const InputFrame &input_frame) {
    const std::string input_file_address = input_frame.get_file_address();
    std::string input_file_name = input_file_address;
    const size_t last_slash = input_file_address.find_last_of("/");
    if (last_slash != string::npos) {
        input_file_name = input_file_address.substr(last_slash+1);
    }

    const size_t last_dot = input_file_name.find_last_of(".");
    if (last_dot != string::npos) {
        input_file_name = input_file_name.substr(0, last_dot);
    }

    if (input_frame.is_still_image()) {
        return input_file_name + ".jpg";
    }

    return input_file_name + "_frame_" + to_string(input_frame.get_frame_number()) + ".jpg";
};


void AlignedImagesProducer::produce_aligned_image(const GroupToStack &group_to_stack, const std::string &output_file_address)  {
    if (group_to_stack.input_frames.size() == 0) {
        cout << "No frames to stack\n";
        m_n_tasks_processed++;
        return;
    }

    if (group_to_stack.input_frames.size() != group_to_stack.alignment_info.size()) {
        throw runtime_error("Number of input frames and alignment information do not match");
    }

    int width_original, height_original;
    const InputFrame &first_frame = group_to_stack.input_frames[0];
    get_photo_resolution(first_frame, &width_original, &height_original);
    std::unique_ptr<StackerBase> stacker = create_stacker(group_to_stack.stack_settings, 3, width_original, height_original);

    for (unsigned int i_frame = 0; i_frame < group_to_stack.input_frames.size(); i_frame++) {
        const InputFrame &input_frame                   = group_to_stack.input_frames[i_frame];
        const FileAlignmentInformation &alignment_info  = group_to_stack.alignment_info[i_frame];
        stacker->add_photo(input_frame);
        stacker->add_alignment_info(    input_frame,
                                        alignment_info.shift_x,
                                        alignment_info.shift_y,
                                        alignment_info.rotation_center_x,
                                        alignment_info.rotation_center_y,
                                        alignment_info.rotation,
                                        alignment_info.ranking,
                                        alignment_info.local_shifts_handler);
    }

    stacker->calculate_stacked_photo();
    const std::vector<std::vector<double>> &stacked_image_double = stacker->get_stacked_image();

    int width = width_original - m_top_left_corner_x;
    if (m_width != -1) {
        width = min(width, m_width);
    }

    int height = height_original - m_top_left_corner_y;
    if (m_height != -1) {
        height = min(height, m_height);
    }


    std::vector<vector<unsigned short>> output_image(3, vector<unsigned short>(width*height, 0));
    for (int color = 0; color < 3; color++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const int x_original = x + m_top_left_corner_x;
                const int y_original = y + m_top_left_corner_y;
                output_image[color][x + width*y] = stacked_image_double[color][x_original + width_original*y_original];
            }
        }
    }

    const Metadata metadata = read_metadata(first_frame);
    const int unix_time = metadata.timestamp + m_timestamp_offset;
    const bool use_green_correction = stacker->contains_only_rgb_raw_files();

    process_and_save_image(&output_image, width, height, output_file_address, unix_time, use_green_correction);

    std::scoped_lock lock(m_output_adresses_and_unix_times_mutex);
    m_output_adresses_and_unix_times.push_back({output_file_address, unix_time});

    m_n_tasks_processed++;
};

void AlignedImagesProducer::produce_aligned_image(  const InputFrame &input_frame,  const std::string &output_file_address, const FileAlignmentInformation &alignment_info)  {

    CalibratedPhotoHandler photo_handler(input_frame, true);
    photo_handler.define_alignment(alignment_info.shift_x, alignment_info.shift_y, alignment_info.rotation_center_x, alignment_info.rotation_center_y, alignment_info.rotation);
    photo_handler.define_local_shifts(alignment_info.local_shifts_handler);
    for (const auto &calibration_frame_handler : m_calibration_frame_handlers) {
        photo_handler.register_calibration_frame(calibration_frame_handler);
    }
    if (m_hot_pixel_identifier) {
        photo_handler.register_hot_pixel_identifier(m_hot_pixel_identifier);
    }
    photo_handler.calibrate();

    const int width_original = photo_handler.get_width();
    const int height_original = photo_handler.get_height();

    int width = width_original - m_top_left_corner_x;
    if (m_width != -1) {
        width = min(width, m_width);
    }

    int height = height_original - m_top_left_corner_y;
    if (m_height != -1) {
        height = min(height, m_height);
    }

    std::vector<vector<unsigned short>> output_image(3, vector<unsigned short>(width*height, 0));

    for (int color = 0; color < 3; color++) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                const int x_original = x + m_top_left_corner_x;
                const int y_original = y + m_top_left_corner_y;
                int value = photo_handler.get_value_by_reference_frame_index(x_original + width_original*y_original, color);
                if (value >= 0) {
                    output_image[color][x + width*y] = value;
                }
            }
        }
    }

    const Metadata metadata = read_metadata(input_frame);
    const int unix_time = metadata.timestamp + m_timestamp_offset;
    const bool use_green_correction = is_raw_file(input_frame.get_file_address());

    process_and_save_image(&output_image, width, height, output_file_address, unix_time, use_green_correction);

    std::scoped_lock lock(m_output_adresses_and_unix_times_mutex);
    m_output_adresses_and_unix_times.push_back({output_file_address, unix_time});

    m_n_tasks_processed++;
};

void AlignedImagesProducer::process_and_save_image( std::vector<std::vector<unsigned short>> *stacked_image,
                                                    int width,
                                                    int height,
                                                    const std::string &output_file_address,
                                                    int unix_time,
                                                    bool use_green_correction) const    {

    unsigned short max_value = 0;
    for (const vector<unsigned short> &color_channel : *stacked_image) {
        for (unsigned short pixel_value : color_channel) {
            max_value = max(max_value, pixel_value);
        }
    }

    if (m_image_stretching_function) {
        m_image_stretching_function(stacked_image, max_value);
    }
    if (max_value > 255) {
        scale_down_image(stacked_image, max_value, 255);
    }
    if (use_green_correction) {
        apply_green_correction(stacked_image, 255);
    }

    //// show AP boxes
    //const LocalShiftsHandler &local_shifts_handler = alignment_info.local_shifts_handler;
    //if (!local_shifts_handler.empty()) {
    //    cout << "Going to draw AP boxes\n";
    //    local_shifts_handler.draw_ap_boxes_into_image(stacked_image, width, height, 50, {0, 255, 0}, {255, 0, 0}, m_top_left_corner_x, m_top_left_corner_y);
    //}


    cv::Mat opencv_image = get_opencv_color_image(stacked_image->at(0).data(), stacked_image->at(1).data(), stacked_image->at(2).data(), width, height);

    // rescale image if it exceeds maximal size
    if (m_max_width != -1 && m_max_height != -1) {
        if (width > m_max_width || height > m_max_height) {
            const float scale_factor_width = m_max_width/static_cast<float>(width);
            const float scale_factor_height = m_max_height/static_cast<float>(height);

            const float scale_factor = min(scale_factor_width, scale_factor_height);
            cv::resize(opencv_image, opencv_image, cv::Size(), scale_factor, scale_factor);
        }
    }

    if (m_add_datetime) {
        const string datetime = unix_time_to_string(unix_time);
        const int current_width = opencv_image.cols;
        const int current_height = opencv_image.rows;

        const float font_size = current_width/1200.0;
        const float font_width = font_size*2;

        cv::putText(opencv_image, datetime, cv::Point(m_datetime_pos_frac_x*current_width, m_datetime_pos_frac_y*current_height), cv::FONT_HERSHEY_SIMPLEX, font_size, CV_RGB(255, 0, 0), font_width);
    }

    cv::imwrite(output_file_address, opencv_image);

};


void AlignedImagesProducer::scale_down_image(   std::vector<std::vector<unsigned short>> *image,
                                                unsigned int origianal_max,
                                                unsigned int new_max)  {

    for (int color = 0; color < 3; color++) {
        for (unsigned int i = 0; i < image->at(color).size(); i++) {
            image->at(color)[i] = image->at(color)[i]*new_max/origianal_max;
        }
    }
};

void AlignedImagesProducer::apply_green_correction(std::vector<std::vector<unsigned short>> *image, unsigned short max_value)    {
    // scale down green (we have 2 green channels)
    std::transform(image->at(1).begin(), image->at(1).end(), image->at(1).begin(), [](unsigned short value) { return value; });

    // for some reason, the max of blue and red has to be 32767, not 65534
    std::transform(image->at(0).begin(), image->at(0).end(), image->at(0).begin(), [max_value](unsigned short value) { return std::min<unsigned short>(value*2, max_value); });
    std::transform(image->at(2).begin(), image->at(2).end(), image->at(2).begin(), [max_value](unsigned short value) { return std::min<unsigned short>(value*2, max_value); });
};

void AlignedImagesProducer::produce_video(const std::string &output_video_address) const {
    cout << "Going to produce video with " << m_output_adresses_and_unix_times.size() << " frames\n";

    if (m_output_adresses_and_unix_times.size() == 0) {
        return;
    }

    TimeLapseVideoCreator timelapse_creator(m_timelapse_video_settings);
    for (const auto &[output_file, unix_time] : m_output_adresses_and_unix_times) {
        timelapse_creator.add_image(output_file, unix_time);
    }
    timelapse_creator.create_video(output_video_address);
};

TimeLapseVideoSettings* AlignedImagesProducer::get_timelapse_video_settings() {
    return &m_timelapse_video_settings;
};