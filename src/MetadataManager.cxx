#include "../headers/MetadataManager.h"

using namespace AstroPhotoStacker;
using namespace std;

Metadata MetadataManager::get_metadata(const InputFrame &input_frame) {
    const string file_path = input_frame.get_file_address();
    if (m_metadata_map.find(file_path) == m_metadata_map.end()) {
        const Metadata metadata = read_metadata(input_frame);
        m_metadata_map[file_path] = metadata;
    }
    return m_metadata_map[file_path];
}