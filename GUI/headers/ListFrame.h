#pragma once

#include <wx/wx.h>

#include <vector>
#include <string>
#include <memory>


/**
 * @brief Frame (window) for displaying a table.
*/
class ListFrame : public wxFrame  {
    public:
        ListFrame() = delete;

        /**
         * @brief Construct a new List Frame object
         *
         * @param parent pointer to the parent frame
         * @param window_title title of the window
         * @param inside_text text inside the window
         * @param table_description description of the table columns
         * @param tabular_data data to be displayed in the table (2D vector of strings - rows and columns)
         * @param size size of the window
         */
        ListFrame(  wxFrame *parent,
                    const std::string &window_title,
                    const std::string &inside_text,
                    const std::vector<std::string> &table_description,
                    const std::vector<std::vector<std::string>> &tabular_data,
                    const wxSize &size = wxDefaultSize);

    private:
        wxFrame                                 *m_parent;
        std::string                             m_window_title;
        std::string                             m_inside_text;
        std::vector<std::string>                m_table_description;
        std::vector<std::vector<std::string>>   m_tabular_data;

        std::unique_ptr<wxSizer>                m_main_sizer = nullptr;

        bool check_inputs_consistency();
};
