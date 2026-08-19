#include "../headers/MeteorShowerStackingTool.h"

#include "../headers/StarFinder.h"
#include "../headers/TaskScheduler.hxx"
#include "../headers/InputFrameReader.h"
#include "../headers/PhotoRanker.h"
#include "../headers/CalibratedPhotoHandler.h"

#include "../headers/FlatFrameHandler.h"
#include "../headers/DarkFrameHandler.h"

using namespace AstroPhotoStacker;
using namespace std;


FrameClusterInfo MeteorShowerStackingTool::get_cluster_info(const FrameAndGroup &frame) const {
    if (m_frame_clusters_map.find(frame) != m_frame_clusters_map.end()) {
        return m_frame_clusters_map.at(frame);
    }
    return FrameClusterInfo();
};

void MeteorShowerStackingTool::set_cluster_selected(const FrameAndGroup &frame, size_t cluster_id, bool selected)  {
    if (m_frame_clusters_map.find(frame) != m_frame_clusters_map.end()) {
        if (cluster_id < m_frame_clusters_map[frame].clusters_selected.size()) {
            m_frame_clusters_map[frame].clusters_selected[cluster_id] = selected;
        }
    }
};

void MeteorShowerStackingTool::recalculate_clusters(const FrameAndGroup &frame, float cluster_fraction_threshold, bool buffer_brightness)  {
    std::vector< std::vector<std::tuple<int, int> > > clusters;
    if (buffer_brightness && m_frame_in_brightness_buffer == frame) {
        const PixelType threshold = get_threshold_value<PixelType>(m_brightness_buffer.data(), m_brightness_buffer_width*m_brightness_buffer_height, cluster_fraction_threshold);
        clusters = get_clusters(m_brightness_buffer.data(), m_brightness_buffer_width, m_brightness_buffer_height, threshold);
    }
    else {
        InputFrameReader input_frame_reader(frame.input_frame, true);
        const std::vector<PixelType> &brightness = input_frame_reader.get_monochrome_data();
        int width = 0;
        int height = 0;
        input_frame_reader.get_photo_resolution(&width, &height);

        if (buffer_brightness) {
            m_brightness_buffer = brightness;
            m_brightness_buffer_width = width;
            m_brightness_buffer_height = height;
            m_frame_in_brightness_buffer = frame;
        }
        const PixelType threshold = get_threshold_value<PixelType>(brightness.data(), width*height, cluster_fraction_threshold);
        clusters = get_clusters(brightness.data(), width, height, threshold);
    }

    FrameClusterInfo cluster_info;
    keep_clusters_with_at_least_n_pixels(&clusters, 10);
    cluster_info.clusters = clusters;
    for (const std::vector<std::tuple<int, int> > &cluster : clusters) {
        cluster_info.clusters_selected.push_back(false);
        cluster_info.clusters_excentricity.push_back(PhotoRanker::get_cluster_excentricity(cluster));
        cluster_info.clusters_correlation.push_back(PhotoRanker::get_cluster_correlation(cluster));
        cluster_info.cluster_fraction_threshold = cluster_fraction_threshold;
    }
    m_frame_clusters_map[frame] = cluster_info;
};

void MeteorShowerStackingTool::recalculate_clusters(const std::vector<FrameAndGroup> &frames, float cluster_fraction_threshold)  {
    TaskScheduler task_scheduler({m_n_cpus});
    m_tasks_processed = 0;
    for (const FrameAndGroup &frame : frames) {
        task_scheduler.submit([this, frame, cluster_fraction_threshold]() {
            recalculate_clusters(frame, cluster_fraction_threshold);
            m_tasks_processed++;
        }, {1});
    }
    task_scheduler.wait_for_tasks();
    m_tasks_processed = 0;
};

void MeteorShowerStackingTool::clear_buffer() {
    m_brightness_buffer.clear();
    m_brightness_buffer_width = 0;
    m_brightness_buffer_height = 0;
    m_frame_in_brightness_buffer = FrameAndGroup();
};

void MeteorShowerStackingTool::clear_clusters() {
    m_frame_clusters_map.clear();
};

void MeteorShowerStackingTool::keep_clusters_with_at_least_n_pixels(std::vector< std::vector<std::tuple<int, int> > > *clusters, int min_n_pixels) {
    clusters->erase(
        std::remove_if(clusters->begin(), clusters->end(),
                       [min_n_pixels](const std::vector<std::tuple<int, int> > &cluster) {
                           return cluster.size() < static_cast<size_t>(min_n_pixels);
                       }),
        clusters->end());
};

const std::vector<std::vector<float>> &MeteorShowerStackingTool::get_stacked_image(int *width, int *height)    {
    *width  = m_stacked_result_width;
    *height = m_stacked_result_height;
    return m_stacked_result_data;
};

void MeteorShowerStackingTool::stack_frames(const FilelistHandler &filelist_handler, const FrameAndGroup &background_frame)    {
    CalibratedPhotoHandler background_frame_reader(background_frame.input_frame, true);

    const AlignmentResultBase &alignment_background_frame = filelist_handler.get_alignment_info(background_frame.group_number, background_frame.input_frame);
    std::map<int, std::vector<std::shared_ptr<CalibrationFrameBase>>> calibration_handlers_map; // group number to vector of calibration frame handlers
    std::vector<std::shared_ptr<CalibrationFrameBase>> background_calibration_frames = calibration_handlers_map[background_frame.group_number];
    for (const std::shared_ptr<CalibrationFrameBase> &calibration_frame_handler : background_calibration_frames)    {
        background_frame_reader.register_calibration_frame(calibration_frame_handler);
    }
    background_frame_reader.define_alignment(alignment_background_frame);
    background_frame_reader.calibrate();

    const std::vector<std::vector<PixelType>> &background_frame_rgb_data_int = background_frame_reader.get_calibrated_data_after_color_interpolation();
    m_stacked_result_width  = background_frame_reader.get_width();
    m_stacked_result_height = background_frame_reader.get_height();
    m_stacked_result_data.clear();
    for (const std::vector<PixelType> &input_channel : background_frame_rgb_data_int)    {
        std::vector<float> this_channel;
        for (PixelType value : input_channel)   {
            this_channel.push_back(value);
        }
        m_stacked_result_data.push_back(std::move(this_channel));
    }
    std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase>>> calibration_frames_map = get_calibration_frames_map(filelist_handler);

    TaskScheduler task_scheduler({m_n_cpus});
    m_tasks_processed = 0;
    const std::vector<FrameInfo> light_frames = filelist_handler.get_checked_frames_of_type(FrameType::LIGHT);
    for (const FrameInfo &frame : light_frames) {
        FrameAndGroup frame_and_group;
        frame_and_group.input_frame = frame.input_frame;
        frame_and_group.group_number = frame.group_number;
        const std::vector<std::shared_ptr<const CalibrationFrameBase>> &calibration_frames = calibration_frames_map.at(frame.group_number);
        task_scheduler.submit([this, frame_and_group, &filelist_handler, &calibration_frames]() {
            process_one_frame(frame_and_group, filelist_handler, calibration_frames);
            m_tasks_processed++;
        }, {1});
    }
    task_scheduler.wait_for_tasks();
    m_tasks_processed = 0;

};

void MeteorShowerStackingTool::process_one_frame(FrameAndGroup frame, const FilelistHandler &filelist_handler, const std::vector<std::shared_ptr<const CalibrationFrameBase>> &calibration_frames)   {
    const AlignmentResultBase &alignment = filelist_handler.get_alignment_info(frame.group_number, frame.input_frame);

    CalibratedPhotoHandler frame_reader(frame.input_frame, true);
    for (const std::shared_ptr<const CalibrationFrameBase> &calibration_frame_handler : calibration_frames)    {
        frame_reader.register_calibration_frame(calibration_frame_handler);
    }
    frame_reader.define_alignment(alignment);
    frame_reader.calibrate();

    const int width = frame_reader.get_width();
    const int height = frame_reader.get_height();
    vector<bool> selected_pixels_mask(width*height, 0);
    vector<pair<int,int>> pixels_in_clusters;

    FrameClusterInfo frame_cluster_info = m_frame_clusters_map.at(frame);

    for (unsigned int i_cluster = 0; i_cluster < frame_cluster_info.clusters.size(); i_cluster++)   {
        if (!frame_cluster_info.clusters_selected[i_cluster]) {
            continue;
        }

        for (const tuple<int, int> &pixel : frame_cluster_info.clusters[i_cluster])   {
            float x = get<0>(pixel);
            float y = get<1>(pixel);

            // transform to reference frame
            //alignment.transform_from_reference_to_shifted_frame(&x, &y);
            alignment.transform_to_reference_frame(&x, &y);
            int x_int = int(x);
            int y_int = int(y);
            if (x_int >= 0 && x_int < width && y_int >= 0 && y_int < height) {
                const unsigned int index_shifted = y_int*width + x_int;
                selected_pixels_mask[index_shifted] = true;
                pixels_in_clusters.push_back({x_int,y_int});
            }
        }
    }

    const float smearing_radius = 8.;
    const float cluster_radius = 3.;
    const float radius_diff = smearing_radius - cluster_radius;
    vector<float> scale_factor_mask(width*height, 0);

    for (const std::pair<int,int> &pixel_in_cluster : pixels_in_clusters)    {
        const int x = pixel_in_cluster.first;
        const int y = pixel_in_cluster.second;

        scale_factor_mask[width*y + x] = 1;

        for (int dx = -smearing_radius; dx <= smearing_radius; dx++)    {
            const int shifted_x = x+dx;
            if (shifted_x < 0 || shifted_x > width) {
                continue;
            }
            for (int dy = -smearing_radius; dy <= smearing_radius; dy++)    {
                const int shifted_y = y+dy;
                if (shifted_y < 0 || shifted_y > height) {
                    continue;
                }
                const unsigned int index = width*(shifted_y) + shifted_x;
                const float r2 = dx*dx + dy*dy;
                const float r = sqrt(r2);
                if (r > smearing_radius)    {
                    continue;
                }
                else if (r < cluster_radius) {
                    scale_factor_mask[index] = 1.;
                }
                else {
                    const float this_sf = (smearing_radius -r)/radius_diff ;
                    scale_factor_mask[index] = std::max<float>(scale_factor_mask[index], this_sf);
                }
            }
        }
    }

    struct SelectedPixelInformation {
        int x;
        int y;
        std::array<float,3> pixel_values; // values in RGB channels
        float scale_factor;
    };

    const std::vector<std::vector<PixelType>> &rgb_data_calibrated = frame_reader.get_calibrated_data_after_color_interpolation();
    vector<SelectedPixelInformation> selected_pixels_information;
    for (int y = 0; y < height; y++)    {
        for (int x = 0; x < width; x++) {
            if (scale_factor_mask[width*y + x] == 0)    {
                continue;
            }

            const int index = width*y + x;
            SelectedPixelInformation this_pixel_info;
            this_pixel_info.x = x;
            this_pixel_info.y = y;
            this_pixel_info.pixel_values[0] = rgb_data_calibrated[0][index];
            this_pixel_info.pixel_values[1] = rgb_data_calibrated[1][index];
            this_pixel_info.pixel_values[2] = rgb_data_calibrated[2][index];
            this_pixel_info.scale_factor = scale_factor_mask[index];
            selected_pixels_information.push_back(this_pixel_info);
        }
    }

    // at this point we prepared everything we could in multithreaded mode, time to lock the mutex
    {
        std::scoped_lock{m_stacking_mutex};
        for (const SelectedPixelInformation &pixel_info : selected_pixels_information)  {
            const int x = pixel_info.x;
            const int y = pixel_info.y;
            const int index = m_stacked_result_width*y + x;

            if (x >= m_stacked_result_width)    return;
            if (y >= m_stacked_result_height)   return;

            const float weight_signal = pixel_info.scale_factor;
            const float weight_background = 1-weight_signal;

            for (unsigned int i_color = 0; i_color < rgb_data_calibrated.size(); i_color++)   {
                const float old_value = m_stacked_result_data[i_color][index];
                const float new_value = pixel_info.pixel_values[i_color];
                const float mixed_value = old_value*weight_background + new_value*weight_signal;
                m_stacked_result_data[i_color][index] = mixed_value;
            }
        }
    }
};

std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase>>>   MeteorShowerStackingTool::get_calibration_frames_map(const FilelistHandler &filelist_handler)   {
    std::map<int, std::vector<std::shared_ptr<const CalibrationFrameBase>>> result;

    FilelistHandler filelist_handler_only_checked = filelist_handler.get_filelist_with_checked_frames();
    std::vector<int> group_numbers = filelist_handler_only_checked.get_group_numbers();

    for (int group_number : group_numbers) {
        vector<shared_ptr<const CalibrationFrameBase>> calibration_frames_handlers_in_group;

        const std::map<InputFrame, FrameInfo> &dark_frames = filelist_handler_only_checked.get_frames(FrameType::DARK, group_number);
        if (dark_frames.size() > 0) {
            const InputFrame &dark_frame = dark_frames.begin()->first;
            if (!dark_frame.is_still_image()) {
                throw std::runtime_error("Dark frame must be a still image");
            }
            std::shared_ptr<const CalibrationFrameBase> dark_frames_handler = std::make_shared<DarkFrameHandler>(dark_frame);
            calibration_frames_handlers_in_group.push_back(dark_frames_handler);
            cout << "Adding dark frame: " << dark_frame.to_string() << endl;
        }

        const std::map<InputFrame, FrameInfo> &flat_frames = filelist_handler_only_checked.get_frames(FrameType::FLAT, group_number);
        if (flat_frames.size() > 0) {
            const InputFrame &flat_frame = flat_frames.begin()->first;
            if (!flat_frame.is_still_image()) {
                throw std::runtime_error("Flat frame must be a still image");
            }
            std::shared_ptr<const CalibrationFrameBase> flat_frames_handler = std::make_shared<FlatFrameHandler>(flat_frame);
            calibration_frames_handlers_in_group.push_back(flat_frames_handler);
            cout << "Adding flat frame: " << flat_frame.to_string() << endl;
        }

        result[group_number] = calibration_frames_handlers_in_group;
    }

    return result;
};

