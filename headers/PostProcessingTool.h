#pragma once

#include "../headers/RGBAlignmentTool.h"
#include "../headers/SharpeningFunctions.h"
#include "../headers/LightPollutionGradientFunctions.h"
#include "../headers/LightPollutionRemovalTool.h"

#include <vector>

namespace AstroPhotoStacker {
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

            void set_rgb_alignment_parameters(const std::pair<float,float> &shift_red, const std::pair<float,float> &shift_blue);

            void set_shift_red(const std::pair<float,float> &shift_red);

            std::pair<float,float> get_shift_red() const;

            void set_shift_blue(const std::pair<float,float> &shift_blue);

            std::pair<float,float> get_shift_blue() const;

            void set_use_auto_rgb_alignment(bool use_auto_rgb_alignment);

            bool get_use_auto_rgb_alignment() const;

            void set_use_light_pollution_removal(bool use_light_pollution_removal);

            bool get_use_light_pollution_removal() const;

            void set_light_pollution_gradient(const std::vector<std::unique_ptr<AstroPhotoStacker::LightPollutionGradientBase>> &light_pollution_gradient);

            const std::vector<std::shared_ptr<AstroPhotoStacker::LightPollutionGradientBase>>& get_light_pollution_gradient() const;

            template<typename PixelType>
            std::vector<std::vector<PixelType>> post_process_image(const std::vector<std::vector<PixelType>> &image, int width, int height) const {
                std::vector<std::vector<PixelType>> processed_image = image;

                if (m_use_light_pollution_removal) {
                    if (m_light_pollution_gradient.size() != processed_image.size()) {
                        throw std::runtime_error("Number of gradient functions must match the number of color channels in the image");
                    }
                    for (unsigned int i_color = 0; i_color < m_light_pollution_gradient.size(); i_color++) {
                        processed_image[i_color] = AstroPhotoStacker::subtract_gradient(processed_image[i_color], width, height, *m_light_pollution_gradient[i_color]);
                    }
                }

                if (m_apply_rgb_alignment) {
                    AstroPhotoStacker::RGBAlignmentTool rgb_alignment_tool = AstroPhotoStacker::RGBAlignmentTool(processed_image, width, height);
                    if (m_use_auto_rgb_alignment) {
                        std::pair<float,float> auto_shift_red, auto_shift_blue;
                        rgb_alignment_tool.get_blue_shift_and_red_shift(&auto_shift_blue, &auto_shift_red, processed_image, width, height);
                        rgb_alignment_tool.calculate_shifted_image(auto_shift_red, auto_shift_blue);
                    }
                    else {
                        rgb_alignment_tool.calculate_shifted_image(m_shift_red, m_shift_blue);
                    }
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
            std::pair<float,float> m_shift_red = {0,0};
            std::pair<float,float> m_shift_blue = {0,0};

            bool m_use_auto_rgb_alignment = false;

            bool m_use_light_pollution_removal = false;
            std::vector<std::shared_ptr<AstroPhotoStacker::LightPollutionGradientBase>> m_light_pollution_gradient;

    };
}