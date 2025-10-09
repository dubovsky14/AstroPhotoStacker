#include "../headers/NonRawFrameReaderVideo.h"
#include "../headers/MetadataCommon.h"

#include <exiv2/exiv2.hpp>
#include <opencv2/opencv.hpp>

using namespace AstroPhotoStacker;
using namespace std;

NonRawFrameReaderVideo::NonRawFrameReaderVideo(const InputFrame &input_frame) : NonRawFrameReaderBase(input_frame) {
};

vector<vector<short int>> NonRawFrameReaderVideo::get_pixels_data(int *width, int *height) {
    const string video_address = m_input_frame.get_file_address();
    const int frame_id = m_input_frame.get_frame_number();
    cv::VideoCapture video(video_address);
    if (!video.isOpened()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }
    video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
    cv::Mat frame;
    video.read(frame);
    m_width = frame.cols;
    m_height = frame.rows;
    const int n_colors = frame.channels();

    std::vector<std::vector<short int>> result(n_colors, std::vector<short int>(m_width*m_height));
    if (n_colors == 3)  {
        int bit_depth = frame.depth();
        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                for (int color = 0; color < n_colors; color++) {
                    if (bit_depth == CV_8U) {
                        result[2-color][y*(*width) + x] = frame.at<cv::Vec3b>(y, x)[color];
                    }
                    else if (bit_depth == CV_16U) {
                        result[2-color][y*(*width) + x] = frame.at<cv::Vec3w>(y, x)[color]/2;
                    }
                    else if (bit_depth == CV_16S) {
                        result[2-color][y*(*width) + x] = frame.at<cv::Vec3s>(y, x)[color];
                    }
                    else {
                        throw std::runtime_error("Unsupported bit depth");
                    }
                }
            }
        }
    }
    else if (n_colors == 1) {
        int bit_depth = frame.depth();
        for (int y = 0; y < *height; y++) {
            for (int x = 0; x < *width; x++) {
                if (bit_depth == CV_8U) {
                    result[0][y*(*width) + x] = frame.at<unsigned char>(y, x);
                }
                else if (bit_depth == CV_16U) {
                    result[0][y*(*width) + x] = frame.at<unsigned short>(y, x)/2;
                }
                else if (bit_depth == CV_16S) {
                    result[0][y*(*width) + x] = frame.at<short>(y, x);
                }
                else {
                    throw std::runtime_error("Unsupported bit depth");
                }
            }
        }
    }
    else {
        throw std::runtime_error("Unsupported number of channels: " + std::to_string(n_colors));
    }

    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }

    return result;
};

void NonRawFrameReaderVideo::get_photo_resolution(int *width, int *height) {
    if (m_width < 0 || m_height < 0) {
        const string video_address = m_input_frame.get_file_address();
        const int frame_id = m_input_frame.get_frame_number();
        cv::VideoCapture video(video_address);
        if (!video.isOpened()) {
            throw std::runtime_error("Unable to open video file: " + video_address);
        }
        video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
        cv::Mat frame;
        video.read(frame);
        m_width = frame.cols;
        m_height = frame.rows;
    }
    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }
};

Metadata NonRawFrameReaderVideo::read_metadata_without_cache() {
    Metadata metadata;
    const string video_address = m_input_frame.get_file_address();
    const int frame_id = m_input_frame.get_frame_number();
    cv::VideoCapture video(video_address);
    if (!video.isOpened()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }
    video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
    cv::Mat frame;
    video.read(frame);
    m_width = frame.cols;
    m_height = frame.rows;
    const int n_colors = frame.channels();
    metadata.video_fps = video.get(cv::CAP_PROP_FPS);
    metadata.monochrome = (n_colors == 1);

    try {
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(video_address);
        if (image.get() == 0) {
            throw std::runtime_error("Failed to open the file " + video_address);
        }
        image->readMetadata();

        Exiv2::ExifData &exifData = image->exifData();
        if (exifData.empty()) {
            std::string error(video_address);
            error += ": No EXIF data found in the file";
            throw Exiv2::Error(Exiv2::kerErrorMessage, error);
        }

        // Date and Time
        const auto dateTime = exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
        if (dateTime != exifData.end()) {
            metadata.date_time = dateTime->toString();
            metadata.timestamp = get_unix_timestamp(metadata.date_time);
        } else {
            std::cerr << "DateTimeOriginal not found in the metadata." << std::endl;
        }

    } catch (Exiv2::AnyError& e) {
        std::cerr << "Error reading file " << video_address << ": " << e.what() << std::endl;

        // get at least the timestamp
        metadata = get_file_creation_timestamp(video_address, metadata);
    }
    return metadata;
};