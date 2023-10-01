#include "../headers/PlateSolver.h"
#include "../headers/GeometricTransformations.h"
#include "../headers/AsterismHasher.h"
#include "../headers/Common.h"

using namespace AstroPhotoStacker;
using namespace std;

PlateSolver::PlateSolver(   const KDTree *kdtree,
                            const vector<tuple<float,float,int> > *stars,
                            unsigned int reference_photo_width, unsigned int reference_photo_height) :
    m_kdtree(kdtree),
    m_reference_stars(stars),
    m_reference_photo_width(reference_photo_width),
    m_reference_photo_height(reference_photo_height)  {};


bool PlateSolver::plate_solve(  const std::vector<std::tuple<float,float,int> > &stars,
                                float *shift_x, float *shift_y, float *rot_center_x, float *rot_center_y, float *rotation) const   {
    for (unsigned int i_star1 = 0; i_star1 < stars.size(); i_star1++)   {
        for (unsigned int i_star2 = i_star1+1; i_star2 < stars.size(); i_star2++)   {
            for (unsigned int i_star3 = i_star2+1; i_star3 < stars.size(); i_star3++)   {
                for (unsigned int i_star4 = i_star3+1; i_star4 < stars.size(); i_star4++)   {

                    vector<tuple<float,float,int> > four_stars_positions = {stars[i_star1], stars[i_star2], stars[i_star3], stars[i_star4]};
                    vector<unsigned int> four_stars_indices = {i_star1, i_star2, i_star3, i_star4};
                    tuple<float,float,float,float> asterism_hash;

                    unsigned int starA, starB, starC, starD;
                    calculate_asterism_hash(four_stars_positions, &asterism_hash, &starA, &starB, &starC, &starD);

                    // closest hashes and star indices from the reference photo
                    const std::vector<std::tuple<PointCoordinatesTuple, StarIndices> > nearest_neighbors =
                        m_kdtree->get_k_nearest_neighbors(asterism_hash, 4);

                    for (const std::tuple<PointCoordinatesTuple, StarIndices> &hash_tuple : nearest_neighbors)  {
                        const StarIndices &reference_star_indices = get<1>(hash_tuple);
                        const tuple<float,float,int> &reference_star_A = m_reference_stars->at(get<0>(reference_star_indices));
                        const tuple<float,float,int> &reference_star_B = m_reference_stars->at(get<1>(reference_star_indices));

                        const tuple<float,float,int> &this_photo_star_A = stars[four_stars_indices[starA]];
                        const tuple<float,float,int> &this_photo_star_B = stars[four_stars_indices[starB]];

                        // calculate shift and rotation
                        *shift_x = get<0>(reference_star_A) - get<0>(this_photo_star_A);
                        *shift_y = get<1>(reference_star_A) - get<1>(this_photo_star_A);
                        *rot_center_x = get<0>(this_photo_star_A);
                        *rot_center_y = get<1>(this_photo_star_A);
                        *rotation = atan2(get<1>(reference_star_B) - get<1>(reference_star_A), get<0>(reference_star_B) - get<0>(reference_star_A)) -
                            atan2(get<1>(this_photo_star_B) - get<1>(this_photo_star_A), get<0>(this_photo_star_B) - get<0>(this_photo_star_A));

                        if (validate_hypothesis(stars, *shift_x, *shift_y, *rot_center_x, *rot_center_y, *rotation))   {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
};


bool PlateSolver::validate_hypothesis(  const std::vector<std::tuple<float,float,int> > &stars,
                                        float shift_x, float shift_y,
                                        float rot_center_x, float rot_center_y, float rotation) const   {
    const GeometricTransformer geometric_transformer(shift_x, shift_y, rot_center_x, rot_center_y, rotation);
    unsigned int n_stars_in_reference_frame(0), n_stars_in_reference_frame_paired(0);

    for (const tuple<float,float,int> &star : stars)   {
        float x = get<0>(star);
        float y = get<1>(star);
        geometric_transformer.transform_to_reference_frame(&x, &y);
        if (x >= 0 && x < m_reference_photo_width && y >= 0 && y < m_reference_photo_height)   {
            n_stars_in_reference_frame++;
            if (has_paired_star(x,y))   {
                n_stars_in_reference_frame_paired++;
            }
        }
    }
    return n_stars_in_reference_frame_paired > 0.5*n_stars_in_reference_frame;
};


bool PlateSolver::has_paired_star(float x, float y, float position_error) const   {
    const float position_error2 = pow2(position_error);
    for (const tuple<float,float,int> &star : *m_reference_stars)   {
        if (pow2(get<0>(star) - x) + pow2(get<1>(star) - y) < position_error2)   {
            return true;
        }
    }
    return false;
};
