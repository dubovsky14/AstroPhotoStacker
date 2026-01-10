#include "../headers/LensCorrectionsTooolGUI.h"
#include "../headers/IndividualColorStretchingBlackCorrectionWhite.h"
#include "../headers/Common.h"
#include "../headers/PhotoGroupingTool.h"
#include "../headers/StackerConfigureTool.h"

#include "../headers/StackSettings.h"

#include "../../headers/AlignedImagesProducer.h"
#include "../../headers/CalibrationFrameBase.h"
#include "../../headers/DarkFrameHandler.h"
#include "../../headers/FlatFrameHandler.h"
#include "../../headers/thread_pool.h"
#include "../../headers/ImageFilesInputOutput.h"
#include "../../headers/TimeLapseVideoCreator.h"
#include "../../headers/PhotoAlignmentHandler.h"


#include "../headers/MainFrame.h"
#include "../headers/ProgressBarWindow.h"

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/progdlg.h>

#include <vector>
#include <string>



using namespace std;
using namespace AstroPhotoStacker;

LensCorrectionsToolGUI::LensCorrectionsToolGUI(MyFrame *parent, const InputFrame &reference_frame) :
        wxFrame(parent, wxID_ANY, "Lens corrections tool", wxDefaultPosition, wxSize(1000, 1000)),
        m_parent(parent)    {

    m_reference_frame = reference_frame;


    InputFrameReader input_frame_data(reference_frame); // to get image resolution
    const int image_width = input_frame_data.get_width();
    const int image_height = input_frame_data.get_height();

    m_lens_corrections_tool = make_unique<AstroPhotoStacker::LensCorrectionsTool>(
        image_width,
        image_height
    );
    m_lens_corrections_tool->initialize();

    m_exposure_stretcher.add_luminance_stretcher(std::make_shared<IndividualColorStretchingBlackCorrectionWhite>());

    m_main_vertical_sizer = new wxBoxSizer(wxVERTICAL);

    m_image_preview = make_unique<ImagePreview>(this, 600, 400, 255, true);
    m_image_preview->set_stretcher(&m_exposure_stretcher);

    if (reference_frame != InputFrame()) {
        m_image_preview->read_preview_from_frame(reference_frame, m_lens_corrections_tool.get());
        m_image_preview->update_preview_bitmap();
    }


    m_image_preview_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_main_vertical_sizer->Add(m_image_preview_sizer, 2, wxCENTER, 5);

    m_image_preview_sizer->Add(m_image_preview->get_image_preview_bitmap(), 1, wxCENTER, 0);


    m_basic_settings_sizer = new wxBoxSizer(wxVERTICAL);
    m_main_vertical_sizer->Add(m_basic_settings_sizer, 1, wxEXPAND | wxALL, 5);
    add_exposure_correction_spin_ctrl();

    SetSizer(m_main_vertical_sizer);
};

void LensCorrectionsToolGUI::initialize_gui()   {


};

void LensCorrectionsToolGUI::add_exposure_correction_spin_ctrl()   {
    m_exposure_correction_slider = make_unique<FloatingPointSlider>(
        this,
        "Exposure correction: ",
        -7.0,
        7.0,
        0.0,
        0.1,
        1,
        [this](float value){
            IndividualColorStretchingToolBase &luminance_stretcher = m_exposure_stretcher.get_luminance_stretcher(0);
            (dynamic_cast<IndividualColorStretchingBlackCorrectionWhite&>(luminance_stretcher)).set_stretching_parameters(0,value,1);
            m_image_preview->update_preview_bitmap();
        }
    );
    m_exposure_correction_slider->add_sizer(m_basic_settings_sizer, 0, wxEXPAND, 5);
};
