#include "../headers/TimeLapseVideoCreator.h"

#include <opencv2/opencv.hpp>

#include <algorithm>

using namespace AstroPhotoStacker;
using namespace std;


void TimeLapseVideoCreator::add_image(const std::string &file_address, int unix_time)    {
    m_input_files.push_back({file_address, unix_time});
};

void TimeLapseVideoCreator::set_fps(int fps)    {
    m_fps = fps;
};

int TimeLapseVideoCreator::get_fps()   const    {
    return m_fps;
};

void TimeLapseVideoCreator::set_n_repeat(int n_repeat)  {
    m_n_repeat = n_repeat;
};

int TimeLapseVideoCreator::get_n_repeat()  const    {
    return m_n_repeat;
};

void TimeLapseVideoCreator::create_video(const std::string &video_address, bool sort_by_time) const  {
    vector<tuple<string,int>> input_files = m_input_files;
    if (sort_by_time)   {
        sort(input_files.begin(), input_files.end(), [](const tuple<string,int> &a, const tuple<string,int> &b)   {
            return get<1>(a) < get<1>(b);
        });
    }

    if (input_files.size() == 0)    {
        return;
    }

    cv::Mat first_image = cv::imread(get<0>(input_files.at(0)));
    if (first_image.empty()) {
        cout << "First image is empty: " << get<0>(input_files.at(0)) << endl;
        return;
    }
    const cv::Size frame_size = first_image.size();

    //cv::VideoWriter video_writer(video_address, 0, 1, frame_size);
    cv::VideoWriter video_writer(video_address, cv::CAP_ANY,  cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), m_fps, frame_size);
    if (!video_writer.isOpened())   {
        cout << "Could not open video writer" << endl;
        cout << "Video address: " << video_address << endl;
        cout << "Frame size: " << frame_size << endl;
        cout << "FPS: " << m_fps << endl;
        return;
    }
    for (int i_repeat = 0; i_repeat < m_n_repeat; i_repeat++)   {
        for (const auto &[file, unixtime] : input_files) {
            cv::Mat image = cv::imread(file);
            if (image.empty())  {
                cout << "Image is empty: " << file << endl;
                continue;
            }
            video_writer.write(image);
        }
    }

    video_writer.release();
};
