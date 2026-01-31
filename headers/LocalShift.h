#pragma once

namespace AstroPhotoStacker {
    struct LocalShift {
        int x;  // coordinates in the images which is being aligned
        int y;  // coordinates in the images which is being aligned
        int dx; // shift in x direction with respect to the reference image
        int dy; // shift in y direction with respect to the reference image
        bool valid_ap;
        float score;
    };
}