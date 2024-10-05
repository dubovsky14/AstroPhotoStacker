#include "../headers/CalibratedPhotoScoreHandler.h"

using namespace AstroPhotoStacker;


bool CalibratedPhotoScoreHandler::empty() const    {
    return m_is_empty;
};

bool CalibratedPhotoScoreHandler::has_local_score() const    {
    return m_local_score_set;
};

void CalibratedPhotoScoreHandler::set_global_score(float score) {
    m_global_score = score;
    m_is_empty = false;
};

float CalibratedPhotoScoreHandler::get_global_score() const {
    return m_global_score;
};

void  CalibratedPhotoScoreHandler::initialize_local_scores(unsigned int width, unsigned int height, float initial_value)    {
    m_width = width;
    m_height = height;
    m_scores_local.resize(width*height, initial_value);
    m_local_score_set = true;
    m_is_empty = false;
};

void  CalibratedPhotoScoreHandler::set_local_score(unsigned int x, unsigned int y, float score) {
    m_scores_local.at(x + y*m_width) = score;
};

float CalibratedPhotoScoreHandler::get_local_score(unsigned int x, unsigned int y) const    {
    if (m_local_score_set)  {
        return m_scores_local.at(x + y*m_width);
    }
    return m_global_score;
};
