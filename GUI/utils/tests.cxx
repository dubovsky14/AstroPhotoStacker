#include "../../headers/TimeLapseVideoCreator.h"
#include "../../headers/Common.h"

#include <iostream>
#include <string>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;

int main(const int argc, const char *argv[]) {
    const string input_folder = argv[1];

    TimeLapseVideoCreator timelapse_creator;
    for (const auto &entry : filesystem::directory_iterator(input_folder)) {
        const string file = entry.path().string();
        bool is_jpg = ends_with(to_upper_copy(file), ".JPG");
        if (!is_jpg) {
            continue;
        }
        timelapse_creator.add_image(file, 0);
    }
    timelapse_creator.get_video_settings()->set_fps(25);
    timelapse_creator.get_video_settings()->set_n_repeat(5);
    timelapse_creator.create_video("video_test.avi", false);
}