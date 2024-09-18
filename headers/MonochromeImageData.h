#pragma once


namespace AstroPhotoStacker {
    struct MonochromeImageData {
        const unsigned short *brightness = nullptr;
        int width  = 0;
        int height = 0;
    };
}