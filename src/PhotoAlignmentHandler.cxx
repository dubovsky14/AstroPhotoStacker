#include "../headers/PhotoAlignmentHandler.h"
#include "../headers/ReferencePhotoHandlerStars.h"
#include "../headers/ReferencePhotoHandlerPlanetary.h"
#include "../headers/ReferencePhotoHandlerPlanetaryZeroRotation.h"
#include "../headers/ReferencePhotoHandlerSurface.h"
#include "../headers/Common.h"
#include "../headers/PhotoRanker.h"
#include "../headers/SharpnessRanker.h"
#include "../headers/thread_pool.h"

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
            if (elements.size() != 2) {
                throw runtime_error("Invalid reference file header.");
            }
            m_reference_file_address = elements[1];
            continue;
        }

        if (elements.size() != 7) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        if (!string_is_float(elements[1]) || !string_is_float(elements[2]) || !string_is_float(elements[3]) || !string_is_float(elements[4]) || !string_is_float(elements[5])) {
            throw runtime_error("Invalid alignment file. Could not read line: " + line);
        }

        FileAlignmentInformation alignment_info;
        alignment_info.file_address = elements[0];
        alignment_info.shift_x = stof(elements[1]);
        alignment_info.shift_y = stof(elements[2]);
        alignment_info.rotation_center_x = stof(elements[3]);
        alignment_info.rotation_center_y = stof(elements[4]);
        alignment_info.rotation = stof(elements[5]);
        alignment_info.ranking = stof(elements[6]);

        m_alignment_information_vector.push_back(alignment_info);

    }
    alignment_file.close();
}

void PhotoAlignmentHandler::add_alignment_info(const std::string &file_address, float x_shift, float y_shift, float rotation_center_x, float rotation_center_y, float rotation, float ranking, const LocalShiftsHandler &local_shifts_handler) {
    FileAlignmentInformation alignment_info;
    alignment_info.file_address = file_address;
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
    alignment_file << c_reference_file_header << " | " << m_reference_file_address << endl;
    alignment_file << "# File address | shift_x | shift_y | rotation_center_x | rotation_center_y | rotation | ranking" << endl;
    for (const FileAlignmentInformation &alignment_info : m_alignment_information_vector) {
        if (alignment_info.file_address == "")  {   // plate-solving failed
            continue;
        }
        alignment_file  <<          alignment_info.file_address
                        << " | " << alignment_info.shift_x
                        << " | " << alignment_info.shift_y
                        << " | " << alignment_info.rotation_center_x
                        << " | " << alignment_info.rotation_center_y
                        << " | " << alignment_info.rotation
                        << " | " << alignment_info.ranking << endl;
    }
    alignment_file.close();
};

void PhotoAlignmentHandler::align_files(const std::string &reference_file_address, const std::vector<std::string> &files) {
    m_reference_photo_handler = reference_photo_handler_factory(reference_file_address);
    m_reference_file_address = reference_file_address;

    const unsigned int n_files = files.size();
    m_alignment_information_vector.resize(n_files);
    m_local_shifts_vector.resize(n_files);
    auto align_file_multicore = [this](const std::string &file_name, unsigned int file_index) {
        float shift_x, shift_y, rot_center_x, rot_center_y, rotation, ranking;
        if (m_reference_photo_handler->calculate_alignment(file_name, &shift_x, &shift_y, &rot_center_x, &rot_center_y, &rotation, &ranking)) {
            FileAlignmentInformation &alignment_info = m_alignment_information_vector[file_index];
            alignment_info.file_address = file_name;
            alignment_info.shift_x = shift_x;
            alignment_info.shift_y = shift_y;
            alignment_info.rotation_center_x = rot_center_x;
            alignment_info.rotation_center_y = rot_center_y;
            alignment_info.rotation = rotation;
            alignment_info.ranking = ranking;

            const ReferencePhotoHandlerSurface *surface_handler = dynamic_cast<const ReferencePhotoHandlerSurface*>(m_reference_photo_handler.get());
            if (surface_handler != nullptr) {
                vector<LocalShift> local_shifts = surface_handler->get_local_shifts(file_name, shift_x, shift_y, rot_center_x, rot_center_y, rotation);
                m_local_shifts_vector[file_index] = local_shifts;
                alignment_info.local_shifts_handler = LocalShiftsHandler(local_shifts);
            }
        }
        else {
            cout << "Plate solving failed for file: " + file_name + "\n";
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

void PhotoAlignmentHandler::align_all_files_in_folder(const std::string &reference_file_address, const std::string &raw_files_folder) {
    const vector<string> files = get_raw_files_in_folder(raw_files_folder);
    align_files(reference_file_address, files);
}

void PhotoAlignmentHandler::reset() {
    m_alignment_information_vector.clear();
    m_reference_photo_handler = nullptr;
}

FileAlignmentInformation PhotoAlignmentHandler::get_alignment_parameters(const std::string &file_address) const    {
    for (const FileAlignmentInformation &alignment_info :  m_alignment_information_vector) {
        if (alignment_info.file_address == file_address) {
            return alignment_info;
        }
    }
    throw runtime_error("File not found in alignment file: " + file_address);
}

const std::vector<FileAlignmentInformation>& PhotoAlignmentHandler::get_alignment_parameters_vector() const    {
    return PhotoAlignmentHandler::m_alignment_information_vector;
};

std::vector<std::string> PhotoAlignmentHandler::get_file_addresses() const  {
    vector<string> file_addresses;
    for (const FileAlignmentInformation &alignment_info : m_alignment_information_vector) {
        file_addresses.push_back(alignment_info.file_address);
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

std::vector<LocalShift> PhotoAlignmentHandler::get_local_shifts(const std::string& file_address) const   {
    for (unsigned int i_file = 0; i_file < m_alignment_information_vector.size(); i_file++) {
        if (m_alignment_information_vector[i_file].file_address == file_address) {
            return m_local_shifts_vector[i_file];
        }
    }
    throw runtime_error("File not found in alignment file: " + file_address);
};

unique_ptr<ReferencePhotoHandlerBase> PhotoAlignmentHandler::reference_photo_handler_factory(const std::string& raw_file_address)   const  {
    if (m_alignment_method == "stars") {
        return make_unique<ReferencePhotoHandlerStars>(raw_file_address);
    }
    else if (m_alignment_method == "planetary") {
        return make_unique<ReferencePhotoHandlerPlanetary>(raw_file_address, 0.05);
    }
    else if (m_alignment_method == "planetary without rotation")    {
        return make_unique<ReferencePhotoHandlerPlanetaryZeroRotation>(raw_file_address, 0.05);
    }
    else if (m_alignment_method == "surface") {
        return make_unique<ReferencePhotoHandlerSurface>(raw_file_address, 0.05);
    }
    else {
        throw runtime_error("Invalid alignment method: " + m_alignment_method);
    }
};
