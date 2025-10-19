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

    PlateSolvingResult plate_solving_result = ReferencePhotoHandlerStars::calculate_alignment(input_frame);
    if (!plate_solving_result.is_valid) {
        return false;
    }

    GeometricTransformer geometric_transformer( plate_solving_result.shift_x,
                                                plate_solving_result.shift_y,
                                                plate_solving_result.rotation_center_x,
                                                plate_solving_result.rotation_center_y,
                                                plate_solving_result.rotation);

    float x_ref = x;
    float y_ref = y;
    geometric_transformer.transform_to_reference_frame(&x_ref, &y_ref);

    m_comet_positions[input_frame] = std::make_pair(x_ref, y_ref);
    return true;
};


PlateSolvingResult ReferencePhotoHandlerComet::calculate_alignment(const InputFrame &input_frame, float *ranking) const {
    try {
        InputFrameReader input_frame_reader(input_frame);
        const int width = input_frame_reader.get_width();
        const int height = input_frame_reader.get_height();

        const vector<PixelType> brightness = input_frame_reader.get_monochrome_data();
        const PixelType threshold = get_threshold_value(brightness.data(), width*height, 0.0005);

        vector<tuple<float,float,int> > clusters = get_stars(brightness.data(), width, height, threshold); // one if these "stars" should be the comet
        keep_only_stars_above_size(&clusters, 9);
        sort_stars_by_size(&clusters);
        clusters.resize(min<int>(clusters.size(), 20));

        if (ranking != nullptr) {
            *ranking = PhotoRanker::calculate_frame_ranking(input_frame);
        }

        PlateSolvingResult plate_solving_result = plate_solve(clusters);
        GeometricTransformer geometric_transformer( plate_solving_result.shift_x,
                                                    plate_solving_result.shift_y,
                                                    plate_solving_result.rotation_center_x,
                                                    plate_solving_result.rotation_center_y,
                                                    plate_solving_result.rotation);

        std::pair<float,float> expected_comet_position = calculate_expected_comet_position(input_frame_reader.get_metadata().timestamp);
        std::pair<float,float> closest_cluster;
        float minimal_distance_squared = 1e10;
        for (const std::tuple<float,float,int> &cluster : clusters) {
            float x = std::get<0>(cluster);
            float y = std::get<1>(cluster);
            geometric_transformer.transform_to_reference_frame(&x, &y);
            const float dx = x - expected_comet_position.first;
            const float dy = y - expected_comet_position.second;
            const float distance_squared = dx*dx + dy*dy;
            if (distance_squared < minimal_distance_squared) {
                minimal_distance_squared = distance_squared;
                closest_cluster = std::make_pair(x, y);
            }
        }

        const std::pair<float,float> comet_position = minimal_distance_squared < 25.0f ? closest_cluster : expected_comet_position;

        plate_solving_result.shift_x -= comet_position.first - m_comet_position_reference_frame.first;
        plate_solving_result.shift_y -= comet_position.second - m_comet_position_reference_frame.second;
        return plate_solving_result;
    }
    catch (runtime_error &e)    {
        cout << "Error: " << e.what() << endl;
        PlateSolvingResult plate_solving_result;
        plate_solving_result.is_valid = false;
        return plate_solving_result;
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

    cout << "Fitting comet path through " << timestamps.size() << " positions. Data:" << endl;
    for (size_t i = 0; i < timestamps.size(); ++i) {
        cout << "  t = " << timestamps[i] << "s, x = " << positions_x[i] << ", y = " << positions_y[i] << endl;
    }


    const auto [v_x, initial_x] = least_squares_fit(timestamps, positions_x);
    const auto [v_y, initial_y] = least_squares_fit(timestamps, positions_y);

    m_comet_initial_position = std::make_pair(initial_x, initial_y);
    m_comet_velocity = std::make_pair(v_x, v_y);

    if (m_comet_position_reference_frame.first < 0.0f && m_comet_position_reference_frame.second < 0.0f) {
        InputFrameReader input_frame_reader_reference_frame(m_reference_input_frame, false);
        const int timestamp = input_frame_reader_reference_frame.get_metadata().timestamp;
        m_comet_position_reference_frame = calculate_expected_comet_position(timestamp);
    }

    cout << "Fitted comet path: " << endl;
    cout << "  Initial position: (" << m_comet_initial_position.first << ", " << m_comet_initial_position.second << ")" << endl;
    cout << "  Velocity: (" << m_comet_velocity.first << ", " << m_comet_velocity.second << ")" << endl;

    return true;
};

std::pair<float,float> ReferencePhotoHandlerComet::calculate_expected_comet_position(int timestamp) const {
    const double dt = static_cast<double>(timestamp - m_minimal_timestamp);
    const float expected_x = m_comet_initial_position.first + m_comet_velocity.first * dt;
    const float expected_y = m_comet_initial_position.second + m_comet_velocity.second * dt;
    return std::make_pair(expected_x, expected_y);
}