#pragma once

#include <wx/wx.h>

inline void set_text_size(wxStaticText* text, int size) {
    wxFont font = text->GetFont();
    font.SetPointSize(size);
    text->SetFont(font);
}