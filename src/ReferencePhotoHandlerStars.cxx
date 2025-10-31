#include "../headers/ReferencePhotoHandlerStars.h"
#include "../headers/InputFrameReader.h"
#include "../headers/AsterismHasher.h"
#include "../headers/PhotoRanker.h"

#include "../headers/AlignmentResultPlateSolving.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <map>

using namespace std;
using namespace AstroPhotoStacker;


ReferencePhotoHandlerStars::ReferencePhotoHandlerStars(const InputFrame &input_frame, float threshold_fraction) :
    ReferencePhotoHandlerBase(input_frame, threshold_fraction) {
    const vector<PixelType> brightness = read_image_monochrome(input_frame, &m_width, &m_height);
    initialize(brightness.data(), m_width, m_height, threshold_fraction);
};


void ReferencePhotoHandlerStars::initialize(const PixelType *brightness, int width, int height, float threshold_fraction)  {
    const PixelType threshold = get_threshold_value<PixelType>(&brightness[0], width*height, threshold_fraction);
    std::vector<std::tuple<float, float, int> > stars = get_stars(&brightness[0], width, height, threshold);
    keep_only_stars_above_size(&stars, 9);
    sort_stars_by_size(&stars);

    m_minimal_number_of_pixels_per_star = stars.size() >= 25 ? get<2>(stars[24]) : get<2>(stars.back());
    keep_only_stars_above_size(&stars, m_minimal_number_of_pixels_per_star);
    if (stars.size() > 30) {
        stars.resize(30);
    }
    initialize(stars, width, height);
};

void ReferencePhotoHandlerStars::initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height)  {
    m_stars = stars;
    m_width = width;
    m_height = height;
    calculate_and_store_hashes();
    m_plate_solver = make_unique<PlateSolver>(m_kd_tree.get(), &m_stars, m_width, m_height);
};

std::unique_ptr<AlignmentResultBase> ReferencePhotoHandlerStars::calculate_alignment(const InputFrame &input_frame)   const    {
    try {
        int width, height;
        const vector<PixelType> brightness = read_image_monochrome(input_frame, &width, &height);
        const PixelType threshold = get_threshold_value(brightness.data(), width*height, 0.0005);

        vector<tuple<float,float,int> > stars = get_stars(brightness.data(), width, height, threshold);
        keep_only_stars_above_size(&stars, m_minimal_number_of_pixels_per_star*1.3);
        sort_stars_by_size(&stars);

        if (stars.size() > 25) {
            stars.resize(25);
        }

        unique_ptr<AlignmentResultBase> result = plate_solve(stars);
        result->set_ranking_score(PhotoRanker::calculate_frame_ranking(input_frame));
        return result;
    }
    catch (runtime_error &e)    {
        cout << "Error: " << e.what() << endl;
        return make_unique<AlignmentResultPlateSolving>();
    }
};


std::unique_ptr<AlignmentResultPlateSolving> ReferencePhotoHandlerStars::plate_solve(const std::vector<std::tuple<float, float, int> > &stars) const    {
    return std::make_unique<AlignmentResultPlateSolving>(m_plate_solver->plate_solve(stars));
};

void ReferencePhotoHandlerStars::calculate_and_store_hashes()  {
    m_kd_tree = make_unique<KDTree<float, 4, tuple<unsigned, unsigned, unsigned, unsigned>>>();

    const unsigned int n_stars = m_stars.size();

    // to avoid duplicit hashes
    map<tuple<unsigned int,unsigned int,unsigned int,unsigned int>, char > used_hashes;

    // we want to calculate the hashes for all possible combinations of 4 stars
    vector<float> asterism_hash_buffer;
    for (unsigned int i_star1 = 0; i_star1 < n_stars; i_star1++)   {
        for (unsigned int i_star2 = i_star1+1; i_star2 < n_stars; i_star2++)   {
            for (unsigned int i_star3 = i_star2+1; i_star3 < n_stars; i_star3++)   {
                for (unsigned int i_star4 = i_star3+1; i_star4 < n_stars; i_star4++)   {

                    // yeah, this is not exactly efficient, but we do it only once (for the reference photo)
                    vector<tuple<float,float,int> > four_stars_positions = {m_stars[i_star1], m_stars[i_star2], m_stars[i_star3], m_stars[i_star4]};
                    vector<unsigned int> four_stars_indices = {i_star1, i_star2, i_star3, i_star4};

                    unsigned int starA, starB, starC, starD;

                    const bool hash_found = calculate_asterism_hash(four_stars_positions, &asterism_hash_buffer, &starA, &starB, &starC, &starD);
                    if (!hash_found) {
                        continue;
                    }

                    tuple<unsigned int, unsigned int, unsigned int, unsigned int> asterism_hash_indices(
                        four_stars_indices[starA],
                        four_stars_indices[starB],
                        four_stars_indices[starC],
                        four_stars_indices[starD]
                    );

                    m_kd_tree->add_point(asterism_hash_buffer, asterism_hash_indices);

                    used_hashes[asterism_hash_indices] = 1;
                }
            }
        }
    }

    m_kd_tree->build_tree_structure();
}
