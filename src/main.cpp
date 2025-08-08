#include "ui/MainFrame.h"

class ConverterApp : public wxApp {
public:
    bool OnInit() override {
        if (!wxApp::OnInit()) return false;
        ConverterFrame* frame = new ConverterFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(ConverterApp);


