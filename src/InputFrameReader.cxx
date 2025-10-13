#include "../headers/InputFrameReader.h"

#include "../headers/InputFormatTypes.h"
#include "../headers/NonRawFrameReaderFactory.h"
#include "../headers/RawFileReaderFactory.h"

#include "../headers/Debayring.h"

using namespace AstroPhotoStacker;
using namespace std;

InputFrameReader::InputFrameReader(const InputFrame &input_frame) : m_input_frame(input_frame) {
    m_is_raw_before_debayering = AstroPhotoStacker::is_raw_file(input_frame.get_file_address());
    m_is_raw_file = m_is_raw_before_debayering;
    m_data_are_loaded = false;
};

void InputFrameReader::load_input_frame_data() {
    if (m_data_are_loaded) {
        return;
    }
    if (m_is_raw_before_debayering) {
        read_raw();
    }
    else {
        read_non_raw();
    }
    m_data_are_loaded = true;
};

bool InputFrameReader::data_are_loaded() const {
    return m_data_are_loaded;
};

Metadata InputFrameReader::get_metadata() const {
    if (m_data_are_loaded) {
        return m_metadata;
    }

    if (!is_raw_file()) {
        unique_ptr<NonRawFrameReaderBase> non_raw_frame_reader = NonRawFrameReaderFactory::get_non_raw_frame_reader(m_input_frame);
        return non_raw_frame_reader->read_metadata();
    }
    else {
        unique_ptr<RawFileReaderBase> raw_file_reader = RawFileReaderFactory::get_raw_file_reader(m_input_frame);
        return raw_file_reader->read_metadata();
    }
};

bool InputFrameReader::is_raw_file() const {
    return m_is_raw_file;
};

bool InputFrameReader::is_raw_file_before_debayering() const {
    return m_is_raw_before_debayering;
};

void InputFrameReader::debayer() {
    if (!m_is_raw_file) {
        return;
    }
    if (!m_data_are_loaded) {
        load_input_frame_data();
    }
    if (!m_is_raw_before_debayering) {
        return;
    }

    m_rgb_data = debayer_raw_data(m_raw_data, m_width, m_height, m_bayer_pattern);
    m_raw_data.clear();
    m_is_raw_before_debayering = false;
};


const std::vector<std::vector<PixelType>>& InputFrameReader::get_rgb_data() {
    if (!m_data_are_loaded) {
        load_input_frame_data();
    }
    return m_rgb_data;
};

std::vector<PixelType> InputFrameReader::get_monochrome_data() {
    if (!m_data_are_loaded) {
        load_input_frame_data();
    }

    if (m_raw_data.size() > 0) {
        std::vector<PixelType> debayered_data = m_raw_data;
        debayer_monochrome(&debayered_data, m_width, m_height, m_bayer_pattern);
        return debayered_data;
    }

    if (m_rgb_data.size()== 0) {
        return {};
    }

    std::vector<PixelType> result(m_rgb_data[0].size(), 0);
    const int n_channels = m_rgb_data.size();
    for (size_t i = 0; i < m_rgb_data[0].size(); i++) {
        int value = 0;
        for (int c = 0; c < n_channels; c++) {
            value += static_cast<int>(m_rgb_data[c][i]);
        }
        result[i] = value / n_channels;
    }
    return result;
};

void InputFrameReader::get_photo_resolution(int *width, int *height){
    if (!m_data_are_loaded) {
        load_input_frame_data();
    }
    *width = m_width;
    *height = m_height;
};

PixelType InputFrameReader::get_pixel_value(int x, int y, int channel) const {
    if (m_is_raw_before_debayering) {
        const int bayer_index = (y%2)*2 + (x%2);
        if (m_bayer_pattern[bayer_index] != channel) {
            return -1;
        }
        return m_raw_data[x + y * m_width];
    }
    return m_rgb_data[channel][x + y * m_width];
}

PixelType InputFrameReader::get_pixel_value(int pixel_index, int channel) const   {
    if (m_is_raw_before_debayering) {
        const int bayer_index = (pixel_index / m_width) * 2 + (pixel_index % m_width) % 2;
        if (m_bayer_pattern[bayer_index] != channel) {
            return -1;
        }
        return m_raw_data[pixel_index];
    }
    if (channel >= static_cast<int>(m_rgb_data.size())) {
        return -1;
    }
    return m_rgb_data[channel][pixel_index];
};

std::vector<std::vector<PixelType>*> InputFrameReader::get_all_data_for_calibration() {
    vector<vector<PixelType>*> result;
    const unsigned int resolution = m_width * m_height;
    if (m_raw_data.size() == resolution) {
        result.push_back(&m_raw_data);
    }
    for (auto &channel_data : m_rgb_data) {
        if (channel_data.size() == resolution) {
            result.push_back(&channel_data);
        }
    }
    return result;
}

char InputFrameReader::get_raw_color(int x, int y) const  {
    const int bayer_index = (y%2)*2 + (x%2);
    return m_bayer_pattern[bayer_index];
};

void InputFrameReader::read_raw() {
    unique_ptr<RawFileReaderBase> raw_file_reader = RawFileReaderFactory::get_raw_file_reader(m_input_frame);
    m_raw_data = raw_file_reader->read_raw_file(&m_width, &m_height, &m_bayer_pattern);
    m_metadata = raw_file_reader->read_metadata();
    m_is_raw_before_debayering = true;
    m_is_raw_file = true;
};

void InputFrameReader::read_non_raw() {
    unique_ptr<NonRawFrameReaderBase> non_raw_frame_reader = NonRawFrameReaderFactory::get_non_raw_frame_reader(m_input_frame);
    m_rgb_data = non_raw_frame_reader->get_pixels_data(&m_width, &m_height);
    m_metadata = non_raw_frame_reader->read_metadata();
    m_is_raw_before_debayering = false;
    m_is_raw_file = false;
};

