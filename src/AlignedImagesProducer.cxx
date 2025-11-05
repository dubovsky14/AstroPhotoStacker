#include "../headers/AlignedImagesProducer.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/InputFrameReader.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/MetadataReader.h"
#include "../headers/Common.h"
#include "../headers/TimeLapseVideoCreator.h"
#include "../headers/StackerFactory.h"
#include "../headers/TaskScheduler.hxx"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

AlignedImagesProducer::AlignedImagesProducer(int n_cpu, int memory_usage_limit_in_mb) :
    m_n_cpu(n_cpu),
    m_memory_usage_limit_in_mb(memory_usage_limit_in_mb)  {
};

void AlignedImagesProducer::limit_output_image_size(int top_left_corner_x, int top_left_corner_y, int width, int height) {
    m_top_left_corner_x = top_left_corner_x;
    m_top_left_corner_y = top_left_corner_y;
    m_width = width;
    m_height = height;
};

void AlignedImagesProducer::add_image(  const InputFrame &input_frame,
                                        const AlignmentResultBase &alignment_result,
                                        const std::vector<std::shared_ptr<const CalibrationFrameBase> > &calibration_frame_handlers) {
    m_frames_to_align.push_back(input_frame);
    m_alignment_info.push_back(alignment_result.clone());
    m_calibration_frame_handlers.push_back(calibration_frame_handlers);
};


void AlignedImagesProducer::add_image_group_to_stack(   const std::vector<InputFrame> &input_frames,
                                                        const std::vector<std::unique_ptr<AlignmentResultBase>> &alignment_vector,
                                                        const StackSettings &stack_settings,
                                                        const std::vector<std::vector<std::shared_ptr<const CalibrationFrameBase> > > *calibration_frame_handlers) {
    GroupToStack group_to_stack;
    group_to_stack.input_frames     = input_frames;
    group_to_stack.stack_settings   = stack_settings;
    for (const auto &alignment_info : alignment_vector) {
        group_to_stack.alignment_info.push_back(alignment_info->clone());
    }

    if (calibration_frame_handlers != nullptr) {
        group_to_stack.calibration_frame_handlers = *calibration_frame_handlers;
    }
    else {
        group_to_stack.calibration_frame_handlers.resize(input_frames.size());
    }

    m_groups_to_stack.push_back(std::move(group_to_stack));
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

    m_n_tasks_processed = 0;
    m_output_addresses_and_unix_times.clear();
    thread_pool pool(n_cpu);

    // add individual frames
    for (int i_file = 0; i_file < n_frames; i_file++) {
        const string output_file_address = output_folder_address + "/" + get_output_file_name(m_frames_to_align[i_file]);
        const AlignmentResultBase *alignment_result = m_alignment_info[i_file].get();
        const std::vector<std::shared_ptr<const CalibrationFrameBase> > &calibration_frame_handlers = m_calibration_frame_handlers[i_file];
        auto submit_alignment = [this, i_file, output_file_address, alignment_result, &calibration_frame_handlers]() {
            produce_aligned_image(m_frames_to_align[i_file], output_file_address, *alignment_result, calibration_frame_handlers);
        };
        pool.submit(submit_alignment);
    }
    pool.wait_for_tasks();

    TaskScheduler task_scheduler({size_t(m_n_cpu), 1024ULL*1024ULL*m_memory_usage_limit_in_mb});
    // add groups
    for (GroupToStack &group_to_stack : m_groups_to_stack) {
        const string output_file_address = output_folder_address + "/" + get_output_file_name(group_to_stack.input_frames[0]);
        const size_t cpus_needed = min<int>(group_to_stack.stack_settings.get_n_cpus(), group_to_stack.input_frames.size());
        const size_t memory_usage_limit =  min<int>(group_to_stack.stack_settings.get_max_memory(), 1024ULL*1024ULL*m_memory_usage_limit_in_mb);
        group_to_stack.stack_settings.set_n_cpus(cpus_needed); // during the final step of median-based algorithms, all CPUs would be used even if there are less frames

        task_scheduler.submit([this, &group_to_stack, output_file_address]() {
            produce_aligned_image(group_to_stack, output_file_address);
        }, {cpus_needed, memory_usage_limit});
    }
    task_scheduler.wait_for_tasks();

    produce_video(output_folder_address + "/video.avi");
};

const std::atomic<int>& AlignedImagesProducer::get_tasks_processed() const {
    return m_n_tasks_processed;
};

int AlignedImagesProducer::get_tasks_total() const {
    return m_frames_to_align.size() + m_groups_to_stack.size();
};

string AlignedImagesProducer::get_output_file_name(const InputFrame &input_frame) {
    const std::string &input_file_address = input_frame.get_file_address();
    std::string input_file_name = input_file_address;
    const size_t last_slash = input_file_address.find_last_of('/');
    if (last_slash != string::npos) {
        input_file_name = input_file_address.substr(last_slash+1);
    }

    const size_t last_dot = input_file_name.find_last_of('.');
    if (last_dot != string::npos) {
        input_file_name = input_file_name.substr(0, last_dot);
    }

    if (input_frame.is_still_image()) {
        return input_file_name + ".jpg";
    }

    return input_file_name + "_frame_" + to_string(input_frame.get_frame_number()) + ".jpg";
};


std::pair<int,int> AlignedImagesProducer::calculate_cropped_width_and_height(int width_original, int height_original) const {
    int width = width_original - m_top_left_corner_x;
    if (m_width != -1) {
        width = std::min(width, m_width);
    }

    int height = height_original - m_top_left_corner_y;
    if (m_height != -1) {
        height = std::min(height, m_height);
    }
    return {width, height};
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

    InputFrameReader reader(first_frame);
    reader.get_photo_resolution(&width_original, &height_original);
    std::unique_ptr<StackerBase> stacker = create_stacker(group_to_stack.stack_settings, 3, width_original, height_original);

    for (unsigned int i_frame = 0; i_frame < group_to_stack.input_frames.size(); i_frame++) {
        const InputFrame &input_frame                   = group_to_stack.input_frames[i_frame];
        const std::unique_ptr<AlignmentResultBase> &alignment_result       = group_to_stack.alignment_info[i_frame];
        const std::vector<std::shared_ptr<const CalibrationFrameBase> > &calibration_frame_handlers = group_to_stack.calibration_frame_handlers[i_frame];
        stacker->add_photo(input_frame, calibration_frame_handlers);
        stacker->add_alignment_info(    input_frame,
                                        *alignment_result);
    }

    stacker->calculate_stacked_photo();
    const std::vector<std::vector<double>> &stacked_image_double = stacker->get_stacked_image();

    const auto [width_crop, height_crop] = calculate_cropped_width_and_height(width_original, height_original);
    std::vector<vector<PixelType>> output_image(3, vector<PixelType>(width_crop*height_crop, 0));
    for (int color = 0; color < 3; color++) {
        for (int y = 0; y < height_crop; y++) {
            for (int x = 0; x < width_crop; x++) {
                const int x_original = x + m_top_left_corner_x;
                const int y_original = y + m_top_left_corner_y;
                output_image[color][x + width_crop*y] = stacked_image_double[color][x_original + width_original*y_original];
            }
        }
    }

    process_save_and_update_counter_and_image_list(&output_image, width_crop, height_crop, output_file_address, first_frame);
};

void AlignedImagesProducer::produce_aligned_image(  const InputFrame &input_frame,
                                                    const std::string &output_file_address,
                                                    const AlignmentResultBase &alignment_result,
                                                    const std::vector<std::shared_ptr<const CalibrationFrameBase> > &calibration_frame_handlers)  {

    CalibratedPhotoHandler photo_handler(input_frame, true);
    photo_handler.define_alignment(alignment_result);

    for (const auto &calibration_frame_handler : calibration_frame_handlers) {
        photo_handler.register_calibration_frame(calibration_frame_handler);
    }
    if (m_hot_pixel_identifier) {
        photo_handler.register_hot_pixel_identifier(m_hot_pixel_identifier);
    }
    photo_handler.calibrate();

    const int width_original = photo_handler.get_width();
    const int height_original = photo_handler.get_height();
    const auto [width, height] = calculate_cropped_width_and_height(width_original, height_original);

    std::vector<vector<PixelType>> output_image(3, vector<PixelType>(width*height, 0));

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

    process_save_and_update_counter_and_image_list(&output_image, width, height, output_file_address, input_frame);
};

void AlignedImagesProducer::process_save_and_update_counter_and_image_list(
                                std::vector<std::vector<PixelType>> *stacked_image,
                                int width,
                                int height,
                                const std::string &output_file_address,
                                const InputFrame &input_frame)     {

    const Metadata metadata = read_metadata(input_frame);
    const int unix_time = metadata.timestamp + m_timestamp_offset;

    process_and_save_image(stacked_image, width, height, output_file_address, unix_time);

    std::scoped_lock lock(m_output_addresses_and_unix_times_mutex);
    m_output_addresses_and_unix_times.emplace_back(output_file_address, unix_time);

    m_n_tasks_processed++;
};

void AlignedImagesProducer::process_and_save_image( std::vector<std::vector<PixelType>> *stacked_image,
                                                    int width,
                                                    int height,
                                                    const std::string &output_file_address,
                                                    int unix_time) const    {

    PixelType max_value = get_max_value_ignoring_borders(*stacked_image, width, height, 5);

    if (m_image_stretching_function) {
        m_image_stretching_function(stacked_image, max_value);
    }


    if (max_value > 255) {
        scale_down_image(stacked_image, max_value, 255);
    }

    if (m_post_processing_tool) {
        *stacked_image = m_post_processing_tool(*stacked_image, width, height);
    }

    for (vector<PixelType> &color_channel : *stacked_image) {
        for (PixelType &value : color_channel) {
            value = min<PixelType>(value, 255);
        }
    }

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

void AlignedImagesProducer::scale_down_image(   std::vector<std::vector<PixelType>> *image,
                                                unsigned int original_max,
                                                unsigned int new_max)  {

    for (int color = 0; color < 3; color++) {
        for (unsigned int i = 0; i < image->at(color).size(); i++) {
            image->at(color)[i] = image->at(color)[i]*float(new_max)/original_max;
        }
    }
};

void AlignedImagesProducer::produce_video(const std::string &output_video_address) const {
    if (m_output_addresses_and_unix_times.size() == 0) {
        return;
    }

    TimeLapseVideoCreator timelapse_creator(m_timelapse_video_settings);
    for (const auto &[output_file, unix_time] : m_output_addresses_and_unix_times) {
        timelapse_creator.add_image(output_file, unix_time);
    }
    timelapse_creator.create_video(output_video_address);
};

TimeLapseVideoSettings* AlignedImagesProducer::get_timelapse_video_settings() {
    return &m_timelapse_video_settings;
};