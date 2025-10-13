#include "../headers/NonRawFrameReaderVideo.h"
#include "../headers/MetadataCommon.h"

#include <exiv2/exiv2.hpp>
#include <opencv2/opencv.hpp>

using namespace AstroPhotoStacker;
using namespace std;

NonRawFrameReaderVideo::NonRawFrameReaderVideo(const InputFrame &input_frame) : NonRawFrameReaderBase(input_frame) {
};

vector<vector<PixelType>> NonRawFrameReaderVideo::get_pixels_data(int *width, int *height) {
    const string video_address = m_input_frame.get_file_address();
    const int frame_id = m_input_frame.get_frame_number();
    cv::VideoCapture video(video_address);
    if (!video.isOpened()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }
    video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
    cv::Mat frame;
    video.read(frame);
    vector<vector<PixelType>> result = opencv_rgb_image_to_vector_vector_short(frame, &m_width, &m_height);

    if (width != nullptr) {
        *width = m_width;
    }
    if (height != nullptr) {
        *height = m_height;
    }

    return result;
};



std::vector<PixelType> NonRawFrameReaderVideo::get_pixels_data_monochrome(int *width, int *height) {
    const string video_address = m_input_frame.get_file_address();
    const int frame_id = m_input_frame.get_frame_number();
    cv::VideoCapture video(video_address);
    if (!video.isOpened()) {
        throw std::runtime_error("Unable to open video file: " + video_address);
    }
    video.set(cv::CAP_PROP_POS_FRAMES, frame_id);
    cv::Mat frame;
    video.read(frame);

    // convert to grayscale
    cv::Mat image;
    cv::cvtColor(frame, image, cv::COLOR_BGR2GRAY);

    vector<PixelType> result = opencv_grayscale_image_to_vector_short(image, &m_width, &m_height);

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