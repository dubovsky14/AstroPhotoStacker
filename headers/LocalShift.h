#pragma once

namespace AstroPhotoStacker {
    struct LocalShift {
        int x;
        int y;
        int dx;
        int dy;
        bool valid_ap;
        float score;
    };
}