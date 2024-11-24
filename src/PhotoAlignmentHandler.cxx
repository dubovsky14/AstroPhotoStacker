#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/ReferencePhotoHandlerStars.h"
#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"
#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/Common.h"
#include "../headers/PhotoRanker.h"
#include "../headers/SharpnessRanker.h"
#include "../headers/thread_pool.h"
#include "../headers/VideoReader.h"

#include <fstream>

using namespace std;
using namespace AstroPhotoStacker;

void PhotoAlignmentHandler::read_from_text_file(const std::string &alignment_file_address) {
    ifstream alignment_file(alignment_file_address);
    if (!alignment_file.is_open()) {
        throw runtime_error("Could not open alignment file: " + alignment_file_address);
    }
    string line;
    while (getline(alignment_file, line)) {
        strip_string(&line);
        if (line.empty()) continue;
        if (line[0] == '#') continue; // Ignore comments (lines starting with #)

        vector<string> elements = split_and_strip_string(line, "|");
        if (starts_with(line, c_reference_file_header)) {
            if (elements.size() != 3) {
                throw runtime_error("Invalid reference file header.");
            }
            if (!string_is_int(elements[2])) {
                throw runtime_error("Invalid reference file header.");
            }
            m_reference_frame = InputFrame(elements[1], stoi(elements[2]));
            continue;
        }

        if (elements.size() != 8) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        if (!string_is_float(elements[1]) || // frame number
            !string_is_int  (elements[2]) || // shift_x
            !string_is_float(elements[3]) || // shift_y
            !string_is_float(elements[4]) || // rotation_center_x
            !string_is_float(elements[5]) || // rotation_center_y
            !string_is_float(elements[6]) || // rotation
            !string_is_float(elements[7])    // ranking
            ) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        FileAlignmentInformation alignment_info;
        alignment_info.input_frame = InputFrame(elements[0], stoi(elements[1]));
        alignment_info.shift_x = stof(elements[2]);
        alignment_info.shift_y = stof(elements[3]);
        alignment_info.rotation_center_x = stof(elements[4]);
        alignment_info.rotation_center_y = stof(elements[5]);
        alignment_info.rotation = stof(elements[6]);
        alignment_info.ranking = stof(elements[7]);

        m_alignment_information_vector.push_back(alignment_info);

    }
    alignment_file.close();
}

void PhotoAlignmentHandler::add_alignment_info(const InputFrame &input_frame, float x_shift, float y_shift, float rotation_center_x, float rotation_center_y, float rotation, float ranking, const LocalShiftsHandler &local_shifts_handler) {
    FileAlignmentInformation alignment_info;
    alignment_info.input_frame = input_frame;
    alignment_info.shift_x = x_shift;
    alignment_info.shift_y = y_shift;
    alignment_info.rotation_center_x = rotation_center_x;
    alignment_info.rotation_center_y = rotation_center_y;
    alignment_info.rotation = rotation;
    alignment_info.ranking = ranking;
    alignment_info.local_shifts_handler = local_shifts_handler;

    m_alignment_information_vector.push_back(alignment_info);
};

void PhotoAlignmentHandler::save_to_text_file(const std::string &alignment_file_address)   {
    sort(m_alignment_information_vector.begin(), m_alignment_information_vector.end(), [](const FileAlignmentInformation &a, const FileAlignmentInformation &b) {
        return a.ranking < b.ranking;
    });
    ofstream alignment_file(alignment_file_address);
    alignment_file << c_reference_file_header << " | " << m_reference_frame.to_string() << endl;
    alignment_file << "# File address | frame_number | shift_x | shift_y | rotation_center_x | rotation_center_y | rotation | ranking" << endl;
    for (const FileAlignmentInformation &alignment_info : m_alignment_information_vector) {
        if (alignment_info.input_frame.get_file_address() == "")  {   // plate-solving failed
            continue;
        }
        alignment_file  <<          alignment_info.input_frame.get_file_address()
                        << " | " << alignment_info.input_frame.get_frame_number()
                        << " | " << alignment_info.shift_x
                        << " | " << alignment_info.shift_y
                        << " | " << alignment_info.rotation_center_x
                        << " | " << alignment_info.rotation_center_y
                        << " | " << alignment_info.rotation
                        << " | " << alignment_info.ranking << endl;
    }
    alignment_file.close();
};

void PhotoAlignmentHandler::align_files(const InputFrame &reference_frame, const std::vector<InputFrame> &files) {
    m_reference_photo_handler = reference_photo_handler_factory(reference_frame);
    m_reference_frame = reference_frame;

    const unsigned int n_files = files.size();
    m_alignment_information_vector.resize(n_files);
    m_local_shifts_vector.resize(n_files);
    auto align_file_multicore = [this](const InputFrame &input_frame, unsigned int file_index) {
        float shift_x, shift_y, rot_center_x, rot_center_y, rotation, ranking;
        if (m_reference_photo_handler->calculate_alignment(input_frame, &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation, &ranking)) {
            FileAlignmentInformation &alignment_info = m_alignment_information_vector[file_index];
            alignment_info.input_frame = input_frame;
            alignment_info.shift_x = shift_x;
            alignment_info.shift_y = shift_y;
            alignment_info.rotation_center_x = rot_center_x;
            alignment_info.rotation_center_y = rot_center_y;
            alignment_info.rotation = rotation;
            alignment_info.ranking = ranking;

            const ReferencePhotoHandlerSurface *surface_handler = dynamic_cast<const ReferencePhotoHandlerSurface*>(m_reference_photo_handler.get());
            if (surface_handler != nullptr) {
                vector<LocalShift> local_shifts = surface_handler->get_local_shifts(input_frame, shift_x, shift_y, rot_center_x, rot_center_y, rotation);
                m_local_shifts_vector[file_index] = local_shifts;
                alignment_info.local_shifts_handler = LocalShiftsHandler(local_shifts);
                if (m_alignment_box_vector_storage) {
                    *m_alignment_box_vector_storage = surface_handler->get_alignment_boxes();
                }
            }
        }
        else {
            cout << "Plate solving failed for frame: " + input_frame.to_string() + "\n";
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
        return a.ranking < b.ranking;
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

std::vector<LocalShift> PhotoAlignmentHandler::get_local_shifts(const InputFrame& input_frame) const   {
    for (unsigned int i_file = 0; i_file < m_alignment_information_vector.size(); i_file++) {
        if (m_alignment_information_vector[i_file].input_frame == input_frame) {
            return m_local_shifts_vector[i_file];
        }
    }
    throw runtime_error("Frame not found in alignment file: "s + input_frame.to_string());
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
    else {
        throw runtime_error("Invalid alignment method: " + m_alignment_method);
    }
};
