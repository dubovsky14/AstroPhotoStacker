// for more details of the algorithm see: https://arxiv.org/pdf/0910.2233.pdf
#pragma once

#include <tuple>
#include <vector>

namespace AstroPhotoStacker   {

    /**
     * @brief Calculate asterism hash as defined in this paper: https://arxiv.org/pdf/0910.2233.pdf
     *
     * @param stars vector of pixel coordinates of 4 stars
     * @param result resulting hash in form of tuple: Xc,Yc,Xd,Yd
     * @param index_star_A if not nullptr, it sets this variable to the index of the corresponding star from the input vector
     * @param index_star_B if not nullptr, it sets this variable to the index of the corresponding star from the input vector
     * @param index_star_C if not nullptr, it sets this variable to the index of the corresponding star from the input vector
     * @param index_star_D if not nullptr, it sets this variable to the index of the corresponding star from the input vector
     * @return true - valid hash
     * @return false - invalid hash (if 4 stars do not fit into the circle with diamater |AB| and center in the middle between star A and star B)
     */
    bool calculate_asterism_hash(   const std::vector<std::tuple<float, float, int > > &stars, std::tuple<float,float,float,float> *result,
                                    unsigned int *index_star_A = nullptr,
                                    unsigned int *index_star_B = nullptr,
                                    unsigned int *index_star_C = nullptr,
                                    unsigned int *index_star_D = nullptr);

    /**
     * @brief Get the indices of most distant stars from the vector of 4 inputs stars (their pixel coordinates)
     *
     * @param stars - vector of 4 stars described by their pixel coordinates
     * @param star1 - index of 1st star from the most distant pair
     * @param star2 - index of 2nd star from the most distant pair
     */
    void get_indices_of_most_distant_stars(const std::vector<std::tuple<float, float, int> > &stars, int *star1, int *star2);

    float get_star_distance_squared(const std::tuple<float, float, int> &star1, const std::tuple<float, float, int> &star2);

    inline float calculate_coordinate_along_axis(const float *original_vector, const float *axis)   {
        return (original_vector[0]*axis[0] + original_vector[1]*axis[1]);
    };
}