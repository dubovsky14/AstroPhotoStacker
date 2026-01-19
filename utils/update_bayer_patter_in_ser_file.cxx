/**
 * Update the Bayer pattern code in a SER file, for example after using X (and/or) Y flipping in FireCapture.
 *
 * Usage:
 *   ./update_bayer_pattern_in_ser_file <filename> <new bayer pattern code>
 * Where <new bayer pattern code> is an integer defined in the SER file specification:
 */

#include <fstream>
#include <iostream>

int main(int argc, const char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <filename> <new bayer pattern code (int number defined in SER specification)>\n";
        return 1;
    }

    const char* filename = argv[1];
    int newBayerPatternCode = std::stoi(argv[2]);

    unsigned char newByte = static_cast<unsigned char>(newBayerPatternCode);
    const std::streamoff  position = 18; // defined in SER file specification

    // Open file for reading and writing in binary mode
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    // Move write position to the desired byte
    file.seekp(position);
    if (!file)
    {
        std::cerr << "Seek failed\n";
        return 1;
    }

    // Write exactly one byte
    file.write(reinterpret_cast<const char*>(&newByte), 1);
    if (!file)
    {
        std::cerr << "Write failed\n";
        return 1;
    }

    file.close();
    return 0;
}
