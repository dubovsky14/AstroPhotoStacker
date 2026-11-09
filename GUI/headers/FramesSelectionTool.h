#pragma once

#include "../headers/FilelistHandlerGUIInterface.h"
#include "../headers/MainFrame.h"
#include "../headers/FloatingPointSlider.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include <vector>
#include <string>
#include <memory>
#include <utility>
#include <functional>


/**
 * @brief Frame (dialog window) for applying selection to frames
*/

enum class ComparisonEnum {
    Less,
    LessEqual,
    Equal,
    NotEqual,
    GreaterEqual,
    Greater,
};



class FramesSelectionTool : public wxFrame  {
    public:
        /**
         * @brief Construct a new Frames Selection Tool object
         *
         * @param parent pointer to the parent frame (main frame)
         * @param filelist_handler_gui_interface pointer to the filelist handler GUI interface object
         */
        FramesSelectionTool(MyFrame *parent, FilelistHandlerGUIInterface *filelist_handler_gui_interface);

    private:
        FilelistHandlerGUIInterface *m_filelist_handler_gui_interface = nullptr;

        wxBoxSizer *m_main_sizer = nullptr;
        wxSize m_window_size = wxSize(600, 400);


        void initialize_comparison_functions_vector();

        void add_value_type_dropdown();

        void add_selection_line();

        void add_apply_button();

        std::vector<std::pair<std::string, std::function<bool(const AstroPhotoStacker::FrameInfo&, const std::string&, ComparisonEnum)>>> m_selection_functions;

        template<typename T>
        static bool compare(const T& lhs, const T& rhs, ComparisonEnum comparison) {
            switch (comparison) {
                case ComparisonEnum::Less:          return lhs < rhs;
                case ComparisonEnum::LessEqual:     return lhs <= rhs;
                case ComparisonEnum::Equal:         return lhs == rhs;
                case ComparisonEnum::NotEqual:      return lhs != rhs;
                case ComparisonEnum::GreaterEqual:  return lhs >= rhs;
                case ComparisonEnum::Greater:       return lhs > rhs;
                default: throw std::invalid_argument("Invalid comparison");
            }
        };

        static const std::vector<std::pair<std::string, ComparisonEnum>> s_comparison_options;

        wxTextCtrl* m_value_input = nullptr;
        int m_comparison_operator_selection = 0;
        int m_value_type_selection = 0;

        MyFrame *m_parent_my_frame = nullptr;

};
