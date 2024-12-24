#pragma once

#include "../headers/MetadataReader.h"
#include "../headers/Metadata.h"
#include "../headers/InputFrame.h"

#include <string>
#include <map>


namespace AstroPhotoStacker {
    class MetadataManager {
        public:
            MetadataManager() = default;

            Metadata get_metadata(const InputFrame &input_frame);

        private:
            std::map<std::string,Metadata> m_metadata_map;
    };
}