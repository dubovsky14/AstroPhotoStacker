#pragma once

#include <vector>

namespace AstroPhotoStacker {
    class CalibratedPhotoScoreHandler   {
        public:
            CalibratedPhotoScoreHandler() = default;

            bool empty() const;
            bool has_local_score() const;

            void set_global_score(float score);
            float get_global_score() const;

            void initialize_local_scores(unsigned int width, unsigned int height, float initial_value);
            void set_local_score(unsigned int x, unsigned int y, float score);
            float get_local_score(unsigned int x, unsigned int y) const;

        private:
            unsigned int m_width  = 0;
            unsigned int m_height = 0;
            float m_global_score  = 1;
            bool  m_is_empty = true;

            bool m_local_score_set = false;
            std::vector<float> m_scores_local;
    };
};