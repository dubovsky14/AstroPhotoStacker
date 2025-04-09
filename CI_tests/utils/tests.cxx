#include "../headers/HotPixelIdentification_test.h"
#include "../headers/KDTreeTest.h"
#include "../headers/TestUtils.h"


#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv)   {
    try    {

        TestRunner test_runner;
        test_runner.run_test("hot_pixel_identifier",    hot_pixel_identification_test,
                            "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/",
                            "temp_hot_pixel_file.txt",
                            "AstroPhotoStacker_test_files/data/CanonEOS6DMarkII_Andromeda/reference_files/hot_pixels.txt");
        test_runner.run_test("kd_tree",                 test_kd_tree);
        test_runner.summarize_tests();

    }
    catch(const std::exception& e)    {
        std::cerr << e.what() << endl;
    }
}
