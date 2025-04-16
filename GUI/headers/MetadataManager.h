#pragma once

#include "../../headers/MetadataReader.h"
#include "../../headers/Metadata.h"
#include "../../headers/InputFrame.h"

#include <string>
#include <map>


class MetadataManager {
    public:
        MetadataManager() = default;

        AstroPhotoStacker::Metadata get_metadata(const AstroPhotoStacker::InputFrame &input_frame);

    private:
        std::map<std::string,AstroPhotoStacker::Metadata> m_metadata_map;
};
