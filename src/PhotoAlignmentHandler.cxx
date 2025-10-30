#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/ReferencePhotoHandlerStars.h"
#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"
#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/ReferencePhotoHandlerComet.h"
#include "../headers/Common.h"
#include "../headers/PhotoRanker.h"
#include "../headers/SharpnessRanker.h"
#include "../headers/thread_pool.h"
#include "../headers/VideoReader.h"

#include "../headers/AlignmentResultFactory.h"

#include <fstream>

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


        FileAlignmentInformation alignment_info;
        alignment_info.input_frame = InputFrame(frame_info_vector[0], stoi(frame_info_vector[1]));
        alignment_info.alignment_result = std::move(alignment_result);

        m_alignment_information_vector.push_back(std::move(alignment_info));

    }
    alignment_file.close();
}

void PhotoAlignmentHandler::add_alignment_info(const InputFrame &input_frame, const AlignmentResultBase &alignment_result) {
    FileAlignmentInformation alignment_info;
    alignment_info.input_frame = input_frame;
    alignment_info.alignment_result = alignment_result.clone();

    m_alignment_information_vector.push_back(alignment_info);
};

void PhotoAlignmentHandler::save_to_text_file(const std::string &alignment_file_address)   {
    sort(m_alignment_information_vector.begin(), m_alignment_information_vector.end(), [](const FileAlignmentInformation &a, const FileAlignmentInformation &b) {
        return a.alignment_result->get_ranking_score() < b.alignment_result->get_ranking_score();
    });
    ofstream alignment_file(alignment_file_address);
    alignment_file << c_reference_file_header << c_separator_in_file << m_reference_frame.to_string() << endl;
    alignment_file << "# File address | frame_number | alignment_info" << endl;
    for (const FileAlignmentInformation &alignment_info : m_alignment_information_vector) {
        if (alignment_info.input_frame.get_file_address() == "")  {   // plate-solving failed
            continue;
        }
        alignment_file  <<          alignment_info.input_frame.get_file_address()
                        << c_separator_in_file << alignment_info.input_frame.get_frame_number()
                        << c_separator_in_file << alignment_info.alignment_result->get_description_string() << endl;
    }
    alignment_file.close();
};

void PhotoAlignmentHandler::align_files(const InputFrame &reference_frame, const std::vector<InputFrame> &files) {
    m_reference_photo_handler = reference_photo_handler_factory(reference_frame);
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

    const unsigned int n_files = files.size();
    m_alignment_information_vector.resize(n_files);
    auto align_file_multicore = [this](const InputFrame &input_frame, unsigned int file_index) {
        unique_ptr<AlignmentResultBase> alignment_result = m_reference_photo_handler->calculate_alignment(input_frame);
        if (alignment_result->is_valid()) {
            FileAlignmentInformation &alignment_info = m_alignment_information_vector[file_index];
            alignment_info.input_frame          = input_frame;
            alignment_info.alignment_result     = std::move(alignment_result);
        }
        else {
            cout << "Alignment calculation failed for frame: " + input_frame.to_string() + "\n";
        }
        m_n_files_aligned += 1;
    };

    m_n_files_aligned = 0;
    thread_pool pool(m_n_cpu);
    for (unsigned int i_file = 0; i_file < files.size(); i_file++)   {
        if (m_n_cpu == 1)   {
            align_file_multicore(files[i_file], i_file);
        }
        else    {
            pool.submit(align_file_multicore, files[i_file], i_file);
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
    m_alignment_information_vector.clear();
    m_reference_photo_handler = nullptr;
}

FileAlignmentInformation PhotoAlignmentHandler::get_alignment_parameters(const InputFrame &input_frame) const    {
    // #TODO: Use map
    for (const FileAlignmentInformation &alignment_info :  m_alignment_information_vector) {
        if (alignment_info.input_frame == input_frame) {
            return alignment_info;
        }
    }
    throw runtime_error("Frame not found in alignment file: " + input_frame.to_string());
}

const std::vector<FileAlignmentInformation>& PhotoAlignmentHandler::get_alignment_parameters_vector() const    {
    return PhotoAlignmentHandler::m_alignment_information_vector;
};

std::vector<std::string> PhotoAlignmentHandler::get_file_addresses() const  {
    vector<string> file_addresses;
    for (const FileAlignmentInformation &alignment_info : m_alignment_information_vector) {
        file_addresses.push_back(alignment_info.input_frame.get_file_address());
    }
    return file_addresses;
};


void PhotoAlignmentHandler::limit_number_of_files(int n_files) {
    if (n_files < 0) {
        return;
    }
    if (n_files > int(m_alignment_information_vector.size())) {
        return;
    }

    sort(m_alignment_information_vector.begin(), m_alignment_information_vector.end(), [](const FileAlignmentInformation &a, const FileAlignmentInformation &b) {
        return a.alignment_result->get_ranking_score() < b.alignment_result->get_ranking_score();
    });
    m_alignment_information_vector.resize(n_files);
};

void PhotoAlignmentHandler::limit_fraction_of_files(float fraction) {
    if (fraction < 0.0 || fraction >= 1.0) {
        return;
    }
    const int n_files = m_alignment_information_vector.size();
    const int n_files_to_keep = n_files*fraction;
    limit_number_of_files(n_files_to_keep);
};

const std::atomic<int>& PhotoAlignmentHandler::get_number_of_aligned_files() const {
    return m_n_files_aligned;
};

unique_ptr<ReferencePhotoHandlerBase> PhotoAlignmentHandler::reference_photo_handler_factory(const InputFrame& input_frame)   const  {
    if (m_alignment_method == "stars") {
        return make_unique<ReferencePhotoHandlerStars>(input_frame);
    }
    else if (m_alignment_method == "planetary") {
        return make_unique<ReferencePhotoHandlerPlanetary>(input_frame, 0.05);
    }
    else if (m_alignment_method == "planetary without rotation")    {
        return make_unique<ReferencePhotoHandlerPlanetaryZeroRotation>(input_frame, 0.05);
    }
    else if (m_alignment_method == "surface") {
        return make_unique<ReferencePhotoHandlerSurface>(input_frame, 0.05);
    }
    else if (m_alignment_method == "comet") {
        return  make_unique<ReferencePhotoHandlerComet>(input_frame);
    }
    else {
        throw runtime_error("Invalid alignment method: " + m_alignment_method);
    }
};
