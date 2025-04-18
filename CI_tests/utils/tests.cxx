#include "../headers/HotPixelIdentification_test.h"
#include "../headers/KDTreeTest.h"
#include "../headers/TestMetadataReader.h"
#include "../headers/ImageReadingTest.h"
#include "../headers/TestLocalShifts.h"

#include "../headers/TestUtils.h"


#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv)   {
    TestRunner test_runner;
    //test_runner.run_test("hot_pixel_identifier",    hot_pixel_identification_test,
    //                    "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/",
    //                    "temp_hot_pixel_file.txt",
    //                    "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/reference_files/hot_pixels.txt");

    test_runner.run_test("kd_tree",                 test_kd_tree);

    test_runner.run_test("Metadata reading - Canon 6D MarkII",    test_metadata_reading,
                        "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/IMG_9138.CR2",
                        6.3, 180.80f, 1600, 600.f, "RGGB", -1);

    test_runner.run_test("Metadata reading ZWO678MC",    test_metadata_reading,
                            "AstroPhotoStacker_test_files/data/ZWO678MC_horse_head/Light_FOV_180.0s_Bin1_678MC_20241226-001229_0001.fit",
                            0, 180.0f, 100, 1197.f, "RGGB", 1735164577);

    test_runner.run_test(   "Image reading - raw Canon 6D Mark II", test_image_reading_raw,
                            InputFrame("AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/IMG_9138.CR2"),
                            std::pair<int,int>{6264, 4180},
                            std::vector<std::tuple<int, int, short int, char>>{
                               {0, 0, 557, 0},
                               {1200, 2679, 605, 1},
                               {2455, 1237, 918, 2},
                               {1944, 2908, 1377, 0},
                               {1711, 1989, 679, 2},
                               {2908, 1944, 1167, 0},
                               {2212, 2013, 942, 1},
                           });


    test_runner.run_test(   "Image reading - raw ZWO 678MC", test_image_reading_raw,
                            InputFrame("AstroPhotoStacker_test_files/data/ZWO678MC_horse_head/Light_FOV_180.0s_Bin1_678MC_20241226-001229_0001.fit"),
                            std::pair<int,int>{3840,2160},
                            std::vector<std::tuple<int, int, short int, char>>{
                               {0, 0, 1896, 0},
                               {2455, 1237, 1808, 2},
                               {1711, 1989, 1680, 2},
                               {2908, 1944, 2400, 0},
                               {2212, 2013, 1968, 1},
                           });


    test_runner.run_test(   "Local shifts calculation: Moon", test_predefined_alignment_boxes,
        InputFrame("AstroPhotoStacker_test_files/data/moon_jpg/original.jpg"),
        InputFrame("AstroPhotoStacker_test_files/data/moon_jpg/shifted.jpg"),
        std::vector<std::tuple<int,int,int,int>>{
            {589,586, 50,50},
            {1052, 274, 50, 50},
            {1546, 237, 50, 50},
            {554,  991, 30, 30},
        },
        std::vector<std::pair<int,int>>{
            {7, 12},
            {0, 0},
            {-7,2},
            {0,-7},
        });

    test_runner.summarize_tests();

}
