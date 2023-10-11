#pragma once

#include "../headers/KDTree.h"

#include <vector>
#include <tuple>

namespace AstroPhotoStacker {
    class PlateSolver   {
        public:
            PlateSolver() = delete;

            PlateSolver(const KDTree *kdtree,
                        const std::vector<std::tuple<float,float,int> > *reference_stars,
                        unsigned int reference_photo_width, unsigned int reference_photo_height);

            bool plate_solve(   const std::vector<std::tuple<float,float,int> > &stars,
                                float *shift_x, float *shift_y,
                                float *rot_center_x, float *rot_center_y, float *rotation) const;


        private:
            const KDTree *m_kdtree;
            const std::vector<std::tuple<float,float,int> > *m_reference_stars;
            unsigned int m_reference_photo_width;
            unsigned int m_reference_photo_height;

            bool validate_hypothesis(   const std::vector<std::tuple<float,float,int> > &stars,
                                        float shift_x, float shift_y,
                                        float rot_center_x, float rot_center_y, float rotation) const;

            bool has_paired_star(float x, float y, float position_error = 3)    const;
    };
}


