#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"

using namespace std;
using namespace AstroPhotoStacker;


void scale_to_8_bits(std::vector<std::vector<unsigned short> > *image, int width, int height)   {
    const int n_colors = image->size();
    // get maximum
    unsigned short max_value = 0;
    for (int i_color = 0; i_color < n_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < width*height; i_pixel++) {
            if (image->at(i_color)[i_pixel] > max_value) {
                max_value = image->at(i_color)[i_pixel];
            }
        }
    }

    // scale
    const double scale_factor = 255.0/max_value;
    for (int i_color = 0; i_color < n_colors; i_color++) {
        for (int i_pixel = 0; i_pixel < width*height; i_pixel++) {
            image->at(i_color)[i_pixel] *= scale_factor;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3)  {
        cerr << "Usage: " << argv[0] << " <raw_file> <output_file>" << endl;
        return 1;
    }
    const string raw_file = argv[1];
    const string output_file = argv[2];
    int width, height;
    vector<char> colors;
    unique_ptr<unsigned short[]> brightness = read_raw_file<unsigned short>(raw_file, &width, &height, &colors);
    cout << colors.size() << endl;

    auto image = convert_raw_data_to_rgb_image(brightness.get(), colors.data(), width, height);
    scale_to_8_bits(&image, width, height);
    crate_color_image(&image[0][0],&image[1][0], &image[2][0], width, height, output_file);
    return 0;
}