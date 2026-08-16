#include "../headers/MeteorShowerStackingTool.h"

#include "../headers/StarFinder.h"
#include "../headers/TaskScheduler.hxx"
#include "../headers/InputFrameReader.h"
#include "../headers/PhotoRanker.h"

using namespace AstroPhotoStacker;
using namespace std;


FrameClusterInfo MeteorShowerStackingTool::get_cluster_info(const InputFrame &frame) const {
    if (m_frame_clusters_map.find(frame) != m_frame_clusters_map.end()) {
        return m_frame_clusters_map.at(frame);
    }
    return FrameClusterInfo();
};


void MeteorShowerStackingTool::recalculate_clusters(const InputFrame &frame, float cluster_fraction_threshold, bool buffer_brightness)  {
    std::vector< std::vector<std::tuple<int, int> > > clusters;
    if (buffer_brightness && m_frame_in_brightness_buffer == frame) {
        const PixelType threshold = get_threshold_value<PixelType>(m_brightness_buffer.data(), m_brightness_buffer_width*m_brightness_buffer_height, cluster_fraction_threshold);
        clusters = get_clusters(m_brightness_buffer.data(), m_brightness_buffer_width, m_brightness_buffer_height, threshold);
    }
    else {
        InputFrameReader input_frame_reader(frame, true);
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
    }
    m_frame_clusters_map[frame] = cluster_info;
};

void MeteorShowerStackingTool::recalculate_clusters(const std::vector<InputFrame> &frames, float cluster_fraction_threshold)  {
    TaskScheduler task_scheduler({m_n_cpus});
    m_tasks_processed = 0;
    for (const InputFrame &frame : frames) {
        task_scheduler.submit([this, frame, cluster_fraction_threshold]() {
            recalculate_clusters(frame, cluster_fraction_threshold);
            m_tasks_processed++;
        }, {1});
    }
    task_scheduler.wait_for_tasks();
};

void MeteorShowerStackingTool::clear_buffer() {
    m_brightness_buffer.clear();
    m_brightness_buffer_width = 0;
    m_brightness_buffer_height = 0;
    m_frame_in_brightness_buffer = InputFrame();
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
