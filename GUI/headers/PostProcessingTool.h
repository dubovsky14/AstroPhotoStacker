#pragma once

#include "../../headers/RGBAlignmentTool.h"
#include "../../headers/SharpeningFunctions.h"

#include <vector>

class PostProcessingTool    {
    public:
        PostProcessingTool() = default;

        void set_apply_sharpening(bool apply_sharpening);

        bool get_apply_sharpening() const;

        void set_kernel_size(int kernel_size);

        int get_kernel_size() const;

        void set_gauss_width(float gauss_width);

        float get_gauss_width() const;

        void set_center_value(float center_value);

        float get_center_value() const;

        void set_apply_rgb_alignment(bool apply_rgb_alignment);

        bool get_apply_rgb_alignment() const;

        void set_rgb_alignment_parameters(const std::pair<int,int> &shift_red, const std::pair<int,int> &shift_blue);

        void set_shift_red(const std::pair<int,int> &shift_red);

        std::pair<int,int> get_shift_red() const;

        void set_shift_blue(const std::pair<int,int> &shift_blue);

        std::pair<int,int> get_shift_blue() const;

        template<typename PixelType>
        std::vector<std::vector<PixelType>> post_process_image(const std::vector<std::vector<PixelType>> &image, int width, int height) const {
            std::vector<std::vector<PixelType>> processed_image = image;

            if (m_apply_rgb_alignment) {
                AstroPhotoStacker::RGBAlignmentTool rgb_alignment_tool = AstroPhotoStacker::RGBAlignmentTool(processed_image, width, height);
                rgb_alignment_tool.calculate_shifted_image(m_shift_red, m_shift_blue);
                processed_image = rgb_alignment_tool.get_shifted_image<PixelType>();
            }

            if (m_apply_sharpening) {
                processed_image = AstroPhotoStacker::sharpen_image(processed_image, width, height, m_kernel_size, m_gauss_width, m_center_value);
            }

            return processed_image;
        };

    private:
        bool m_apply_sharpening = false;
        int m_kernel_size = 15;
        float m_gauss_width = 2.1;
        float m_center_value = 0.35;

        bool m_apply_rgb_alignment = false;
        std::pair<int,int> m_shift_red = {0,0};
        std::pair<int,int> m_shift_blue = {0,0};


};
