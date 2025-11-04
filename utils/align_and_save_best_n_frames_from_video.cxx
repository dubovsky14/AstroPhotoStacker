
#include "../headers/InputArgumentsParser.h"
#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/VideoReader.h"
#include "../headers/MetadataReader.h"
#include "../headers/FitFileSaver.h"
#include "../headers/InputFrameReader.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>


using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv) {
    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string video_file         = input_arguments_parser.get_argument<string>("video_file");
        const int n_frames              = input_arguments_parser.get_argument<int>("n_frames");
        const int n_cpu                 = input_arguments_parser.get_argument<int>("n_cpu");
        const string output_folder      = input_arguments_parser.get_argument<string>("output_folder");
        const int n_bit_depth           = input_arguments_parser.get_optional_argument<int>("n_bit_depth", 8);
        const string reference_frame    = input_arguments_parser.get_optional_argument<string>("reference_frame", "");

        // check values
        if (n_bit_depth != 8 && n_bit_depth != 16) {
            throw runtime_error("n_bit_depth should be either 8 or 16");
        }

        InputFrame reference_input_frame =  reference_frame == "" ?
                                            InputFrame(video_file, 0) : InputFrame(reference_frame);

        vector<InputFrame> video_frames = get_video_frames(video_file);

        PhotoAlignmentHandler photo_alignment_handler;
        photo_alignment_handler.set_alignment_method("planetary without rotation");
        photo_alignment_handler.set_number_of_cpu_threads(n_cpu);
        photo_alignment_handler.align_files(reference_input_frame, video_frames);

        vector<std::pair<InputFrame, float>> frame_ranking_pairs;
        const std::map<InputFrame, std::unique_ptr<AlignmentResultBase>>& alignment_results_map = photo_alignment_handler.get_alignment_results_map();
        for (const auto &pair : alignment_results_map) {
            const InputFrame &frame = pair.first;
            const AlignmentResultBase &alignment_result = *pair.second;
            frame_ranking_pairs.emplace_back(frame, alignment_result.get_ranking_score());
        }

        std::sort(frame_ranking_pairs.begin(), frame_ranking_pairs.end(),
                  [](const std::pair<InputFrame, float> &a, const std::pair<InputFrame, float> &b) {
                      return a.second < b.second;
                  });


        // Select the best N frames
        const int n_best_frames = std::min(n_frames, static_cast<int>(frame_ranking_pairs.size()));
        ofstream ranking_file(output_folder + "/frame_ranking.txt");
        for (int i = 0; i < n_best_frames; ++i) {
            InputFrameReader frame_reader(frame_ranking_pairs[i].first);
            const Metadata metadata = frame_reader.get_metadata();
            const std::vector<PixelType> &raw_data = frame_reader.get_raw_data();
            const int width = frame_reader.get_width();
            const int height = frame_reader.get_height();

            FitFileSaver fit_file_saver(width, height);
            fit_file_saver.set_metadata(metadata);
            fit_file_saver.set_bits_per_pixel(n_bit_depth);
            const string output_file = output_folder + "/frame_" + to_string(i+1) + ".fit";
            fit_file_saver.save(output_file, raw_data);
            const AlignmentResultBase &alignment_result = *alignment_results_map.at(frame_ranking_pairs[i].first);
            ranking_file << output_file << " | -1 | "
                         << " | " << alignment_result.get_description_string() << endl;

        }

    }
    catch (const exception &e) {
        cout << endl << endl;
        cout << e.what() << endl;
        abort();
    }
    return 0;
}