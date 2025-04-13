#include "../headers/PlateSolver.h"
#include "../headers/GeometricTransformations.h"
#include "../headers/AsterismHasher.h"
#include "../headers/Common.h"

using namespace AstroPhotoStacker;
using namespace std;

using StarIndices = tuple<unsigned, unsigned, unsigned, unsigned>;

PlateSolver::PlateSolver(   const KDTree<float, 4, std::tuple<unsigned, unsigned, unsigned, unsigned>> *kdtree,
                            const vector<tuple<float,float,int> > *stars,
                            unsigned int reference_photo_width, unsigned int reference_photo_height) :
    m_kdtree(kdtree),
    m_reference_stars(stars),
    m_reference_photo_width(reference_photo_width),
    m_reference_photo_height(reference_photo_height)  {};


PlateSolvingResult PlateSolver::plate_solve(  const std::vector<std::tuple<float,float,int> > &stars) const   {
    for (unsigned int i_star1 = 0; i_star1 < stars.size(); i_star1++)   {
        for (unsigned int i_star2 = i_star1+1; i_star2 < stars.size(); i_star2++)   {
            for (unsigned int i_star3 = i_star2+1; i_star3 < stars.size(); i_star3++)   {
                for (unsigned int i_star4 = i_star3+1; i_star4 < stars.size(); i_star4++)   {

                    vector<tuple<float,float,int> > four_stars_positions = {stars[i_star1], stars[i_star2], stars[i_star3], stars[i_star4]};
                    vector<unsigned int> four_stars_indices = {i_star1, i_star2, i_star3, i_star4};
                    vector<float> asterism_hash(4);

                    unsigned int starA, starB, starC, starD;
                    calculate_asterism_hash(four_stars_positions, &asterism_hash, &starA, &starB, &starC, &starD);

                    // closest hashes and star indices from the reference photo
                    const std::vector<std::tuple<std::array<float, 4>, StarIndices> > nearest_neighbors =
                        m_kdtree->get_k_nearest_neighbors(asterism_hash.data(), 4);

                    for (const tuple<std::array<float, 4>, StarIndices> &hash_tuple : nearest_neighbors)  {
                        const StarIndices &reference_star_indices = get<1>(hash_tuple);
                        const tuple<float,float,int> &reference_star_A = m_reference_stars->at(get<0>(reference_star_indices));
                        const tuple<float,float,int> &reference_star_B = m_reference_stars->at(get<1>(reference_star_indices));

                        const tuple<float,float,int> &this_photo_star_A = stars[four_stars_indices[starA]];
                        const tuple<float,float,int> &this_photo_star_B = stars[four_stars_indices[starB]];

                        // calculate shift and rotation
                        PlateSolvingResult plate_solving_result;
                        plate_solving_result.shift_x = get<0>(reference_star_A) - get<0>(this_photo_star_A);
                        plate_solving_result.shift_y = get<1>(reference_star_A) - get<1>(this_photo_star_A);
                        plate_solving_result.rotation_center_x = get<0>(this_photo_star_A);
                        plate_solving_result.rotation_center_y = get<1>(this_photo_star_A);
                        plate_solving_result.rotation = atan2(get<1>(reference_star_B) - get<1>(reference_star_A), get<0>(reference_star_B) - get<0>(reference_star_A)) -
                            atan2(get<1>(this_photo_star_B) - get<1>(this_photo_star_A), get<0>(this_photo_star_B) - get<0>(this_photo_star_A));
                        plate_solving_result.is_valid = true;

                        if (validate_hypothesis(stars, plate_solving_result))   {
                            return plate_solving_result;
                        }
                    }
                }
            }
        }
    }
    PlateSolvingResult plate_solving_result;
    plate_solving_result.is_valid = false;
    return plate_solving_result;
};


bool PlateSolver::validate_hypothesis(  const std::vector<std::tuple<float,float,int> > &stars,
                                        const PlateSolvingResult &plate_solving_result) const   {
    const GeometricTransformer geometric_transformer(plate_solving_result);
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
