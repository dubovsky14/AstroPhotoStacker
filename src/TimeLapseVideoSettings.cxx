#include "../headers/TimeLapseVideoSettings.h"

#include <string>
#include <stdexcept>

using namespace AstroPhotoStacker;
using namespace std;

void TimeLapseVideoSettings::set_fps(float fps)   {
    m_fps = fps;
};

float TimeLapseVideoSettings::get_fps()   const   {
    return m_fps;
};

void TimeLapseVideoSettings::set_n_repeat(int n_repeat) {
    m_n_repeat = n_repeat;
};

int TimeLapseVideoSettings::get_n_repeat()  const   {
    return m_n_repeat;
};

void TimeLapseVideoSettings::set_codec(const char codec[4]) {
    for (int i = 0; i < 4; i++) {
        m_codec[i] = codec[i];
    }
};

void TimeLapseVideoSettings::set_codec(const std::string &codec)    {
    if (codec.size() != 4) {
        throw std::invalid_argument("TimeLapseVideoSettings::set_codec: Codec must be 4 characters long");
    }
    for (int i = 0; i < 4; i++) {
        m_codec[i] = codec[i];
    }
};

const char* TimeLapseVideoSettings::get_codec() const   {
    return m_codec;
};

void TimeLapseVideoSettings::configure_timelapse_video_creator(TimeLapseVideoCreator *settings) const {
    settings->set_fps(m_fps);
    settings->set_n_repeat(m_n_repeat);
    settings->set_codec(m_codec);
};