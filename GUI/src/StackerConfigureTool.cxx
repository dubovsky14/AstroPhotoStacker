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
    FilelistHandler filelist_handler_only_checked = filelist_handler.get_filelist_with_checked_files();
    int width, height;
    get_photo_resolution(filelist_handler_only_checked.get_files(FileTypes::LIGHT)[0], &width, &height);
    std::unique_ptr<AstroPhotoStacker::StackerBase> stacker = AstroPhotoStacker::create_stacker(
        get_stacker_type_for_factory(stack_settings.get_stacking_algorithm()),
        3,
        width,
        height,
        stack_settings.use_color_interpolation()
    );
    configure_stacker(stacker.get(), stack_settings);
    const vector<string>            &light_frames       = filelist_handler_only_checked.get_files(FileTypes::LIGHT);
    const vector<AlignmentFileInfo> &alignment_info_vec = filelist_handler_only_checked.get_alignment_info(FileTypes::LIGHT);
    for (unsigned int i = 0; i < light_frames.size(); ++i) {
        const std::string &file = light_frames[i];
        const AlignmentFileInfo &alignment_info = alignment_info_vec[i];
        stacker->add_photo(file);
        stacker->add_alignment_info(    file,
                                        alignment_info.shift_x,
                                        alignment_info.shift_y,
                                        alignment_info.rotation_center_x,
                                        alignment_info.rotation_center_y,
                                        alignment_info.rotation,
                                        alignment_info.ranking);
    }

    const vector<string> &dark_frames = filelist_handler_only_checked.get_files(FileTypes::DARK);
    if (dark_frames.size() > 0) {
        std::shared_ptr<const CalibrationFrameBase> dark_frames_handler = std::make_shared<DarkFrameHandler>(dark_frames[0]);
        cout << "Adding dark frame: " << dark_frames[0] << endl;
        stacker->add_calibration_frame_handler(dark_frames_handler);
    }

    const vector<string> &flat_frames = filelist_handler_only_checked.get_files(FileTypes::FLAT);
    if (flat_frames.size() > 0) {
        std::shared_ptr<const CalibrationFrameBase> flat_frames_handler = std::make_shared<FlatFrameHandler>(flat_frames[0]);
        cout << "Adding flat frame: " << flat_frames[0] << endl;
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