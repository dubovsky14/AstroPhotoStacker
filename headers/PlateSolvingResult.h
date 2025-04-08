#pragma once

namespace AstroPhotoStacker {
    struct PlateSolvingResult   {
        float shift_x           = 0;
        float shift_y           = 0;
        float rotation_center_x = 0;
        float rotation_center_y = 0;
        float rotation          = 0;
        bool  is_valid          = false;
    };
}