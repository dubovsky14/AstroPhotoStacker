/**
 * @brief Program for creating a video with only frames that pass the quality filter. You provide the input video and alignment file and it produces a new video in SER format, containing only the N best frames.
 */


#include "../headers/VideoReader.h"
#include "../headers/InputArgumentsParser.h"
#include "../headers/FilelistHandler.h"
#include "../headers/VideoWriterSER.h"
#include "../headers/InputFrameReader.h"
#include "../headers/Common.h"
#include "../headers/CommonImageOperations.h"
#include "../headers/FrameType.h"
#include "../headers/AlignmentResultDummy.h"
#include "../headers/MetadataReader.h"

#include <string>
#include <iostream>
#include <vector>

using namespace std;
using namespace AstroPhotoStacker;

int main(int argc, const char **argv) {

    try {
        InputArgumentsParser input_arguments_parser(argc, argv);

        const string input_file_address         = input_arguments_parser.get_argument<string>("input_file");
        const string output_file_address        = input_arguments_parser.get_argument<string>("output_file");
        const string alignment_file_address     = input_arguments_parser.get_optional_argument<string>("alignment_file", "");
        const int   number_of_frames_to_keep    = input_arguments_parser.get_optional_argument<int>("number_of_frames_to_keep", -1);
        const float fraction_to_keep            = input_arguments_parser.get_optional_argument<float>("fraction_to_keep", -1.0f);
        const int   output_bit_depth            = input_arguments_parser.get_optional_argument<int>("output_bit_depth", 8);

        if ((number_of_frames_to_keep < 0) == (fraction_to_keep < 0.0)) {
            throw runtime_error("You must provide either number_of_frames_to_keep or fraction_to_keep");
        }

        vector<InputFrame> video_frames = get_video_frames(input_file_address);
        if (video_frames.empty()) {
            throw runtime_error("No frames found in input video: " + input_file_address);
        }

        FilelistHandler filelist_handler;
        for (const InputFrame &input_frame : video_frames) {
            const Metadata metadata = read_metadata(input_frame);
            filelist_handler.add_file(input_frame.get_file_address(), FrameType::LIGHT, 0, true, AlignmentResultDummy(), metadata);
        }

        if (!alignment_file_address.empty()) {
            filelist_handler.load_alignment_from_file(alignment_file_address);
        }

        if (!filelist_handler.all_checked_frames_are_aligned()) {
            throw runtime_error("Not all checked frames are aligned. Please provide an alignment file with alignment for all frames.");
        }

        const int n_frames_total = filelist_handler.get_number_of_checked_frames(FrameType::LIGHT);

        std::vector<std::pair<InputFrame, float>> frames_and_scores;
        const std::map<AstroPhotoStacker::InputFrame,FrameInfo> &frames_map = filelist_handler.get_frames(FrameType::LIGHT, 0);
        for (const InputFrame &input_frame : video_frames) {
            const FrameInfo &frame_info = frames_map.at(input_frame);
            frames_and_scores.emplace_back(std::make_pair(input_frame, frame_info.alignment_result->get_ranking_score()));
        }

        // sort frames by score
        std::sort(frames_and_scores.begin(), frames_and_scores.end(), [](const std::pair<InputFrame, float> &a, const std::pair<InputFrame, float> &b) {
            return a.second < b.second;
        });

        int n_frames_to_keep = number_of_frames_to_keep;
        if (fraction_to_keep > 0.0f) {
            n_frames_to_keep = static_cast<int>(static_cast<float>(n_frames_total) * fraction_to_keep + 0.5f);
        }
        n_frames_to_keep = std::min(n_frames_to_keep, n_frames_total);

        cout << "Total frames in input video: " << n_frames_total << endl;
        cout << "Frames to keep in output video: " << n_frames_to_keep << endl;

        InputFrameReader input_frame_reader(video_frames[0]);
        const Metadata first_frame_metadata = frames_map.at(video_frames[0]).metadata;

        cout << "Camera model: " << first_frame_metadata.camera_model << endl;
        cout << "shutting speed: " << first_frame_metadata.exposure_time << " s" << endl;
        cout << "Aperture: f/" << first_frame_metadata.aperture << endl;
        cout << "ISO: " << first_frame_metadata.iso << endl;
        cout << "Video FPS: " << first_frame_metadata.video_fps << endl;
        cout << "Temperature: " << first_frame_metadata.temperature << " C" << endl;

        if (!input_frame_reader.is_raw_file()) {
            throw runtime_error("Input video frames are not raw files. This program only supports raw video frames.");
        }

        int width, height;
        input_frame_reader.get_photo_resolution(&width, &height);
        const float fps = get_fps_of_video(input_file_address);

        VideoWriterSER video_writer(output_file_address, first_frame_metadata, width, height, fps, output_bit_depth);

        for (int i_frame = 0; i_frame < n_frames_to_keep; i_frame++) {
            const InputFrame &input_frame = frames_and_scores[i_frame].first;
            InputFrameReader frame_reader(input_frame);
            std::vector<PixelType> frame_data = frame_reader.get_raw_data();

            if (output_bit_depth == 8) {
                std::vector<unsigned char> frame_data_8bit(frame_data.size(), 0);
                std::transform(frame_data.begin(), frame_data.end(), frame_data_8bit.begin(), [](PixelType x) -> unsigned char { return static_cast<unsigned char>(x / 256); });
                video_writer.write_frame(frame_data_8bit);
            }
            else if (output_bit_depth == 16) {
                std::vector<unsigned short> frame_data_16bit(frame_data.size(), 0);
                std::transform(frame_data.begin(), frame_data.end(), frame_data_16bit.begin(), [](PixelType x) -> unsigned short { return static_cast<unsigned short>(x); });
                video_writer.write_frame(frame_data_16bit);
            }
            else {
                throw runtime_error("Unsupported output bit depth: " + to_string(output_bit_depth));
            }

            cout << "Added frame " << i_frame + 1 << " / " << n_frames_to_keep << "\r";
        }

    }
    catch (const exception &e) {
        cout << e.what() << endl;
        abort();
    }

    return 0;
}