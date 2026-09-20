#pragma once

#include "../headers/FrameScore.h"
#include "../headers/InputFrame.h"
#include "../headers/PixelType.h"

#include <vector>

namespace AstroPhotoStacker {
    class FrameRankingTool {
        public:

            //static FrameScore get_ranking_for_deep_sky_objects(const InputFrame& frame, PixelType cluster_threshold);

            static FrameScore get_ranking_for_deep_sky_objects(const std::vector<PixelType>& frame, int width, int height, PixelType cluster_threshold);



            //static FrameScore get_ranking_for_planetary_objects(const InputFrame& frame, PixelType cluster_threshold);

            static FrameScore get_ranking_for_planetary_objects(const std::vector<PixelType>& frame, int width, int height, int gaussian_kernel_size, double gaussian_sigma);



            //static FrameScore get_ranking_otsu_based(const InputFrame& frame);

            static FrameScore get_ranking_otsu_based(const std::vector<PixelType>& frame, int width, int height);


            static FrameScore add_brighness_info(const std::vector<PixelType>& frame, FrameScore frame_score);
        private:
            FrameRankingTool() = delete;


    };
}