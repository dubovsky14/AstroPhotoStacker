#include "../headers/VideoReaderSer.h"


void AstroPhotoStacker::read_uint_from_file(std::ifstream *file, unsigned int *value, size_t position_in_file)  {
    file->seekg(position_in_file, std::ios::beg);
    file->read(reinterpret_cast<char*>(value), sizeof(unsigned int));
};