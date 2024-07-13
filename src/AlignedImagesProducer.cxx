#include "../headers/AlignedImagesProducer.h"
#include "../headers/CalibratedPhotoHandler.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/MetadataReader.h"

#include "../headers/thread_pool.h"

#include <opencv2/opencv.hpp>

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

void AlignedImagesProducer::add_image(const std::string &file_address, const FileAlignmentInformation &alignment_info) {
    m_files_to_align.push_back(file_address);
    m_alignment_info.push_back(alignment_info);
};

void AlignedImagesProducer::add_calibration_frame_handler(std::shared_ptr<const CalibrationFrameBase> calibration_frame_handler) {
    m_calibration_frame_handlers.push_back(calibration_frame_handler);
};

void AlignedImagesProducer::set_datetime_position(float x_frac, float y_frac)   {
    m_datetime_pos_frac_x = x_frac;
    m_datetime_pos_frac_y = y_frac;
};

void AlignedImagesProducer::produce_aligned_images(const std::string &output_folder_address) const {
    if (m_files_to_align.size() == 0) {
        return;
    }

    const int n_files = m_files_to_align.size();
    const int n_cpu = min(m_n_cpu, n_files);

    thread_pool pool(n_cpu);
    for (int i_file = 0; i_file < n_files; i_file++) {
        const string output_file_address = output_folder_address + "/" + get_file_name(m_files_to_align[i_file]);
        const FileAlignmentInformation alignment_info = m_alignment_info[i_file];
        auto submit_alignment = [this, i_file, output_file_address, alignment_info]() {
            cout << "Aligning " << m_files_to_align[i_file] << endl;
            produce_aligned_image(m_files_to_align[i_file], output_file_address, alignment_info);
        };
        pool.submit(submit_alignment);
    }
    pool.wait_for_tasks();
};

string AlignedImagesProducer::get_file_name(const std::string &file_address) {
    const size_t last_slash = file_address.find_last_of("/");
    if (last_slash == string::npos) {
        return file_address;
    }
    return file_address.substr(last_slash+1);
};

void AlignedImagesProducer::produce_aligned_image( const std::string &input_file_address,
                            const std::string &output_file_address,
                            const FileAlignmentInformation &alignment_info) const {

    CalibratedPhotoHandler photo_handler(input_file_address, true);
    photo_handler.define_alignment(alignment_info.shift_x, alignment_info.shift_y, alignment_info.rotation_center_x, alignment_info.rotation_center_y, alignment_info.rotation);
    for (const auto &calibration_frame_handler : m_calibration_frame_handlers) {
        photo_handler.register_calibration_frame(calibration_frame_handler);
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
                int value = photo_handler.get_value_by_reference_frame_index(x_original + width*y_original, color);
                if (value >= 0) {
                    output_image[color][x + width*y] = value;
                }
            }
        }
    }

    if (!m_add_datetime) {
        crate_color_image(&output_image[0][0], &output_image[1][0], &output_image[2][0], width, height, output_file_address);
    }
    else {
        const Metadata metadata = read_metadata(input_file_address);
        const string datetime = metadata.date_time;

        cv::Mat opencv_image = get_opencv_color_image(&output_image[0][0], &output_image[1][0], &output_image[2][0], width, height);

        cv::putText(opencv_image, datetime, cv::Point(m_datetime_pos_frac_x*width, m_datetime_pos_frac_y*height), cv::FONT_HERSHEY_SIMPLEX, 3, CV_RGB(255, 0, 0), 2);

        cv::imwrite(output_file_address, opencv_image);
    }
};