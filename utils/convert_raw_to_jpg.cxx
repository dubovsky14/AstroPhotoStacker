#include "../headers/raw_file_reader.h"
#include "../headers/ImageFilesInputOutput.h"

#include <vector>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;

std::vector<std::string> get_file_paths(const std::string& path) {
    std::vector<std::string> file_paths;

    if (std::filesystem::is_directory(path)) {
        for (const auto & entry : std::filesystem::directory_iterator(path)) {
            file_paths.push_back(entry.path().string());
        }
    } else if (std::filesystem::is_regular_file(path)) {
        file_paths.push_back(path);
    }

    return file_paths;
}

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

            //downscale green
            if (i_color == 1 || i_color == 3)   {
                image->at(i_color)[i_pixel] /= 2;
            }
            else {
                image->at(i_color)[i_pixel] = min<int>(image->at(i_color)[i_pixel], 128);
            }
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3)  {
        cerr << "Usage: " << argv[0] << " <raw_file> <output_file>" << endl;
        return 1;
    }

    const std::string input_address = argv[1];
    const std::string output_address = argv[2];
    std::vector<std::string> file_paths = get_file_paths(argv[1]);
    for (const string &input_file : file_paths)  {
        // get file name
        const string raw_file = input_file.substr(input_file.find_last_of("/\\") + 1);

        // drop extension
        const string raw_file_wo_extension = raw_file.substr(0, raw_file.find_last_of("."));

        const string output_file = output_address + "/" + raw_file_wo_extension + ".jpg";

        int width, height;
        vector<char> colors;
        vector<unsigned short> brightness = read_raw_file<unsigned short>(input_file, &width, &height, &colors);
        cout << colors.size() << endl;

        auto image = convert_raw_data_to_rgb_image(brightness.data(), colors.data(), width, height);
        scale_to_8_bits(&image, width, height);
        crate_color_image(&image[0][0],&image[1][0], &image[2][0], width, height, output_file);
    }

    return 0;
}