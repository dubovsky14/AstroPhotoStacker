#include "../headers/HotPixelIdentification_test.h"
#include "../headers/KDTreeTest.h"
#include "../headers/TestMetadataReader.h"
#include "../headers/ImageReadingTest.h"
#include "../headers/TestLocalShifts.h"
#include "../headers/TestFitFileSaver.h"
#include "../headers/AsterismHashTests.h"

#include "../headers/TestUtils.h"


#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv)   {
    TestRunner test_runner;


    test_runner.run_test("kd_tree",                 test_kd_tree);

    test_runner.run_test("Metadata reading - Canon 6D MarkII",    test_metadata_reading,
                        "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/IMG_9138.CR2",
                        6.3, 180.80f, 1600, 600.f, "RGGB", "Canon 6D Mark II", -1);

    test_runner.run_test("Metadata reading ZWO678MC",    test_metadata_reading,
                            "AstroPhotoStacker_test_files/data/ZWO678MC_horse_head/Light_FOV_180.0s_Bin1_678MC_20241226-001229_0001.fit",
                            0, 180.0f, 100, 1197.f, "RGGB", "ZWO 678MC", 1735164577);

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

    test_runner.run_test("Saving 6D Mark II raw file into into file", test_metadata_fit_file_saver,
        InputFrame("AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/IMG_9138.CR2"),
        "output_tests/IMG_9138_test.fit",
        16);

    test_runner.run_test("Saving ZWO 678MC fit file into fit file", test_metadata_fit_file_saver,
        InputFrame("AstroPhotoStacker_test_files/data/ZWO678MC_horse_head/Light_FOV_180.0s_Bin1_678MC_20241226-001229_0001.fit"),
        "output_tests/Light_FOV_180.0s_Bin1_678MC_20241226-001229_0001_test.fit",
        16);

    test_runner.run_test("Saving ZWO 678MC video frame into 8-bit fit file", test_metadata_fit_file_saver,
        InputFrame("AstroPhotoStacker_test_files/data/Jupiter_video/shortened_jupiter.avi", 3),
        "output_tests/jupiter_video_frame3.fit",
        8);


    test_runner.run_test("Asterism hash test #1", test_asterism_hasher,
        std::vector<std::tuple<float,float,int>>{
            {0  , 0  , 1},
            {2  , 3  , 2},
            {1  , 1  , 3},
            {1.5, 2.5, 4},
        },
        std::vector<float>{0.231, 0.154, 0.538, 0.692},
        1, 0, 3, 2);

    test_runner.run_test("Asterism hash test #2", test_asterism_hasher,
        std::vector<std::tuple<float,float,int>>{
            {1.6, 4.0, 1},
            {0.54045, 6.62247, 1},
            {0.95128, 4.77509, 1},
            {0.48508, 5.92898, 1},
        },
        std::vector<float>{0.23, 0.45, 0.67, 0.89},
        0,1,2,3);

    test_runner.run_test("Asterism hash test #3", test_asterism_hasher,
        std::vector<std::tuple<float,float,int>>{
            {0.95128, 4.77509, 1},
            {0.54045, 6.62247, 1},
            {0.48508, 5.92898, 1},
            {1.6, 4.0, 1},
        },
        std::vector<float>{0.23, 0.45, 0.67, 0.89},
        3,1,0,2);

    test_runner.summarize_tests();

}
