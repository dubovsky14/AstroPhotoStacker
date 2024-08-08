#pragma once

#include "../headers/KDTree.h"

#include <vector>
#include <tuple>

namespace AstroPhotoStacker {

    /**
     * @brief Class responsible for calculating how a photo should be rotated and shifted to match a reference photo
    */
    class PlateSolver   {
        public:
            PlateSolver() = delete;

            /**
             * @brief Construct a new Plate Solver object
             *
             * @param kdtree - pointer to the KDTree object containing the reference asterism hashes and indices of the reference stars
             * @param reference_stars - pointer to the vector containing the reference stars
             * @param reference_photo_width - width of the reference photo
             * @param reference_photo_height - height of the reference photo
            */
            PlateSolver(const KDTree<float, 4, std::tuple<unsigned, unsigned, unsigned, unsigned>> *kdtree,
                        const std::vector<std::tuple<float,float,int> > *reference_stars,
                        unsigned int reference_photo_width, unsigned int reference_photo_height);

            /**
             * @brief Calculate the shift, rotation and rotation center of the photo to match the reference photo
             *
             * @param stars - vector of tuples containing the x and y coordinates of the stars and number of their pixels
             * @param shift_x - pointer to the variable where the horizontal shift will be stored
             * @param shift_y - pointer to the variable where the vertical shift will be stored
             * @param rot_center_x - pointer to the variable where the x coordinate of the rotation center will be stored
             * @param rot_center_y - pointer to the variable where the y coordinate of the rotation center will be stored
             * @param rotation - pointer to the variable where the rotation angle will be stored
             * @return true - if the plate was solved
             * @return false - if the plate was not solved
            */
            bool plate_solve(   const std::vector<std::tuple<float,float,int> > &stars,
                                float *shift_x, float *shift_y,
                                float *rot_center_x, float *rot_center_y, float *rotation) const;


        private:
            const KDTree<float, 4, std::tuple<unsigned, unsigned, unsigned, unsigned>> *m_kdtree;
            const std::vector<std::tuple<float,float,int> > *m_reference_stars;
            unsigned int m_reference_photo_width;
            unsigned int m_reference_photo_height;

            bool validate_hypothesis(   const std::vector<std::tuple<float,float,int> > &stars,
                                        float shift_x, float shift_y,
                                        float rot_center_x, float rot_center_y, float rotation) const;

            bool has_paired_star(float x, float y, float position_error = 3)    const;
    };
}


