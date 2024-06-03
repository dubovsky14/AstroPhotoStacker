#pragma once
#include "../headers/StackerBase.h"

#include <vector>
#include <mutex>

namespace AstroPhotoStacker {

    /**
     * @brief Base class for stacking algorithms that do not require to keep all the values in memory
     */
    class StackerSimpleBase : public StackerBase {
        public:

            /**
             * @brief Construct a new Stacker Mean Value object
             *
             * @param number_of_colors - number of colors in the stacked photo
             * @param width - width of the photo
             * @param height - height of the photo
             * @param interpolate_colors - not implemented yet, it's there just for compatibility with other stacking algorithms
            */
            StackerSimpleBase(int number_of_colors, int width, int height, bool interpolate_colors);

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

            /**
             * @brief Get number of pixel lines that we can proces at once (limited by memory usage)
             *
             * @return int - number of pixel lines that we can proces at once
            */
            virtual int get_height_range_limit() const;

            virtual void add_photo_to_stack(unsigned int file_index, int y_min, int y_max) override;

            /**
             * @brief Put all the partial results together
            */
            virtual void calculate_final_image() = 0;

            /**
             * @brief Allocate arrays for stacking where partial results are stored
            */
            virtual void allocate_arrays_for_stacking() = 0;

            /**
             * @brief Clean up the arrays for stacking where partial results were stored
            */
            virtual void deallocate_arrays_for_stacking() = 0;


            /**
             * @brief Process the given pixel for given color
             *
             * @param i_color - color index
             * @param i_pixel - pixel index (it's the total index = y*width + x)
             * @param value - value of the pixel from individual photo
             * @param i_thread - thread index
            */
            virtual void process_pixel(int i_color, int i_pixel, int value, int i_thread) = 0;
    };
}