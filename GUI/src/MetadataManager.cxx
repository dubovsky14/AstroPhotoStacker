#include "../headers/MetadataManager.h"

using namespace std;

AstroPhotoStacker::Metadata MetadataManager::get_metadata(const AstroPhotoStacker::InputFrame &input_frame) {
    const string file_path = input_frame.get_file_address();
    if (m_metadata_map.find(file_path) == m_metadata_map.end()) {
        const AstroPhotoStacker::Metadata metadata = AstroPhotoStacker::read_metadata(input_frame);
        m_metadata_map[file_path] = metadata;
    }
    return m_metadata_map[file_path];
}