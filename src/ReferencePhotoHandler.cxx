#include "../headers/ReferencePhotoHandler.h"
#include "../headers/raw_file_reader.h"

using namespace std;


ReferencePhotoHandler::ReferencePhotoHandler(const std::string &raw_file_address, float threshold_fraction) {
    unique_ptr<unsigned short[]> brightness = read_raw_file(raw_file_address, &m_width, &m_height);
    Initialize(&brightness[0], m_width, m_height, threshold_fraction);
};

void ReferencePhotoHandler::Initialize(const std::vector<std::tuple<float, float, int> > &stars, int width, int height)  {
    m_stars = stars;
    m_width = width;
    m_height = height;
};