#include "../headers/PhotoGroupingTool.h"

#include <algorithm>

using namespace std;
using namespace AstroPhotoStacker;

void PhotoGroupingTool::add_file(const InputFrame &input_frame, int unix_timestamp, float score)  {
    PhotoInfo new_photo;
    new_photo.input_frame = input_frame;
    new_photo.unix_timestamp = unix_timestamp;
    new_photo.score = score;
    m_photos.push_back(new_photo);
};

void PhotoGroupingTool::define_maximum_time_difference_in_group(int time_interval)  {
    m_time_interval = time_interval;
};

void PhotoGroupingTool::run_grouping()  {
    std::sort(m_photos.begin(), m_photos.end(), [](const PhotoInfo &a, const PhotoInfo &b) {
        return a.unix_timestamp < b.unix_timestamp;
    });


    m_groups.clear();
    vector<size_t> photos_not_grouped;
    for (size_t i = 0; i < m_photos.size(); i++) {
        photos_not_grouped.push_back(i);
    }


    while (!photos_not_grouped.empty()) {
        // firstly check how many photos can be grouped for each photo, if that photo was the first in the group
        vector<int> number_of_photos_in_group(m_photos.size(), 1);
        for (unsigned int i_main_photo = 0; i_main_photo < photos_not_grouped.size(); i_main_photo++) {
            for (unsigned int i_other_photo = i_main_photo+1; i_other_photo < photos_not_grouped.size(); i_other_photo++)    {
                const int dt = m_photos.at(photos_not_grouped[i_other_photo]).unix_timestamp - m_photos.at(photos_not_grouped[i_main_photo]).unix_timestamp;
                if (dt > m_time_interval)   {
                    break;
                }
                else    {
                    number_of_photos_in_group[i_main_photo]++;
                }
            }
        }

        // find the photo resulting in the largest group, and group the photos and remove them from the list
        const unsigned int index_max_photos = std::distance(number_of_photos_in_group.begin(), std::max_element(number_of_photos_in_group.begin(), number_of_photos_in_group.end()));
        const unsigned int photos_in_group = number_of_photos_in_group.at(index_max_photos);
        vector<size_t> current_group;
        for (unsigned int i_photo_in_group = 0; i_photo_in_group < photos_in_group; i_photo_in_group++) {
            current_group.push_back(photos_not_grouped.at(index_max_photos));
            photos_not_grouped.erase(photos_not_grouped.begin() + index_max_photos);
        }

        if (current_group.size() > 0) {
            m_groups.push_back(current_group);
        }
    }

    // sort groups by time of the first frame
    std::sort(m_groups.begin(), m_groups.end(), [this](const vector<size_t> &a, const vector<size_t> &b) {
        return m_photos[a[0]].unix_timestamp < m_photos[b[0]].unix_timestamp;
    });

    // not sort photos in each group by score
    for (vector<size_t> &group : m_groups) {
        std::sort(group.begin(), group.end(), [this](size_t a, size_t b) {
            return m_photos[a].score < m_photos[b].score;
        });
    }
};

std::vector<std::vector<InputFrame>> PhotoGroupingTool::get_groups() const {
    std::vector<std::vector<InputFrame>> result;
    for (const vector<size_t> &group : m_groups) {
        std::vector<InputFrame> current_group;
        for (size_t i : group) {
            current_group.push_back(m_photos[i].input_frame);
        }
        result.push_back(current_group);
    }
    return result;
};

const std::vector<std::vector<size_t>>& PhotoGroupingTool::get_groups_indices() const   {
    return m_groups;
};