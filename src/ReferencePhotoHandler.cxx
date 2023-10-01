#include "../headers/ReferencePhotoHandler.h"
#include "../headers/raw_file_reader.h"
#include "../headers/AsterismHasher.h"

#include <vector>
#include <tuple>
#include <algorithm>
#include <map>

using namespace std;
using namespace AstroPhotoStacker;


ReferencePhotoHandler::ReferencePhotoHandler(const std::string &raw_file_address, float threshold_fraction) {
    unique_ptr<unsigned short[]> brightness = read_raw_file(raw_file_address, &m_width, &m_height);
    Initialize(&brightness[0], m_width, m_height, threshold_fraction);
};

void ReferencePhotoHandler::Initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height)  {
    m_stars = stars;
    m_width = width;
    m_height = height;
    CalculateAndStoreHashes();
};

tuple<tuple<float,float,float,float>,unsigned int, unsigned int, unsigned int, unsigned int> ReferencePhotoHandler::get_hash(unsigned int hash_index) const  {
    tuple<unsigned int, unsigned int, unsigned int, unsigned int> indices;
    float coordinates[4];

    m_kd_tree->get_point(hash_index, coordinates, &indices);

    return make_tuple(
        make_tuple(
            coordinates[0],
            coordinates[1],
            coordinates[2],
            coordinates[3]
        ),
        get<0>(indices),
        get<1>(indices),
        get<2>(indices),
        get<3>(indices)
    );
};

void ReferencePhotoHandler::CalculateAndStoreHashes()  {
    m_kd_tree = make_unique<KDTree>(10000);

    const unsigned int n_stars = m_stars.size();

    // to avoid duplicit hashes
    map<tuple<unsigned int,unsigned int,unsigned int,unsigned int>, char > used_hashes;

    // we want to calculate the hashes for all possible combinations of 4 stars from leading 20 stars
    const unsigned int n_stars_all_hashes = min(n_stars, 20u);
    for (unsigned int i_star1 = 0; i_star1 < n_stars_all_hashes; i_star1++)   {
        for (unsigned int i_star2 = i_star1+1; i_star2 < n_stars_all_hashes; i_star2++)   {
            for (unsigned int i_star3 = i_star2+1; i_star3 < n_stars_all_hashes; i_star3++)   {
                for (unsigned int i_star4 = i_star3+1; i_star4 < n_stars_all_hashes; i_star4++)   {

                    // yeah, this is not exactly efficient, but we do it only once (for the reference photo)
                    vector<tuple<float,float,int> > four_stars_positions = {m_stars[i_star1], m_stars[i_star2], m_stars[i_star3], m_stars[i_star4]};
                    vector<unsigned int> four_stars_indices = {i_star1, i_star2, i_star3, i_star4};
                    tuple<float,float,float,float> asterism_hash;

                    unsigned int starA, starB, starC, starD;
                    calculate_asterism_hash(four_stars_positions, &asterism_hash, &starA, &starB, &starC, &starD);

                    tuple<unsigned int, unsigned int, unsigned int, unsigned int> asterism_hash_indices(
                        four_stars_indices[starA],
                        four_stars_indices[starB],
                        four_stars_indices[starC],
                        four_stars_indices[starD]
                    );

                    m_kd_tree->add_point(asterism_hash, asterism_hash_indices);

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
        tuple<float,float,float,float> asterism_hash;
        calculate_asterism_hash(four_stars_positions, &asterism_hash, &starA, &starB, &starC, &starD);

        tuple<unsigned int, unsigned int, unsigned int, unsigned int> asterism_hash_indices(
            four_stars_indices[starA],
            four_stars_indices[starB],
            four_stars_indices[starC],
            four_stars_indices[starD]
        );

        m_kd_tree->add_point(asterism_hash, asterism_hash_indices);
    }

    m_kd_tree->create_tree_structure();
}