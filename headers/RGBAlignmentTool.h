#pragma once

#include "../headers/ImageFilesInputOutput.h"

#include <opencv2/opencv.hpp>

#include <vector>
#include <string>

namespace AstroPhotoStacker {

    /**
     * @brief A class used to align RGB images for planetary images - to correct for atmospheric dispersion
     */
    class RGBAlignmentTool  {
        public:
            RGBAlignmentTool() = delete;

            RGBAlignmentTool(const std::string &file_address)   {
                m_data_original = cv::imread(file_address, cv::IMREAD_COLOR);
                m_data_shifted = m_data_original.clone();
            };

            void calculate_shifted_image(const std::pair<int,int> &shift_red, const std::pair<int,int> &shift_blue) {
                cv::Mat trans_mat_red = (cv::Mat_<double>(2,3) << 1, 0, shift_red.first, 0, 1, shift_red.second);
                cv::Mat trans_mat_blue = (cv::Mat_<double>(2,3) << 1, 0, shift_blue.first, 0, 1, shift_blue.second);

                cv::Mat color_channels_original[3];
                split(m_data_original, color_channels_original);

                cv::Mat color_channels_shifted[3];
                split(m_data_shifted, color_channels_shifted);

                cv::warpAffine(color_channels_original[0], color_channels_shifted[0],trans_mat_blue, color_channels_shifted[0].size());
                cv::warpAffine(color_channels_original[2], color_channels_shifted[2],trans_mat_red, color_channels_shifted[2].size());

                cv::merge(color_channels_shifted, 3, m_data_shifted);
            };

            int get_width() const  {
                return m_data_original.cols;
            };

            int get_height() const {
                return m_data_original.rows;
            };

            void save_shifted_image(const std::string &file_address) const  {
                cv::imwrite(file_address, m_data_shifted);
            };

        private:
            cv::Mat m_data_original;
            cv::Mat m_data_shifted;
    };
}