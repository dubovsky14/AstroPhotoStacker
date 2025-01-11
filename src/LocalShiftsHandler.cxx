#include "../headers/LocalShiftsHandler.h"
#include "../headers/Common.h"

#include <vector>
#include <tuple>

using namespace AstroPhotoStacker;
using namespace std;

LocalShiftsHandler::LocalShiftsHandler(const std::vector<LocalShift> &shifts)  {
    initialize(shifts);
};

bool LocalShiftsHandler::calculate_shifted_coordinates(int x, int y, int *shifted_x, int *shifted_y, float *score)   {
    if (empty()) {
        return false;
    }
    const int n_neighbors = 3;
    const int query_point[2] = {x, y};
    const std::vector<std::tuple<std::array<int, 2>, std::tuple<int,int,bool,float>>> &closest_nodes = m_kd_tree_shifts.get_k_nearest_neighbors_buffer(query_point, n_neighbors);
    if (closest_nodes.size() == 0) {
        return false;
    }

    bool valid_point_found = false;

    float sum_weights = 0;
    float sum_x = 0;
    float sum_y = 0;
    float score_leading = -1;
    for (const std::tuple<std::array<int,2>, tuple<int,int,bool,float>> &node: closest_nodes) {
        const std::array<int,2> &coordinates = get<0>(node);
        const tuple<int,int,bool,float> &value = get<1>(node);
        const int dx = get<0>(value);
        const int dy = get<1>(value);
        const bool valid_ap = get<2>(value);

        if (!valid_ap) {
            continue;
        }
        valid_point_found = true;

        if (score_leading == -1) {
            score_leading = get<3>(value);
        }

        const float dist2 = (x - coordinates[0]) * (x - coordinates[0]) + (y - coordinates[1]) * (y - coordinates[1]);
        if (dist2 == 0) {
            *shifted_x = coordinates[0];
            *shifted_y = coordinates[1];
            return true;
        }
        const float weight = 1.0 / dist2;
        sum_weights += weight;
        sum_x += weight * dx;
        sum_y += weight * dy;
    }

    if (!valid_point_found) {
        return false;
    }

    *shifted_x = x + sum_x / sum_weights;
    *shifted_y = y + sum_y / sum_weights;
    if (score != nullptr) {
        *score = score_leading;
    }
    return true;
};

void LocalShiftsHandler::draw_ap_boxes_into_image(std::vector<std::vector<unsigned short>> *image, int width, int height, int boxsize, const std::vector<int> &valid_ap_color, const std::vector<int> &invalid_ap_color, int global_shift_x, int global_shift_y) const    {
    for (const auto &shift : m_shifts) {
        const int x = shift.x + shift.dx - global_shift_x;
        const int y = shift.y + shift.dy - global_shift_y;
        const bool valid_ap = shift.valid_ap;

        const int x_min = max(0, x - boxsize/2);
        const int x_max = min(width-1, x + boxsize/2);
        const int y_min = max(0, y - boxsize/2);
        const int y_max = min(height-1, y + boxsize/2);

        if (x_min >= width || x_max < 0 || y_min >= height || y_max < 0) {
            continue;
        }

        const vector<int> &colors = valid_ap ? valid_ap_color : invalid_ap_color;

        for (int i_color = 0; i_color < 3; i_color++) {
            for (int y = y_min; y <= y_max; y++) {
                (*image)[i_color][y*width + x_min] = colors[i_color]; // left
                (*image)[i_color][y*width + x_max] = colors[i_color]; // right
            }
            for (int x = x_min; x <= x_max; x++) {
                (*image)[i_color][y_min*width + x] = colors[i_color]; // top
                (*image)[i_color][y_max*width + x] = colors[i_color]; // bottom
            }
        }
    }
};

std::string LocalShiftsHandler::to_string() const   {
    if (empty()) {
        return "";
    }

    // x,y,dx,dy,valid_ap,score;x,y,dx,dy,valid_ap,score;...
    std::string result = "";
    for (const LocalShift &shift : m_shifts) {
        result += std::to_string(shift.x) + "," +
                  std::to_string(shift.y) + "," +
                  std::to_string(shift.dx) + "," +
                  std::to_string(shift.dy) + "," +
                  std::to_string(shift.valid_ap) + "," +
                  std::to_string(shift.score) + ";";
    }
    return result;
};

LocalShiftsHandler::LocalShiftsHandler(const std::string &string_from_alignment_file)   {
    vector<LocalShift> shifts;
    // x,y,dx,dy,valid_ap,score;x,y,dx,dy,valid_ap,score;...
    const vector<string> elements = split_string(string_from_alignment_file, ";");
    for (const string &element : elements) {
        const vector<string> values = split_string(element, ",");
        if (values.size() != 6) {
            continue;
        }
        LocalShift shift;
        shift.x = std::stoi(values[0]);
        shift.y = std::stoi(values[1]);
        shift.dx = std::stoi(values[2]);
        shift.dy = std::stoi(values[3]);
        shift.valid_ap = std::stoi(values[4]);
        shift.score = std::stof(values[5]);
        shifts.push_back(shift);
    }
    initialize(shifts);
};

void LocalShiftsHandler::initialize(const std::vector<LocalShift> &shifts)  {
    m_shifts = shifts;

    if (shifts.empty()) {
        m_empty = true;
        return;
    }

    for (const auto &shift : shifts) {
        const std::vector<int> coordinate = {shift.x, shift.y};
        const std::tuple<int,int,bool,float> value = std::tuple<int,int,bool,float>(shift.dx, shift.dy, shift.valid_ap, shift.score);

        m_kd_tree_shifts.add_point(coordinate, value);
    }
    m_kd_tree_shifts.build_tree_structure();
    m_empty = false;
};