#include "../CI_tests/headers/HotPixelIdentification_test.h"


#include <iostream>
#include <string>
#include <stdexcept>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv)   {
    try    {
        if (argc < 2) {
                throw std::string("Invalid input! One input argument is required: Type of the test");
        }

        const std::string test_type = argv[1];
        if (test_type == "hot_pixel_identifier")    hot_pixel_identification_test(argc, argv);
    }
    catch(const std::exception& e)    {
        std::cerr << e.what() << endl;
    }
}
