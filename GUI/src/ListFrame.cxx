#include "../headers/ListFrame.h"

#include <wx/grid.h>

#include <vector>
#include <stdexcept>
#include <iostream>


using namespace std;

ListFrame::ListFrame(   wxFrame *parent,
                        const std::string &window_title,
                        const std::string &inside_text,
                        const std::vector<std::string> &table_description,
                        const std::vector<std::vector<std::string>> &tabular_data,
                        const wxSize &size)
    :   wxFrame(parent, wxID_ANY, window_title, wxDefaultPosition, size),
        m_parent(parent),
        m_window_title(window_title),
        m_inside_text(inside_text),
        m_table_description(table_description),
        m_tabular_data(tabular_data)         {

    const bool inputs_are_consistent = check_inputs_consistency();
    if (!inputs_are_consistent) {
        throw  runtime_error("ERROR: tabular inputs are not consistent!");
    };

    //m_main_sizer = new make_unique<wxBoxSizer>(wxVERTICAL);
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* inside_static_text = new wxStaticText(this, wxID_ANY, inside_text);
    inside_static_text->SetFont(wxFont(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));

    wxGrid* grid = new wxGrid(this, wxID_ANY);
    const int n_rows = tabular_data.size();
    const int n_cols = table_description.size();
    grid->CreateGrid(n_rows, n_cols);

    vector<unsigned int> col_widths(n_cols, 0);

    // Set column labels
    for (unsigned int i = 0; i < table_description.size(); ++i)   {
        grid->SetColLabelValue(i, table_description[i]);
        if (table_description[i].length() > col_widths[i])   {
            col_widths[i] = table_description[i].length();
        };
    };

    // Set data
    for (unsigned int i = 0; i < tabular_data.size(); ++i)   {
        for (unsigned int j = 0; j < tabular_data[i].size(); ++j)   {
            grid->SetCellValue(i, j, tabular_data[i][j]);
            if (tabular_data[i][j].length() > col_widths[j])   {
                col_widths[j] = tabular_data[i][j].length();
            };
        };
    };

    // Set column widths
    for (unsigned int i = 0; i < col_widths.size(); ++i)   {
        grid->SetColSize(i, col_widths[i]*10);
    };


    wxButton* button_ok = new wxButton(this, wxID_ANY, "OK");
    button_ok->Bind(wxEVT_BUTTON, [this](wxCommandEvent&){
        this->Close();
        this->Destroy();
    });

    main_sizer->Add(inside_static_text, 0, wxALIGN_CENTER_HORIZONTAL, 5);
    main_sizer->Add(grid, 0, wxALIGN_CENTER_HORIZONTAL, 5);
    main_sizer->Add(button_ok, 0, wxALIGN_CENTER_HORIZONTAL, 5);


    this->SetSizer(main_sizer);

};

bool ListFrame::check_inputs_consistency()  {
    const unsigned int n_columns = m_table_description.size();
    for (const auto& row : m_tabular_data)    {
        if (row.size() != n_columns)    {
            return false;
        };
    };
    return true;
};