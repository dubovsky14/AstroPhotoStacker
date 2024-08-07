#pragma once



namespace AstroPhotoStacker {
    template<class ValueType>
    float get_sharpness_factor(const ValueType *image_data, unsigned int width, unsigned int height)    {
        double sum_weights          = 0;
        double sum_diff2_weighted   = 0;

        for (unsigned int y = 0; y < height; y++)    {
            for (unsigned int x = 0; x < width; x++)    {
                // right
                if (x < width - 1)  {
                    const double diff = image_data[y * width + x] - image_data[y * width + x + 1];
                    const double weight = std::max(image_data[y * width + x], image_data[y * width + x + 1]);
                    sum_diff2_weighted += diff * diff * weight;
                    sum_weights += weight;
                }

                // down
                if (y < height - 1)  {
                    const double diff = image_data[y * width + x] - image_data[(y + 1) * width + x];
                    const double weight = std::max(image_data[y * width + x], image_data[(y + 1) * width + x]);
                    sum_diff2_weighted += diff * diff * weight;
                    sum_weights += weight;
                }
            }
        }
        return sum_diff2_weighted / sum_weights;
    }


}