#include "../headers/ReferencePhotoHandlerComet.h"
#include "../headers/GeometricTransformations.h"
#include "../headers/InputFrameReader.h"
#include "../headers/PhotoRanker.h"

using namespace AstroPhotoStacker;
using namespace std;

ReferencePhotoHandlerComet::ReferencePhotoHandlerComet(const InputFrame &input_frame, float threshold_fraction) :
    ReferencePhotoHandlerStars(input_frame, threshold_fraction) {
    m_reference_input_frame = input_frame;
};

bool ReferencePhotoHandlerComet::add_comet_position(const InputFrame &input_frame, float x, float y) {
    if (m_reference_input_frame == input_frame) {
        m_comet_positions[input_frame]      = std::make_pair(x, y);
        m_comet_position_reference_frame    = std::make_pair(x, y);
        return true;
    }

    std::unique_ptr<AlignmentResultBase> plate_solving_result = ReferencePhotoHandlerStars::calculate_alignment(input_frame);
    if (!plate_solving_result->is_valid()) {
        return false;
    }

    plate_solving_result->transform_to_reference_frame(&x, &y);
    m_comet_positions[input_frame] = std::make_pair(x, y);
    return true;
};


std::unique_ptr<AlignmentResultBase> ReferencePhotoHandlerComet::calculate_alignment(const InputFrame &input_frame) const {
    try {
        InputFrameReader input_frame_reader(input_frame);
        const int width = input_frame_reader.get_width();
        const int height = input_frame_reader.get_height();

        const vector<PixelType> brightness = input_frame_reader.get_monochrome_data();
        const PixelType threshold = get_threshold_value(brightness.data(), width*height, 0.002);

        vector<tuple<float,float,int> > clusters = get_stars(brightness.data(), width, height, threshold); // one if these "stars" should be the comet
        keep_only_stars_above_size(&clusters, 9);
        sort_stars_by_size(&clusters);
        clusters.resize(min<int>(clusters.size(), 20));


        std::unique_ptr<AlignmentResultPlateSolving> plate_solving_result = plate_solve(clusters);

        std::pair<float,float> expected_comet_position = calculate_expected_comet_position(input_frame_reader.get_metadata().timestamp);
        std::pair<float,float> closest_cluster;
        float minimal_distance_squared = 1e10;
        for (const std::tuple<float,float,int> &cluster : clusters) {
            float x = std::get<0>(cluster);
            float y = std::get<1>(cluster);
            plate_solving_result->transform_to_reference_frame(&x, &y);
            const float dx = x - expected_comet_position.first;
            const float dy = y - expected_comet_position.second;
            const float distance_squared = dx*dx + dy*dy;
            if (distance_squared < minimal_distance_squared) {
                minimal_distance_squared = distance_squared;
                closest_cluster = std::make_pair(x, y);
            }
        }

        const std::pair<float,float> comet_position = minimal_distance_squared < 25.0f ? closest_cluster : expected_comet_position;

        float shift_x,shift_y,rotation_center_x,rotation_center_y,rotation, zoom;
        plate_solving_result->get_parameters(&shift_x, &shift_y, &rotation_center_x, &rotation_center_y, &rotation, &zoom);

        shift_x -= comet_position.first - m_comet_position_reference_frame.first;
        shift_y -= comet_position.second - m_comet_position_reference_frame.second;

        plate_solving_result->set_parameters(shift_x, shift_y, rotation_center_x, rotation_center_y, rotation, zoom);
        plate_solving_result->set_ranking_score(PhotoRanker::calculate_frame_ranking(input_frame));

        return plate_solving_result;
    }
    catch (runtime_error &e)    {
        cout << "Error: " << e.what() << endl;
        return make_unique<AlignmentResultPlateSolving>();
    }
};



bool ReferencePhotoHandlerComet::fit_comet_path() {
    vector<double> timestamps;
    vector<double> positions_x;
    vector<double> positions_y;
    m_minimal_timestamp = std::numeric_limits<int>::max();
    for (const auto &entry : m_comet_positions) {
        InputFrameReader input_frame_reader(entry.first, false);
        const int timestamp = input_frame_reader.get_metadata().timestamp;
        if (timestamp >= 0) {
            timestamps.push_back(timestamp);
            positions_x.push_back(entry.second.first);
            positions_y.push_back(entry.second.second);
            m_minimal_timestamp = min(m_minimal_timestamp, timestamp);
        }
    }

    for (double &ts : timestamps) {
        ts -= m_minimal_timestamp;
    }

    if (timestamps.size() < 2) {
        return false;
    }

    auto least_squares_fit = [](const vector<double> &x, const vector<double> &y) -> pair<double,double> {
        const size_t n = x.size();
        double sum_x = 0.0;
        double sum_y = 0.0;
        double sum_xy = 0.0;
        double sum_xx = 0.0;

        for (size_t i = 0; i < n; ++i) {
            sum_x += x[i];
            sum_y += y[i];
            sum_xy += x[i] * y[i];
            sum_xx += x[i] * x[i];
        }

        const double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
        const double intercept = (sum_y - slope * sum_x) / n;

        return std::make_pair(slope, intercept);
    };

    const auto [v_x, initial_x] = least_squares_fit(timestamps, positions_x);
    const auto [v_y, initial_y] = least_squares_fit(timestamps, positions_y);

    m_comet_initial_position = std::make_pair(initial_x, initial_y);
    m_comet_velocity = std::make_pair(v_x, v_y);

    if (m_comet_position_reference_frame.first < 0.0f && m_comet_position_reference_frame.second < 0.0f) {
        InputFrameReader input_frame_reader_reference_frame(m_reference_input_frame, false);
        const int timestamp = input_frame_reader_reference_frame.get_metadata().timestamp;
        m_comet_position_reference_frame = calculate_expected_comet_position(timestamp);
    }

    return true;
};

std::pair<float,float> ReferencePhotoHandlerComet::calculate_expected_comet_position(int timestamp) const {
    const double dt = static_cast<double>(timestamp - m_minimal_timestamp);
    const float expected_x = m_comet_initial_position.first + m_comet_velocity.first * dt;
    const float expected_y = m_comet_initial_position.second + m_comet_velocity.second * dt;
    return std::make_pair(expected_x, expected_y);
}