#include "../headers/StackerConfigureTool.h"

#include "../../headers/ImageFilesInputOutput.h"
#include "../../headers/StackerKappaSigmaBase.h"
#include "../../headers/StackerMeanValue.h"

#include "../../headers/FlatFrameHandler.h"
#include "../../headers/DarkFrameHandler.h"
#include "../../headers/InputFrameReader.h"

#include <memory>

using namespace std;
using namespace AstroPhotoStacker;

std::unique_ptr<AstroPhotoStacker::StackerBase> get_configured_stacker(const StackSettings& stack_settings, const FilelistHandlerGUIInterface& filelist_handler)    {
    FilelistHandlerGUIInterface filelist_handler_only_checked = filelist_handler.get_filelist_with_checked_frames();
    std::unique_ptr<AstroPhotoStacker::StackerBase> stacker = nullptr;

    vector<int> group_numbers = filelist_handler_only_checked.get_group_numbers();
    for (int group_number : group_numbers) {
        const std::map<InputFrame, FrameInfo> &light_frames = filelist_handler_only_checked.get_frames(FrameType::LIGHT, group_number);
        if (light_frames.size() == 0) {
            continue;
        }

        if (stacker == nullptr) {
            int width, height;
            AstroPhotoStacker::InputFrameReader input_frame_reader(light_frames.begin()->first);
            input_frame_reader.load_input_frame_data();
            input_frame_reader.get_photo_resolution(&width, &height);
            cout << "Resolution: " << width << "x" << height << endl;
            stacker = AstroPhotoStacker::create_stacker(stack_settings, 3, width, height);
        }

        vector<shared_ptr<const CalibrationFrameBase>> calibration_frames_handlers;

        const std::map<InputFrame, FrameInfo> &dark_frames = filelist_handler_only_checked.get_frames(FrameType::DARK, group_number);
        if (dark_frames.size() > 0) {
            const InputFrame &dark_frame = dark_frames.begin()->first;
            if (!dark_frame.is_still_image()) {
                throw std::runtime_error("Dark frame must be a still image");
            }
            std::shared_ptr<const CalibrationFrameBase> dark_frames_handler = std::make_shared<DarkFrameHandler>(dark_frame);
            calibration_frames_handlers.push_back(dark_frames_handler);
            cout << "Adding dark frame: " << dark_frame.to_string() << endl;
        }

        const std::map<InputFrame, FrameInfo> &flat_frames = filelist_handler_only_checked.get_frames(FrameType::FLAT, group_number);
        if (flat_frames.size() > 0) {
            const InputFrame &flat_frame = flat_frames.begin()->first;
            if (!flat_frame.is_still_image()) {
                throw std::runtime_error("Flat frame must be a still image");
            }
            std::shared_ptr<const CalibrationFrameBase> flat_frames_handler = std::make_shared<FlatFrameHandler>(flat_frame);
            calibration_frames_handlers.push_back(flat_frames_handler);
            cout << "Adding flat frame: " << flat_frame.to_string() << endl;
        }

        for (const auto &light_frame : light_frames) {
            const InputFrame &input_frame = light_frame.first;
            const AlignmentFileInfo &alignment_info = light_frame.second.alignment_info;
            stacker->add_photo(input_frame, calibration_frames_handlers);
            stacker->add_alignment_info(    input_frame,
                                            alignment_info.shift_x,
                                            alignment_info.shift_y,
                                            alignment_info.rotation_center_x,
                                            alignment_info.rotation_center_y,
                                            alignment_info.rotation,
                                            alignment_info.ranking,
                                            alignment_info.local_shifts_handler);

        }
    }
    return stacker;
};
