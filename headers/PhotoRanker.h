#pragma once

#include <string>
#include <vector>

namespace AstroPhotoStacker     {

    /**
     * @brief Class to rank photos based on the tracking performance of the mount
    */
    class PhotoRanker   {
        public:
            PhotoRanker()   = delete;

            /**
             * @brief Construct a new Photo Ranker object
             *
             * @param path_to_input_files - vector of strings containing the path to the input files
            */
            PhotoRanker(const std::vector<std::string> &path_to_input_files);

            /**
             * @brief Construct a new Photo Ranker object
             *
             * @param path_to_lights_folder - path to the folder containing the lights
            */
            PhotoRanker(const std::string& path_to_lights_folder);

            /**
             * @brief Run the ranking for all files defined previously in the constructor
            */
            void rank_all_files();

            /**
             * @brief Run the ranking for a single file
             *
             * @param file_address - path to the file to be ranked
             * @return float - the ranking of the file
            */
            static float calculate_file_ranking(const std::string &file_address);

            /**
             * @brief Calculate excentricity of a cluster, defined as distance between 2 most distant points minus diameter of a circle with the same area
             *
             * @param file_address - path to the file to be ranked
             * @return float - excentricity
            */
            static float get_cluster_excentricity(const std::vector<std::tuple<int,int>> &cluster);

            /**
             * @brief Get the ranking of all files
             *
             * @return std::vector<std::tuple<std::string,float>> - vector of tuples containing the file path and the ranking, ordered by ranking
            */
            std::vector<std::tuple<std::string,float>> get_ranking() const;


        private:
            std::vector<std::string>    m_input_files;
            std::vector<float>          m_ranking;


    };
}