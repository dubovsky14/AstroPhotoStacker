#pragma once
#include "../headers/StackerBase.h"

#include <vector>
#include <mutex>

namespace AstroPhotoStacker {

    /**
     * @brief Class for stacking photos using mean value stacking algorithm. It is the most basic algorithm, which usually does not work very well
     */
    class StackerMeanValue : public StackerBase {
        public:

            /**
             * @brief Construct a new Stacker Mean Value object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - not implemented yet, it's there just for compatibility with other stacking algorithms
            */
            StackerMeanValue(int number_of_colors, int width, int height, bool interpolate_colors);

            /**
             * @brief Calculate the stacked photo
            */
            virtual void calculate_stacked_photo() override;

            /**
             * @brief Set the number of CPU threads to be used for stacking
             *
             * @param n_cpu - number of CPU threads
            */
            virtual void set_number_of_cpu_threads(unsigned int n_cpu) override;

            /**
             * @brief Get the number of CPU threads used for stacking
             *
             * @return unsigned int - number of CPU threads
            */
            virtual int get_tasks_total() const override;

        protected:
            std::vector<std::vector<unsigned short>> m_number_of_stacked_pixels;

            std::vector<std::mutex> m_mutexes;
            std::vector<std::vector<std::vector<int>>>                  m_values_to_stack_individual_threads;  // [thread][color][pixel]
            std::vector<std::vector<std::vector<short unsigned int>>>   m_counts_to_stack_individual_threads;  // [thread][color][pixel]

            /**
             * @brief Get number of pixel lines that we can proces at once (limited by memory usage)
             *
             * @return int - number of pixel lines that we can proces at once
            */
            virtual int get_height_range_limit() const;

            virtual void add_photo_to_stack(unsigned int file_index, int y_min, int y_max) override;
    };
}