#pragma once

#include "../headers/InputFrame.h"
#include "../headers/Metadata.h"
#include "../headers/ThreadSafeCacheSystem.h"
#include "../headers/PixelType.h"

#include <string>
#include <vector>
#include <array>
#include <map>

namespace AstroPhotoStacker {

    /**
     * @brief Base class for reading frames (both raw and non-raw), taking care of metadata caching
     */
    class FrameReaderBase {
        public:
            virtual ~FrameReaderBase() = default;

            FrameReaderBase() = delete;

            FrameReaderBase(const InputFrame &input_frame);

            Metadata read_metadata();

        protected:
            InputFrame m_input_frame;

            virtual Metadata read_metadata_without_cache() = 0;

            static ThreadSafeCacheSystem<std::string, Metadata> s_metadata_cache;

            void scale_8bit_image_to_16bit(std::vector<std::vector<PixelType>> *image_data);

            void scale_8bit_image_to_16bit(std::vector<PixelType> *image_data);
    };
}