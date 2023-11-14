#pragma once

#include <wx/wx.h>

#include <vector>
#include <string>
#include <memory>

class ListFrame : public wxFrame  {
    public:
        ListFrame() = delete;

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
