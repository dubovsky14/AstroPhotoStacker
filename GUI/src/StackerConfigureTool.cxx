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
    std::unique_ptr<AstroPhotoStacker::StackerBase> stacker = AstroPhotoStacker::create_stacker(
        get_stacker_type_for_factory(stack_settings.get_stacking_algorithm()),
        3,
        width,
        height,
        stack_settings.use_color_interpolation()
    );
    configure_stacker(stacker.get(), stack_settings);
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
        std::shared_ptr<const CalibrationFrameBase> dark_frames_handler = std::make_shared<DarkFrameHandler>(dark_frame.get_file_address());
        cout << "Adding dark frame: " << dark_frame.to_string() << endl;
        stacker->add_calibration_frame_handler(dark_frames_handler);
    }

    const vector<InputFrame> &flat_frames = filelist_handler_only_checked.get_frames(FileTypes::FLAT);
    if (flat_frames.size() > 0) {
        const InputFrame &flat_frame = flat_frames[0];
        if (!flat_frame.is_still_image()) {
            throw std::runtime_error("Flat frame must be a still image");
        }
        std::shared_ptr<const CalibrationFrameBase> flat_frames_handler = std::make_shared<FlatFrameHandler>(flat_frame.get_file_address());
        cout << "Adding flat frame: " << flat_frame.to_string() << endl;
        stacker->add_calibration_frame_handler(flat_frames_handler);
    }


    return stacker;
};


std::string get_stacker_type_for_factory(const std::string &stacking_algorithm_app_name)    {
    if (stacking_algorithm_app_name == "kappa-sigma mean") {
        return "kappa_sigma_clipping";
    } else if (stacking_algorithm_app_name == "kappa-sigma median") {
        return "kappa_sigma_median";
    } else if (stacking_algorithm_app_name == "average") {
        return "mean";
    }
    else if (stacking_algorithm_app_name == "median") {
        return "median";
    }
    else if (stacking_algorithm_app_name == "cut-off average") {
        return "cut_off_average";
    }
    else if (stacking_algorithm_app_name == "maximum") {
        return "maximum";
    }
    else if (stacking_algorithm_app_name == "minimum") {
        return "minimum";
    }
    else if (stacking_algorithm_app_name == "best score") {
        return "best_score";
    }
    else {
        throw std::runtime_error("Unknown stacker type: " + stacking_algorithm_app_name);
    }
};


void configure_stacker(StackerBase* stacker, const StackSettings &settings)   {

    // kappa-sigma
    StackerKappaSigmaBase *stacker_kappa_sigma = dynamic_cast<StackerKappaSigmaBase*>(stacker);
    if (stacker_kappa_sigma != nullptr) {
        stacker_kappa_sigma->set_kappa(settings.get_kappa());
        stacker_kappa_sigma->set_number_of_iterations(settings.get_kappa_sigma_iter());
        cout << "Kappa: " << settings.get_kappa() << endl;
        cout << "Number of iterations: " << settings.get_kappa_sigma_iter() << endl;
    }

    stacker->set_number_of_cpu_threads(settings.get_n_cpus());

    // Memory limit
    if (dynamic_cast<StackerMeanValue*>(stacker) == nullptr) {
        stacker->set_memory_usage_limit(settings.get_max_memory());
    }

    // cut-off average
    if (dynamic_cast<StackerCutOffAverage*>(stacker) != nullptr) {
        StackerCutOffAverage *stacker_cut_off_average = dynamic_cast<StackerCutOffAverage*>(stacker);
        stacker_cut_off_average->set_tail_fraction_to_cut_off(settings.get_cut_off_tail_fraction());
    }
};