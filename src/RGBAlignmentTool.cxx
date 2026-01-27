#include "../headers/RGBAlignmentTool.h"

#include "../headers/StarFinder.h"
#include "../headers/Common.h"
#include "../headers/CommonImageOperations.h"


using namespace std;
using namespace AstroPhotoStacker;


void RGBAlignmentTool::get_blue_shift_and_red_shift_internal(   std::pair<float,float> *blue_shift, std::pair<float,float> *red_shift,
                                                                const std::vector<std::vector<unsigned short>> &image, int width, int height)    const {


    if (image.size() != 3) {
        throw std::runtime_error("Input image must have 3 color channels");
    }

    std::pair<float,float> colors_center_of_mass[3];

    for (int i_color = 0; i_color < 3; i_color++) {
        const unsigned short otsu_threshold = max<unsigned short>(get_otsu_threshold(image[i_color].data(), width*height), 1);
        const unsigned short max_value = *max_element(image[i_color].data(), image[i_color].data() + width*height);
        const unsigned short threshold = max<unsigned short>(0.05*max_value, otsu_threshold);

        std::vector< std::vector<std::tuple<int, int> > > clusters = get_clusters_non_recursive(image[i_color].data(), width, height, threshold);
        std::sort(clusters.begin(), clusters.end(), []
                                    (const std::vector<std::tuple<int, int> > &a, const std::vector<std::tuple<int, int> > &b)
                                    {return a.size() > b.size();});

        if (clusters.size() == 0) {
            throw std::runtime_error("No clusters found in the reference photo");
        }
        if (clusters.at(0).size()  < 10) {
            throw std::runtime_error("No clusters found in the reference photo");
        }

        const std::vector<std::tuple<int,int>> &leading_cluster = clusters.at(0);

        double x_sum = 0;
        double y_sum = 0;

        for (const auto &[x,y] : leading_cluster) {
            x_sum += x;
            y_sum += y;
        }
        colors_center_of_mass[i_color] = std::make_pair(x_sum/leading_cluster.size(), y_sum/leading_cluster.size());

    }

    *blue_shift = std::make_pair<float,float>(colors_center_of_mass[1].first - colors_center_of_mass[2].first,
                                              colors_center_of_mass[1].second - colors_center_of_mass[2].second);

    *red_shift = std::make_pair<float,float>(colors_center_of_mass[1].first - colors_center_of_mass[0].first,
                                             colors_center_of_mass[1].second - colors_center_of_mass[0].second);
};