#include "../headers/ReferencePhotoHandlerStars.h"
#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/AsterismHasher.h"
#include "../headers/PhotoRanker.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <map>

using namespace std;
using namespace AstroPhotoStacker;


ReferencePhotoHandlerStars::ReferencePhotoHandlerStars(const InputFrame &input_frame, float threshold_fraction) :
    ReferencePhotoHandlerBase(input_frame, threshold_fraction) {
    const vector<unsigned short> brightness = read_image_monochrome<unsigned short>(input_frame, &m_width, &m_height);
    initialize(brightness.data(), m_width, m_height, threshold_fraction);
};


void ReferencePhotoHandlerStars::initialize(const unsigned short *brightness, int width, int height, float threshold_fraction)  {
    const unsigned short threshold = get_threshold_value<unsigned short>(&brightness[0], width*height, threshold_fraction);
    std::vector<std::tuple<float, float, int> > stars = get_stars(&brightness[0], width, height, threshold);
    keep_only_stars_above_size(&stars, 9);
    sort_stars_by_size(&stars);
    initialize(stars, width, height);
};

void ReferencePhotoHandlerStars::initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height)  {
    m_stars = stars;
    m_width = width;
    m_height = height;
    calculate_and_store_hashes();
    m_plate_solver = make_unique<PlateSolver>(m_kd_tree.get(), &m_stars, m_width, m_height);
};

PlateSolvingResult ReferencePhotoHandlerStars::calculate_alignment(const InputFrame &input_frame, float *ranking)   const    {
    try {
        int width, height;
        const vector<unsigned short> brightness = read_image_monochrome<unsigned short>(input_frame, &width, &height);
        const unsigned short threshold = get_threshold_value<unsigned short>(brightness.data(), width*height, 0.0005);

        vector<tuple<float,float,int> > stars = get_stars(brightness.data(), width, height, threshold);
        keep_only_stars_above_size(&stars, 9);
        sort_stars_by_size(&stars);
        stars.resize(min<int>(stars.size(), 20));

        if (ranking != nullptr) {
            *ranking = PhotoRanker::calculate_frame_ranking(input_frame);
        }

        return plate_solve(stars);
    }
    catch (runtime_error &e)    {
        cout << "Error: " << e.what() << endl;
        PlateSolvingResult plate_solving_result;
        plate_solving_result.is_valid = false;
        return plate_solving_result;
    }
};


PlateSolvingResult ReferencePhotoHandlerStars::plate_solve(const std::vector<std::tuple<float, float, int> > &stars) const    {
    return m_plate_solver->plate_solve(stars);
};

void ReferencePhotoHandlerStars::calculate_and_store_hashes()  {
    m_kd_tree = make_unique<KDTree<float, 4, tuple<unsigned, unsigned, unsigned, unsigned>>>();

    const unsigned int n_stars = m_stars.size();

    // to avoid duplicit hashes
    map<tuple<unsigned int,unsigned int,unsigned int,unsigned int>, char > used_hashes;

    // we want to calculate the hashes for all possible combinations of 4 stars from leading 20 stars
    const unsigned int n_stars_all_hashes = min(n_stars, 20u);
    vector<float> asterism_hash_buffer;
    for (unsigned int i_star1 = 0; i_star1 < n_stars_all_hashes; i_star1++)   {
        for (unsigned int i_star2 = i_star1+1; i_star2 < n_stars_all_hashes; i_star2++)   {
            for (unsigned int i_star3 = i_star2+1; i_star3 < n_stars_all_hashes; i_star3++)   {
                for (unsigned int i_star4 = i_star3+1; i_star4 < n_stars_all_hashes; i_star4++)   {

                    // yeah, this is not exactly efficient, but we do it only once (for the reference photo)
                    vector<tuple<float,float,int> > four_stars_positions = {m_stars[i_star1], m_stars[i_star2], m_stars[i_star3], m_stars[i_star4]};
                    vector<unsigned int> four_stars_indices = {i_star1, i_star2, i_star3, i_star4};

                    unsigned int starA, starB, starC, starD;
                    calculate_asterism_hash(four_stars_positions, &asterism_hash_buffer, &starA, &starB, &starC, &starD);

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

    // now some random hashes
    for (unsigned int i_hash = 0; i_hash < 2000; i_hash++)   {
        vector<unsigned int> four_stars_indices;
        for (unsigned int i_star = 0; i_star < 4; i_star++) {
            four_stars_indices.push_back(rand()%n_stars);
        }

        // to avoid duplicit hashes
        sort(four_stars_indices.begin(), four_stars_indices.end());

        tuple<unsigned int, unsigned int, unsigned int, unsigned int> these_indices(
            four_stars_indices[0],
            four_stars_indices[1],
            four_stars_indices[2],
            four_stars_indices[3]
        );

        if (used_hashes.find(these_indices) == used_hashes.end())   {
            continue;
        }
        used_hashes[these_indices] = 1;

        // ok, duplicit hashes eliminated, now we can calculate the hash itself and store it
        vector<tuple<float,float,int> > four_stars_positions = {
            m_stars[four_stars_indices[0]],
            m_stars[four_stars_indices[1]],
            m_stars[four_stars_indices[2]],
            m_stars[four_stars_indices[3]]
        };

        unsigned int starA, starB, starC, starD;
        calculate_asterism_hash(four_stars_positions, &asterism_hash_buffer, &starA, &starB, &starC, &starD);

        tuple<unsigned int, unsigned int, unsigned int, unsigned int> asterism_hash_indices(
            four_stars_indices[starA],
            four_stars_indices[starB],
            four_stars_indices[starC],
            four_stars_indices[starD]
        );

        m_kd_tree->add_point(asterism_hash_buffer, asterism_hash_indices);
    }

    m_kd_tree->build_tree_structure();
}
