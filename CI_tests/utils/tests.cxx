#include "../headers/HotPixelIdentification_test.h"
#include "../headers/KDTreeTest.h"
#include "../headers/TestMetadataReader.h"
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

    test_runner.run_test("metadata_reading_CanonR7",    test_metadata_reading,
                        "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/IMG_9138.CR2",
                        6.3, 180.80f, 1600, 600.f, "2023-09-08 07:56:14 PM");

    test_runner.run_test("metadata_reading_ZWO678MC",    test_metadata_reading,
                            "AstroPhotoStacker_test_files/data/ZWO678MC_horse_head/Light_FOV_180.0s_Bin1_678MC_20241226-001229_0001.fit",
                            0, 180.0f, 100, 1197.f, "2024-12-25T23:09:37.105251");

    test_runner.summarize_tests();

}
