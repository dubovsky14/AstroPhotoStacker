#include "../headers/StackerConfigureTool.h"

#include "../../headers/ImageFilesInputOutput.h"
#include "../../headers/StackerKappaSigmaBase.h"
#include "../../headers/StackerMeanValue.h"

#include "../../headers/FlatFrameHandler.h"
#include "../../headers/DarkFrameHandler.h"

#include <memory>

using namespace std;
using namespace AstroPhotoStacker;

std::unique_ptr<AstroPhotoStacker::StackerBase> get_configured_stacker(const StackSettings& stack_settings, const FilelistHandler& filelist_handler)    {
    FilelistHandler filelist_handler_only_checked = filelist_handler.get_filelist_with_checked_frames();
    int width, height;
    get_photo_resolution(filelist_handler_only_checked.get_frames(FileTypes::LIGHT)[0], &width, &height);
    cout << "Resolution: " << width << "x" << height << endl;
    std::unique_ptr<AstroPhotoStacker::StackerBase> stacker = AstroPhotoStacker::create_stacker(stack_settings, 3, width, height);

    const vector<InputFrame>        &light_frames       = filelist_handler_only_checked.get_frames(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler_only_checked.get_alignment_info();
    for (unsigned int i = 0; i < light_frames.size(); ++i) {
        const InputFrame &frame = light_frames[i];
        const AlignmentFileInfo &alignment_info = alignment_info_vec[i];
        stacker->add_photo(frame);
        stacker->add_alignment_info(    frame,
                                        alignment_info.shift_x,
                                        alignment_info.shift_y,
                                        alignment_info.rotation_center_x,
                                        alignment_info.rotation_center_y,
                                        alignment_info.rotation,
                                        alignment_info.ranking,
                                        alignment_info.local_shifts_handler);
    }
    const vector<InputFrame> &dark_frames = filelist_handler_only_checked.get_frames(FileTypes::DARK);
    if (dark_frames.size() > 0) {
        const InputFrame &dark_frame = dark_frames[0];
        if (!dark_frame.is_still_image()) {
            throw std::runtime_error("Dark frame must be a still image");
        }
        std::shared_ptr<const CalibrationFrameBase> dark_frames_handler = std::make_shared<DarkFrameHandler>(dark_frame);
        cout << "Adding dark frame: " << dark_frame.to_string() << endl;
        stacker->add_calibration_frame_handler(dark_frames_handler);
    }

    const vector<InputFrame> &flat_frames = filelist_handler_only_checked.get_frames(FileTypes::FLAT);
    if (flat_frames.size() > 0) {
        const InputFrame &flat_frame = flat_frames[0];
        if (!flat_frame.is_still_image()) {
            throw std::runtime_error("Flat frame must be a still image");
        }
        std::shared_ptr<const CalibrationFrameBase> flat_frames_handler = std::make_shared<FlatFrameHandler>(flat_frame);
        cout << "Adding flat frame: " << flat_frame.to_string() << endl;
        stacker->add_calibration_frame_handler(flat_frames_handler);
    }


    return stacker;
};
