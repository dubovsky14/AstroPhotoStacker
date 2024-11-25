#pragma once

#include <wx/wx.h>

#include <string>
#include <cmath>

inline void set_text_size(wxStaticText* text, int size) {
    wxFont font = text->GetFont();
    font.SetPointSize(size);
    text->SetFont(font);
}

inline std::string get_rounded_value(float value, unsigned int n_decimals)  {
    int int_value = value;

    int decimal_part = (abs(value)-abs(int_value))*pow(10, n_decimals);
    std::string decimal_part_str = std::to_string(decimal_part);
    if (decimal_part_str.size() < n_decimals) {
        decimal_part_str = std::string(n_decimals - decimal_part_str.size(), '0') + decimal_part_str;
    }
    return std::to_string(int_value) + "." + decimal_part_str;
};