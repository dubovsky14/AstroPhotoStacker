#include "../headers/PhotoGroupingTool.h"

#include <iostream>

using namespace std;


int main()  {
    PhotoGroupingTool photo_grouping_tool;
    photo_grouping_tool.add_file("file00", 1728674080, 0.1);
    photo_grouping_tool.add_file("file01", 1728674081, 0.1);
    photo_grouping_tool.add_file("file02", 1728674081, 0.7);
    photo_grouping_tool.add_file("file03", 1728674081, 0.5);
    photo_grouping_tool.add_file("file04", 1728674081, 0.44);
    photo_grouping_tool.add_file("file05", 1728674081, 0.14);
    photo_grouping_tool.add_file("file06", 1728674082, 0.62);
    photo_grouping_tool.add_file("file07", 1728674082, 0.47);
    photo_grouping_tool.add_file("file08", 1728674082, 0.82);
    photo_grouping_tool.add_file("file09", 1728674083, 0.91);

    photo_grouping_tool.add_file("file10", 1728674143, 0.98);
    photo_grouping_tool.add_file("file11", 1728674141, 0.5);
    photo_grouping_tool.add_file("file12", 1728674141, 0.2);
    photo_grouping_tool.add_file("file13", 1728674142, 0.4);
    photo_grouping_tool.add_file("file14", 1728674142, 0.8);
    photo_grouping_tool.add_file("file15", 1728674142, 0.2);
    photo_grouping_tool.add_file("file16", 1728674142, 0.5);
    photo_grouping_tool.add_file("file17", 1728674143, 0.2);
    photo_grouping_tool.add_file("file18", 1728674143, 0.8);
    photo_grouping_tool.add_file("file19", 1728674143, 0.12);
    photo_grouping_tool.add_file("file20", 1728674144, 0.65);

    photo_grouping_tool.add_file("file31", 1728674192, 0.4);
    photo_grouping_tool.add_file("file32", 1728674193, 0.3);
    photo_grouping_tool.add_file("file33", 1728674193, 0.1);
    photo_grouping_tool.add_file("file34", 1728674193, 0.7);
    photo_grouping_tool.add_file("file35", 1728674194, 0.5);
    photo_grouping_tool.define_maximum_time_difference_in_group(10);
    photo_grouping_tool.run_grouping();
    const vector<vector<string>> groups = photo_grouping_tool.get_groups();
    for (const vector<string> &group : groups) {
        for (const string &file : group) {
            cout << file << " ";
        }
        cout << endl;
    }
    return 0;
}