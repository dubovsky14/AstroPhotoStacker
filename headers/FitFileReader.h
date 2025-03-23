#pragma once

#include "../headers/Metadata.h"
#include "../headers/FitFileMetadataReader.h"

#include <string>
#include <fstream>
#include <memory>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <limits>

namespace AstroPhotoStacker {
    /**
     * @brief: Class for reading input images in FIT format
     */
    class FitFileReader : public FitFileMetadataReader {
        public:
            FitFileReader(const std::string &input_file);

            const std::vector<unsigned short int> &get_data() const {
                return m_data;
            }

            template<class ValueType>
            std::vector<ValueType> get_data_templated() const {
                std::vector<ValueType> brightness(m_width * m_height);
                for (unsigned int i_pixel = 0; i_pixel < m_data.size(); i_pixel++) {
                    const unsigned short int pixel = m_data[i_pixel];
                    brightness[i_pixel] = pixel;
                    if (pixel > std::numeric_limits<ValueType>::max()) {
                        std::cout << ("Value too large for the output type\n");
                        std::abort();
                    }
                }
                return brightness;
            }

        private:
            void read_data(std::ifstream &file);

            std::vector<unsigned short int> m_data;

    };
}