#include "../headers/HotPixelIdentifier.h"
#include "../headers/raw_file_reader.h"
#include "../headers/Common.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <fstream>

#include "../headers/thread_pool.h"

using namespace std;
using namespace AstroPhotoStacker;



void HotPixelIdentifier::add_photos(const std::vector<std::string> &photo_addresses)    {
    thread_pool pool(m_n_cpu);
    for (const auto &photo_address : photo_addresses) {
        pool.submit([this, photo_address]() {
            add_photo(photo_address);
        });
    }
    pool.wait_for_tasks();
};

void HotPixelIdentifier::add_photo(const std::string &photo_address)    {
    int width,height;
    unique_ptr<unsigned short int[]> pixel_value_array = read_raw_file(photo_address, &width, &height);
    add_photo(pixel_value_array.get(), width, height, 14);
};

void HotPixelIdentifier::add_photo(const unsigned short int *pixel_value_array, int width, int height, int image_bit_depth) {
    m_n_photos_processed++;
    const auto hot_pixel_candidates = get_hot_pixel_candidates_from_photo(pixel_value_array, width, height, image_bit_depth);
    {
        scoped_lock lock(m_mutex);
        for (const auto &hot_pixel_candidate : hot_pixel_candidates) {
            const auto &hot_pixel_candidate_coordinates = hot_pixel_candidate.first;
            if (m_hot_pixel_candidates.find(hot_pixel_candidate_coordinates) == m_hot_pixel_candidates.end()) {
                m_hot_pixel_candidates[hot_pixel_candidate_coordinates] = 1;
            }
            else {
                m_hot_pixel_candidates[hot_pixel_candidate_coordinates]++;
            }
        }
    }
};


void HotPixelIdentifier::compute_hot_pixels()   {
    m_hot_pixels.clear();
    for (const auto &hot_pixel_candidate : m_hot_pixel_candidates) {
        const auto &hot_pixel_candidate_coordinates = hot_pixel_candidate.first;
        const auto &hot_pixel_candidate_value = hot_pixel_candidate.second;
        if (hot_pixel_candidate_value > m_n_photos_processed*0.5) {
            m_hot_pixels[hot_pixel_candidate_coordinates] = true;
        }
    }
};


vector<tuple<int,int>> HotPixelIdentifier::get_hot_pixels() const   {
    vector<tuple<int,int>> hot_pixels;
    for (const auto &hot_pixel : m_hot_pixels) {
        const auto &hot_pixel_coordinates = hot_pixel.first;
        const auto &hot_pixel_count = hot_pixel.second;
        if (hot_pixel_count > m_n_photos_processed*0.5) {
            hot_pixels.push_back(hot_pixel_coordinates);
        }
    }
    sort(hot_pixels.begin(), hot_pixels.end());
    return hot_pixels;
};

std::map<std::tuple<int,int>,int> HotPixelIdentifier::get_hot_pixel_candidates_from_photo(const unsigned short int *pixel_value_array, int width, int height, int image_bit_depth) {
    std::map<std::tuple<int,int>, int> hot_pixel_candidates;
    const int max_value = pow(2, image_bit_depth)-1;
    int hot_pixel_threshold = 0.8*max_value;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const int current_pixel_index = y*width + x;
            const int current_pixel_value = pixel_value_array[current_pixel_index];
            if (current_pixel_value < hot_pixel_threshold) {
                continue;
            }

            bool is_hot_pixel = true;
            for (int i_shift_y = -1; i_shift_y <= 1; i_shift_y++) {
                const int neighbor_y = y + i_shift_y;
                if (neighbor_y < 0 || neighbor_y >= height) {
                    continue;
                }
                for (int i_shift_x = -1; i_shift_x <= 1; i_shift_x++) {
                    if (i_shift_x == 0 && i_shift_y == 0) {
                        continue;
                    }
                    const int neighbor_x = x + i_shift_x;
                    if (neighbor_x < 0 || neighbor_x >= width) {
                        continue;
                    }
                    const int neighbor_pixel_index = current_pixel_index + i_shift_x + i_shift_y*width;
                    const int neighbor_pixel_value = pixel_value_array[neighbor_pixel_index];
                    if (neighbor_pixel_value > current_pixel_value*0.52) {
                        is_hot_pixel = false;
                        break;
                    }
                }
            }
            if (is_hot_pixel) {
                hot_pixel_candidates[std::make_tuple(x,y)] = 1;
            }
        }
    }
    return hot_pixel_candidates;
}


void HotPixelIdentifier::save_hot_pixels_to_file(const std::string &file_address) const {
    ofstream output_file(file_address);
    output_file << "# hot pixel x | hot pixel y" << endl;
    for (const auto &hot_pixel : m_hot_pixels) {
        const auto &hot_pixel_coordinates = hot_pixel.first;
        output_file << get<0>(hot_pixel_coordinates) << " | " << get<1>(hot_pixel_coordinates) << endl;
    }
    output_file.close();
};

void HotPixelIdentifier::load_hot_pixels_from_file(const std::string &file_address) {\
    m_hot_pixels.clear();
    ifstream input_file(file_address);
    if (!input_file.is_open()) {
        throw runtime_error("Could not open file " + file_address);
    }
    string line;
    while (getline(input_file, line)) {
        if (line[0] == '#') {
            continue;
        }
        const auto elements = split_and_strip_string(line, "|");
        const int x = stoi(elements[0]);
        const int y = stoi(elements[1]);
        m_hot_pixels[std::make_tuple(x,y)] = true;
    }
    input_file.close();
};

void HotPixelIdentifier::set_n_cpu(unsigned int n_cpu) {
    m_n_cpu = n_cpu;
};

bool HotPixelIdentifier::is_hot_pixel(int x, int y) const   {
    return m_hot_pixels.find(std::make_tuple(x,y)) != m_hot_pixels.end();
};

const std::atomic<int>& HotPixelIdentifier::get_number_of_processed_photos() const  {
    return m_n_photos_processed;
};