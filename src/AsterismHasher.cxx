#include "../headers/AsterismHasher.h"

#include <string>
#include <iostream>
#include <cmath>
#include <stdexcept>

using namespace std;
using namespace PlateSolver;

// for more details see chapter 2.2 here: https://arxiv.org/pdf/0910.2233.pdf
bool PlateSolver::calculate_asterism_hash(const vector<tuple<float, float, int> > &stars, tuple<float,float,float,float> *result,
    unsigned int *index_star_A, unsigned int *index_star_B, unsigned int *index_star_C, unsigned int *index_star_D)   {

    if (stars.size() != 4)  {
        throw runtime_error("Cannot calculate hash of #stars != 4");
    }

    // extract indices of stars A, B, C and D. For now symmetric agains A <-> B and C <-> D swap, we will solve this later
    int starA(-1), starB(-1), starC(-1), starD(-1);
    get_indices_of_most_distant_stars(stars, &starA, &starB);
    for (int i_star = 0; i_star < 4; i_star++) {
        if (i_star == starA || i_star == starB) continue;
        if (starC < 0)  {
            starC = i_star;
            continue;
        }
        starD = i_star;
        break;
    }

    if (index_star_A != nullptr)    {*index_star_A = starA;}
    if (index_star_B != nullptr)    {*index_star_B = starB;}
    if (index_star_C != nullptr)    {*index_star_C = starC;}
    if (index_star_D != nullptr)    {*index_star_D = starD;}


    // do C and D fit into the circle with diameter |AB| and center in the middle of A and B? if not, return false
    const float radius2 = 0.5*get_star_distance_squared(stars[starA], stars[starB]);
    const tuple<float,float, int>    center_of_circle(0.5*(get<0>(stars[starA]) + get<0>(stars[starB])), 0.5*(get<1>(stars[starA]) + get<1>(stars[starB])),0);
    if (radius2 < get_star_distance_squared(center_of_circle, stars[starC]))    return false;
    if (radius2 < get_star_distance_squared(center_of_circle, stars[starD]))    return false;

    // calculate axis of the coordinate system given by stars A and B
    const float axisAB[] = {get<0>(stars[starB]) - get<0>(stars[starA]), get<1>(stars[starB]) - get<1>(stars[starA])};
    const float axisX[] = {float((axisAB[0] + axisAB[1])*0.5),float((axisAB[1] - axisAB[0])*0.5)};
    const float axisY[] = {float((axisAB[0] - axisAB[1])*0.5),float((axisAB[1] + axisAB[0])*0.5)};
    const float axis_length2_inverse = 1/(axisX[0]*axisX[0] + axisX[1]*axisX[1]);

    // position of stars C and D, still with the coordinate system of pixels, but with the center in star A
    const float vectorAC[] = {get<0>(stars[starC]) - get<0>(stars[starA]), get<1>(stars[starC]) - get<1>(stars[starA])};
    const float vectorAD[] = {get<0>(stars[starD]) - get<0>(stars[starA]), get<1>(stars[starD]) - get<1>(stars[starA])};

    float Xc = axis_length2_inverse*calculate_coordinate_along_axis(vectorAC, axisX);
    float Yc = axis_length2_inverse*calculate_coordinate_along_axis(vectorAC, axisY);
    float Xd = axis_length2_inverse*calculate_coordinate_along_axis(vectorAD, axisX);
    float Yd = axis_length2_inverse*calculate_coordinate_along_axis(vectorAD, axisY);

    // breaking symmetry for A <-> B swapping
    if (Xc + Xd > 1)    {
        Xc = 1-Xc;
        Yc = 1-Yc;
        Xd = 1-Xd;
        Yd = 1-Yd;
        if (index_star_A != nullptr)    {*index_star_A = starB;}
        if (index_star_B != nullptr)    {*index_star_B = starA;}
    }

    // breaking symmetry for C <-> D swapping
    if (Xc <= Xd)   {
        get<0>(*result) = Xc;
        get<1>(*result) = Yc;
        get<2>(*result) = Xd;
        get<3>(*result) = Yd;
    }
    else {
        get<0>(*result) = Xd;
        get<1>(*result) = Yd;
        get<2>(*result) = Xc;
        get<3>(*result) = Yc;

        if (index_star_C != nullptr)    {*index_star_C = starD;}
        if (index_star_D != nullptr)    {*index_star_D = starC;}
    }
    return true;
};

void PlateSolver::get_indices_of_most_distant_stars(const std::vector<std::tuple<float, float, int> > &stars, int *star1, int *star2)   {
    float max_distance2 = -1;
    for (unsigned int i_star1 = 0; i_star1 < 4; i_star1++)  {
        const float star1_x(get<0>(stars[i_star1])), star1_y(get<1>(stars[i_star1]));
        for (unsigned int i_star2 = 0; i_star2 < i_star1; i_star2++)  {
            const float star2_x(get<0>(stars[i_star2])), star2_y(get<1>(stars[i_star2]));
            const float distance2 = pow2(star2_x-star1_x) + pow2(star2_y-star1_y);
            if (distance2 > max_distance2)  {
                *star1 = i_star1;
                *star2 = i_star2;
                max_distance2 = distance2;
            }
        }
    }
};

float PlateSolver::get_star_distance_squared(const std::tuple<float, float, int> &star1, const std::tuple<float, float, int> &star2)  {
    return pow2(get<0>(star1) - get<0>(star2)) + pow2(get<1>(star1) - get<1>(star2));
};
