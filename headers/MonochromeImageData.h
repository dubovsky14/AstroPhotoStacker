#pragma once

#include <vector>
#include <memory>


namespace AstroPhotoStacker {

    /**
     * @brief Struct holding pointer to brightness data and image resolution for monochrome images. It does not own the data.
     */
    struct MonochromeImageData {
        const short *brightness = nullptr;
        int width  = 0;
        int height = 0;
    };

    /**
     * @brief Struct holding pointer to brightness data and image resolution for monochrome images. It owns the data.
     */
    struct MonochromeImageDataWithStorage : public MonochromeImageData {
        std::unique_ptr<std::vector<short>> brightness_storage = nullptr;

        MonochromeImageDataWithStorage(int image_width, int image_height) {
            this->width  = image_width;
            this->height = image_height;
            brightness_storage = std::make_unique<std::vector<short>>(width * height);
            brightness = brightness_storage->data();
        }
    };


}