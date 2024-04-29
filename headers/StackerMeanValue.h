#pragma once
#include "../headers/StackerBase.h"

#include <vector>
#include <mutex>

namespace AstroPhotoStacker {
    class StackerMeanValue : public StackerBase {
        public:
            StackerMeanValue(int number_of_colors, int width, int height, bool interpolate_colors);

            virtual void calculate_stacked_photo() override;

            virtual void set_number_of_cpu_threads(unsigned int n_cpu) override;

            virtual int get_tasks_total() const override;

        protected:
            std::vector<std::vector<unsigned short>> m_number_of_stacked_pixels;

            std::vector<std::mutex> m_mutexes;
            std::vector<std::vector<std::vector<int>>>                  m_values_to_stack_individual_threads;  // [thread][color][pixel]
            std::vector<std::vector<std::vector<short unsigned int>>>   m_counts_to_stack_individual_threads;  // [thread][color][pixel]

            void add_photo_to_stack(unsigned int i_file);
    };
}