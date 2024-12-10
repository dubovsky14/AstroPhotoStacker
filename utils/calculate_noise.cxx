#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"
#include "../headers/InputFrame.h"

#include <vector>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, char **argv) {
    if (argc != 2)  {
        cerr << "Usage: " << argv[0] << " <raw_file>" << endl;
        return 1;
    }

    const std::string input_file = argv[1];
    int width, height;
    vector<char> colors;
    vector<unsigned short> brightness = read_raw_file<unsigned short>(InputFrame(input_file), &width, &height, &colors);

    double mean = 0;
    double mean2 = 0;
    for (unsigned short value : brightness) {
        mean += value;
        mean2 += value*value;
    }

    mean /= brightness.size();
    mean2 /= brightness.size();

    const double variance = mean2 - mean*mean;
    const double std_dev = sqrt(variance);

    cout << "Average noise level: " << mean << "\t\tRMS: " << std_dev << endl;


    return 0;
}