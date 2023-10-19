#pragma once

#include <string>
#include <vector>

namespace AstroPhotoStacker     {
    class PhotoRanker   {
        public:
            PhotoRanker()   = delete;

            PhotoRanker(const std::vector<std::string> &path_to_input_files);
            PhotoRanker(const std::string& path_to_lights_folder);

            void rank_all_files();

            static float calculate_file_ranking(const std::string &file_address);

            static float get_cluster_excentricity(const std::vector<std::tuple<int,int>> &cluster);

            std::vector<std::tuple<std::string,float>> get_ranking() const;


        private:
            std::vector<std::string>    m_input_files;
            std::vector<float>          m_ranking;


    };
}