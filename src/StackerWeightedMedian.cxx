#include "../headers/StackerWeightedMedian.h"
#include "../headers/CalibratedPhotoHandler.h"

#include "../headers/thread_pool.h"

#include <iostream>
#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

const ScoreType StackerWeightedMedian::c_default_score = 0;

StackerWeightedMedian::StackerWeightedMedian(int number_of_colors, int width, int height, bool interpolate_colors) :
    StackerBase(number_of_colors, width, height, interpolate_colors)   {
};

void StackerWeightedMedian::calculate_stacked_photo()  {
    const long long int n_files = m_files_to_stack.size();
    const int height_range = get_height_range_limit();

    if (height_range == 0) {
        throw runtime_error("The memory set by the user is not sufficient, please increase it");
    }

    for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
        m_values_to_stack.push_back(vector<std::tuple<short,ScoreType>>(m_width*height_range*n_files, {-1,c_default_score}));
    }

    int i_slice = 0;
    int n_slices = m_height/height_range + (m_height % height_range > 0);
    for (int y_min = 0; y_min < m_height; y_min += height_range) {

        i_slice++;
        const int y_max = min(y_min + height_range, m_height);
        cout << "Stacking slice " << i_slice << " of " << n_slices << endl;

        auto submit_photo_stack = [this, y_min, y_max](unsigned int file_index) {
            cout << string("Adding ") + m_files_to_stack[file_index] + string(" to stack\n");
            add_photo_to_stack(file_index, y_min, y_max);
            m_n_tasks_processed++;
        };

        thread_pool pool(m_n_cpu);
        for (unsigned int i_file = 0; i_file < m_files_to_stack.size(); i_file++) {
            if (m_n_cpu > 1) {
                pool.submit(submit_photo_stack, i_file);
            }
            else {
                submit_photo_stack(i_file);
            }
        }
        pool.wait_for_tasks();
        auto submit_median_calculation = [this, y_min, y_max](int i_color, int y_final_array, int y_values_to_stack_array ) {
            process_line(y_final_array, y_values_to_stack_array, i_color);
        };

        for (int i_color = 0; i_color < m_number_of_colors; i_color++) {
            for (int y_final_array = y_min; y_final_array < y_max; y_final_array++) {
                const int y_values_to_stack_array = y_final_array - y_min;
                if (m_n_cpu > 1) {
                    pool.submit(submit_median_calculation, i_color, y_final_array, y_values_to_stack_array);
                }
                else {
                    submit_median_calculation(i_color, y_final_array, y_values_to_stack_array);
                }
            }
        }
        pool.wait_for_tasks();
    }


    m_values_to_stack.clear();

    // fix green pixels
    if (m_number_of_colors == 3 && !m_interpolate_colors) {
        for (int i_pixel = 0; i_pixel < m_width*m_height; i_pixel++) {
            m_stacked_image[1][i_pixel] /= 2;
        }
    }
    fix_empty_pixels();
};

void StackerWeightedMedian::add_photo_to_stack(unsigned int file_index, int y_min, int y_max)  {
    const unsigned long long int n_files = m_files_to_stack.size();

    CalibratedPhotoHandler calibrated_photo = get_calibrated_photo(file_index, y_min, y_max);

    if (m_interpolate_colors)   {
        for (int color = 0; color < 3; color++)   {
            const unsigned int slice_shift = y_min*m_width;
            for (int y = y_min; y < y_max; y++)  {
                const unsigned int start_of_line_index_reference_frame = y*m_width;
                const unsigned int start_of_line_index_this_slice = (y*m_width - slice_shift)*n_files + file_index;
                for (int x = 0; x < m_width; x++)   {
                    const short value = calibrated_photo.get_value_by_reference_frame_index(start_of_line_index_reference_frame+x, color);
                    const ScoreType score = value < 0 ? c_default_score : calibrated_photo.get_score(x,y);
                    m_values_to_stack[color][start_of_line_index_this_slice + x*n_files] = {value, score};

                }
            }
        }
    }
    else    {
        short int value;
        char color;
        for (int y = y_min; y < y_max; y++)  {
            for (int x = 0; x < m_width; x++)   {
                calibrated_photo.get_value_by_reference_frame_coordinates(x, y, &value, &color);
                const unsigned int index_stacking_array = (y-y_min)*m_width + x;
                if (color >= 0) {
                    const ScoreType score = value < 0 ? c_default_score : calibrated_photo.get_score(x,y);
                    m_values_to_stack[color][n_files*index_stacking_array + file_index] = {value, score};
                }
            }
        }
    }
};


int StackerWeightedMedian::get_height_range_limit() const {
    int height_range = m_height;
    const long long int n_files = m_files_to_stack.size();
    if (m_memory_usage_limit_in_mb > 0) {
        const unsigned long long int memory_needed_for_stacked_image = 3*sizeof(double)*m_width*m_height;
        const unsigned long long int memory_needed_for_calibrated_photos = m_n_cpu*3*sizeof(unsigned short)*m_width*m_height;
        const unsigned long long int memory_usage_limit = m_memory_usage_limit_in_mb*1024ULL*1024ULL - memory_needed_for_stacked_image - memory_needed_for_calibrated_photos;
        const unsigned long long int memory_usage_per_line = m_number_of_colors*m_width*n_files*(sizeof(unsigned short) + sizeof(ScoreType));
        height_range = min(height_range, int(memory_usage_limit/memory_usage_per_line));
    }
    return height_range;
};


void StackerWeightedMedian::process_line(int y_index_final_array, int y_index_values_to_stack_array, int i_color)    {
    const long long int n_files = m_files_to_stack.size();
    for (int i_width = 0; i_width < m_width; i_width++) {
        const unsigned long long int pixel_index = m_width*y_index_final_array + i_width;
        const unsigned long long int pixel_index_stacking_array = m_width*y_index_values_to_stack_array*n_files + i_width*n_files;

        tuple<short,ScoreType> *slice_begin = &m_values_to_stack[i_color][pixel_index_stacking_array];
        tuple<short,ScoreType> *slice_end   = &m_values_to_stack[i_color][pixel_index_stacking_array + n_files];

        int number_of_stacked_pixels = 0;
        for (int i = 0; i < n_files; i++) {
            if (std::get<1>(slice_begin[i]) != c_default_score) {
                number_of_stacked_pixels++;
            }
        }
        int first_non_negative_index = n_files - number_of_stacked_pixels;

        sort(slice_begin, slice_end, [](const tuple<short,ScoreType> &a, const tuple<short,ScoreType> &b) {
            return std::get<1>(a) < std::get<1>(b);
        });

        m_stacked_image[i_color][pixel_index] = get_stacked_value_from_pixel_array(slice_begin + first_non_negative_index, number_of_stacked_pixels);
    }
};


double StackerWeightedMedian::get_stacked_value_from_pixel_array(tuple<short,ScoreType> *ordered_array_begin, unsigned int number_of_stacked_pixels) {
    if (number_of_stacked_pixels == 0) {
        return c_empty_pixel_value;
    }

    sort(ordered_array_begin, ordered_array_begin + number_of_stacked_pixels, [](const tuple<short,ScoreType> &a, const tuple<short,ScoreType> &b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    ScoreSumType sum_of_scores = 0;
    for (unsigned int i_pixel = 0; i_pixel < number_of_stacked_pixels; i_pixel++) {
        sum_of_scores += std::get<1>(ordered_array_begin[i_pixel]);
    }

    ScoreSumType sum_of_scores_half = sum_of_scores/2;
    ScoreSumType sum_of_scores_so_far = 0;
    for (unsigned int i_pixel = 0; i_pixel < number_of_stacked_pixels; i_pixel++) {
        sum_of_scores_so_far += std::get<1>(ordered_array_begin[i_pixel]);
        if (sum_of_scores_so_far >= sum_of_scores_half) {
            return std::get<0>(ordered_array_begin[i_pixel]);
        }
    }


    return std::get<0>(ordered_array_begin[number_of_stacked_pixels-1]); // should never happen, but compiler will complain if it is not here
};

int StackerWeightedMedian::get_tasks_total() const  {
    const long long int n_files = m_files_to_stack.size();
    const int height_range = get_height_range_limit();
    int n_slices = m_height/height_range + (m_height % height_range > 0);

    return n_slices*n_files;
};