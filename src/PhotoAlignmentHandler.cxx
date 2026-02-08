#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/ReferencePhotoHandlerComet.h"
#include "../headers/Common.h"
#include "../headers/PhotoRanker.h"
#include "../headers/VideoReader.h"
#include "../headers/TaskScheduler.hxx"

#include "../headers/AlignmentResultFactory.h"
#include "../headers/ReferencePhotoHandlerFactory.h"

#include <fstream>
#include <filesystem>

using namespace std;
using namespace AstroPhotoStacker;

void PhotoAlignmentHandler::read_from_text_file(const std::string &alignment_file_address) {
    ifstream alignment_file(alignment_file_address);
    if (!alignment_file.is_open()) {
        throw runtime_error("Could not open alignment file: " + alignment_file_address);
    }
    string line;

    const AlignmentResultFactory &alignment_result_factory = AlignmentResultFactory::get_instance();
    while (getline(alignment_file, line)) {
        strip_string(&line);
        if (line.empty()) continue;
        if (line[0] == '#') continue; // Ignore comments (lines starting with #)

        if (starts_with(line, c_reference_file_header)) {
            const vector<string> elements = split_and_strip_string(line, c_separator_in_file);
            if (elements.size() != 3) {
                throw runtime_error("Invalid reference file header.");
            }
            if (!string_is_int(elements[2])) {
                throw runtime_error("Invalid reference file header.");
            }
            m_reference_frame = InputFrame(elements[1], stoi(elements[2]));
            continue;
        }


        const int end_of_frame_info_position = find_nth_occurrence(line, c_separator_in_file, 2);
        if (end_of_frame_info_position == -1) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        const std::string frame_info = line.substr(0, end_of_frame_info_position);
        const vector<string> frame_info_vector = split_and_strip_string(frame_info, c_separator_in_file);

        if (!string_is_float(frame_info_vector[1])) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        unique_ptr<AlignmentResultBase> alignment_result = alignment_result_factory.create_alignment_result_from_description_string(line.substr(end_of_frame_info_position + c_separator_in_file.length()));
        const InputFrame input_frame = InputFrame(frame_info_vector[0], stoi(frame_info_vector[1]));

        m_alignment_results_map[input_frame] = std::move(alignment_result);

    }
    alignment_file.close();
}

void PhotoAlignmentHandler::add_alignment_info(const InputFrame &input_frame, const AlignmentResultBase &alignment_result) {
    m_alignment_results_map[input_frame] = alignment_result.clone();
};

void PhotoAlignmentHandler::save_to_text_file(const std::string &alignment_file_address)   {
    vector<tuple<InputFrame, std::unique_ptr<AlignmentResultBase>>> sorted_frames_alignment_vector;
    for (const auto &entry : m_alignment_results_map) {
        sorted_frames_alignment_vector.push_back(std::make_tuple(entry.first, entry.second->clone()));
    }

    sort(sorted_frames_alignment_vector.begin(), sorted_frames_alignment_vector.end(), [](const tuple<InputFrame, std::unique_ptr<AlignmentResultBase>> &a, const tuple<InputFrame, std::unique_ptr<AlignmentResultBase>> &b) {
        return get<1>(a)->get_ranking_score() < get<1>(b)->get_ranking_score();
    });
    ofstream alignment_file(alignment_file_address);
    alignment_file << c_reference_file_header << c_separator_in_file << m_reference_frame.get_file_address() << c_separator_in_file << m_reference_frame.get_frame_number() << endl;
    alignment_file << "# File address | frame_number | alignment_info" << endl;
    for (const tuple<InputFrame, std::unique_ptr<AlignmentResultBase>> &alignment_info : sorted_frames_alignment_vector) {
        if (std::get<0>(alignment_info).get_file_address() == "")  {   // plate-solving failed
            continue;
        }
        alignment_file  << std::get<0>(alignment_info).get_file_address()
                        << c_separator_in_file << std::get<0>(alignment_info).get_frame_number()
                        << c_separator_in_file << std::get<1>(alignment_info)->get_description_string() << endl;
    }
    alignment_file.close();
};

void PhotoAlignmentHandler::align_files(const InputFrame &reference_frame, const std::vector<InputFrame> &files) {
    m_reference_photo_handler = ReferencePhotoHandlerFactory::get_reference_photo_handler(reference_frame, m_alignment_method, m_configurable_algorithm_settings_map);
    m_reference_frame = reference_frame;

    ReferencePhotoHandlerComet *comet_handler = dynamic_cast<ReferencePhotoHandlerComet*>(m_reference_photo_handler.get());
    if (comet_handler != nullptr) {
        for (const std::pair<const InputFrame, std::pair<float, float>> &comet_position_entry : m_comet_positions) {
            comet_handler->add_comet_position(comet_position_entry.first, comet_position_entry.second.first, comet_position_entry.second.second);
        }
        const bool fit_success = comet_handler->fit_comet_path();
        if (!fit_success) {
            cout << "Warning: Comet path fitting failed.\n";
        }
    }

    std::mutex alignment_map_mutex;
    auto align_file_multicore = [this, &alignment_map_mutex](const InputFrame &input_frame) {
        unique_ptr<AlignmentResultBase> alignment_result = m_reference_photo_handler->calculate_alignment(input_frame);
        if (!alignment_result->is_valid()) {
            cout << "Alignment calculation failed for frame: " + input_frame.to_string() + "\n";
        }
        std::lock_guard<std::mutex> lock(alignment_map_mutex);
        m_alignment_results_map[input_frame] = std::move(alignment_result);
        m_n_files_aligned += 1;
    };

    std::map<size_t, InputFrame> task_id_to_input_frame_map;
    auto exception_handler = [task_id_to_input_frame_map](size_t task_id, const std::exception &e) {
        try {
            throw e;
        }
        catch (const std::exception &e) {
            std::throw_with_nested(runtime_error("Error while aligning file: " + task_id_to_input_frame_map.at(task_id).to_string() + ". " + e.what()));
        }
    };

    m_n_files_aligned = 0;
    TaskScheduler pool({size_t(m_n_cpu)});
    for (unsigned int i_file = 0; i_file < files.size(); i_file++)   {
        if (m_n_cpu == 1)   {
            align_file_multicore(files[i_file]);
        }
        else    {
            pool.submit(align_file_multicore, {1}, files[i_file]);
        }
    }
    pool.wait_for_tasks();
};

void PhotoAlignmentHandler::align_all_files_in_folder(const InputFrame &reference_frame, const std::string &raw_files_folder) {
    const vector<string> files = get_frame_files_in_folder(raw_files_folder);
    vector<InputFrame> input_frames;
    for (const string &file : files) {
        if (is_valid_video_file(file)) {
            vector<InputFrame> video_frames = get_video_frames(file);
            input_frames.insert(input_frames.end(), video_frames.begin(), video_frames.end());
            cout << "Video file: " << file << " - " << video_frames.size() << " frames\n";
        }
        else {
            input_frames.push_back(InputFrame(file));
        }
    }
    align_files(reference_frame, input_frames);
}

void PhotoAlignmentHandler::reset() {
    m_alignment_results_map.clear();
    m_reference_photo_handler = nullptr;
}

std::unique_ptr<AlignmentResultBase> PhotoAlignmentHandler::get_alignment_parameters(const InputFrame &input_frame) const    {
    auto it = m_alignment_results_map.find(input_frame);
    if (it != m_alignment_results_map.end()) {
        return it->second->clone();
    }
    throw runtime_error("Frame not found in alignment file: " + input_frame.to_string());
}

const std::map<InputFrame, std::unique_ptr<AlignmentResultBase>>& PhotoAlignmentHandler::get_alignment_results_map() const {
    return m_alignment_results_map;
};

std::vector<std::string> PhotoAlignmentHandler::get_file_addresses() const  {
    vector<string> file_addresses;
    for (const std::pair<const InputFrame, std::unique_ptr<AlignmentResultBase>> &alignment_info : m_alignment_results_map) {
        file_addresses.push_back(alignment_info.first.get_file_address());
    }
    return file_addresses;
};


void PhotoAlignmentHandler::limit_number_of_files(int n_files) {
    if (n_files < 0) {
        return;
    }
    if (n_files > int(m_alignment_results_map.size())) {
        return;
    }

    vector<tuple<InputFrame, std::unique_ptr<AlignmentResultBase>>> sorted_frames_alignment_vector;
    for (const auto &entry : m_alignment_results_map) {
        sorted_frames_alignment_vector.push_back(std::make_tuple(entry.first, entry.second->clone()));
    }

    sort(sorted_frames_alignment_vector.begin(), sorted_frames_alignment_vector.end(), [](const tuple<InputFrame, std::unique_ptr<AlignmentResultBase>> &a, const tuple<InputFrame, std::unique_ptr<AlignmentResultBase>> &b) {
        return get<1>(a)->get_ranking_score() < get<1>(b)->get_ranking_score();
    });

    m_alignment_results_map.clear();
    for (int i = 0; i < n_files; i++) {
        auto &entry = sorted_frames_alignment_vector[i];
        m_alignment_results_map[std::get<0>(entry)] = std::move(std::get<1>(entry));
    }
};

void PhotoAlignmentHandler::limit_fraction_of_files(float fraction) {
    if (fraction < 0.0 || fraction >= 1.0) {
        return;
    }
    const int n_files = m_alignment_results_map.size();
    const int n_files_to_keep = n_files*fraction;
    limit_number_of_files(n_files_to_keep);
};

const std::atomic<int>& PhotoAlignmentHandler::get_number_of_aligned_files() const {
    return m_n_files_aligned;
};

void PhotoAlignmentHandler::set_alignment_method(const std::string& alignment_method, const ConfigurableAlgorithmSettingsMap& configurable_algorithm_settings_map) {
    m_alignment_method = alignment_method;
    m_configurable_algorithm_settings_map = configurable_algorithm_settings_map;
};