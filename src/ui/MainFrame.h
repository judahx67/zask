#pragma once

#include <wx/wx.h>
#include <wx/filepicker.h>
#include <wx/process.h>
#include <wx/filename.h>
#include <wx/timer.h>
#include <wx/thread.h>
#include <vector>
#include "core/MediaConverter.h"

class ConverterFrame : public wxFrame {
public:
    ConverterFrame() : wxFrame(nullptr, wxID_ANY, "Cross-Platform Media Converter", wxDefaultPosition, wxSize(760, 760)) {
        wxPanel* panel = new wxPanel(this);

        wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

        // Input file picker
        wxBoxSizer* inputSizer = new wxBoxSizer(wxHORIZONTAL);
        inputPicker = new wxFilePickerCtrl(panel, wxID_ANY, wxEmptyString, "Choose media file", wxFileSelectorDefaultWildcardStr, wxDefaultPosition, wxDefaultSize, wxFLP_DEFAULT_STYLE | wxFLP_FILE_MUST_EXIST);
        inputSizer->Add(new wxStaticText(panel, wxID_ANY, "Input:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        inputSizer->Add(inputPicker, 1, wxEXPAND);
        rootSizer->Add(inputSizer, 0, wxEXPAND | wxALL, 8);

        // Output file picker
        wxBoxSizer* outputSizer = new wxBoxSizer(wxHORIZONTAL);
        outputPicker = new wxFilePickerCtrl(panel, wxID_ANY, wxEmptyString, "Save as", wxFileSelectorDefaultWildcardStr, wxDefaultPosition, wxDefaultSize, wxFLP_SAVE | wxFLP_USE_TEXTCTRL);
        outputSizer->Add(new wxStaticText(panel, wxID_ANY, "Output:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        outputSizer->Add(outputPicker, 1, wxEXPAND);
        rootSizer->Add(outputSizer, 0, wxEXPAND | wxALL, 8);

        // Options row 1: Trimming
        wxBoxSizer* trimSizer = new wxBoxSizer(wxHORIZONTAL);
        startTimeCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(120, -1));
        endTimeCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(120, -1));
        startTimeCtrl->SetHint("Start -ss (e.g. 00:00:05)");
        endTimeCtrl->SetHint("End -to (e.g. 00:00:10)");
        trimSizer->Add(new wxStaticText(panel, wxID_ANY, "Trim:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        trimSizer->Add(new wxStaticText(panel, wxID_ANY, "Start"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
        trimSizer->Add(startTimeCtrl, 0, wxRIGHT, 16);
        trimSizer->Add(new wxStaticText(panel, wxID_ANY, "End"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
        trimSizer->Add(endTimeCtrl, 0, wxRIGHT, 16);
        rootSizer->Add(trimSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

        // Accurate trim toggle
        accurateTrim = new wxCheckBox(panel, wxID_ANY, "Accurate trim (precise, slower)");
        accurateTrim->SetValue(true);
        rootSizer->Add(accurateTrim, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

        // Output kind selector
        wxArrayString kinds;
        kinds.Add("Video");
        kinds.Add("Audio");
        outputKind = new wxRadioBox(panel, wxID_ANY, "Output", wxDefaultPosition, wxDefaultSize, kinds, 1, wxRA_SPECIFY_COLS);
        outputKind->SetSelection(0);
        rootSizer->Add(outputKind, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

        // Video options
        wxStaticBoxSizer* videoBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Video Options");
        wxFlexGridSizer* videoGrid = new wxFlexGridSizer(0, 2, 6, 8);
        videoGrid->AddGrowableCol(1, 1);
        videoCodecChoice = new wxChoice(panel, wxID_ANY);
        videoCodecChoice->Append("Copy");
        videoCodecChoice->Append("H.264 (libx264)");
        videoCodecChoice->Append("HEVC (libx265)");
        videoCodecChoice->Append("VP9 (libvpx-vp9)");
        videoCodecChoice->Append("AV1 (libaom-av1)");
        videoCodecChoice->SetSelection(0);
        videoGrid->Add(new wxStaticText(panel, wxID_ANY, "Video codec:"), 0, wxALIGN_CENTER_VERTICAL);
        videoGrid->Add(videoCodecChoice, 1, wxEXPAND);

        speedChoice = new wxChoice(panel, wxID_ANY);
        speedChoice->Append("ultrafast");
        speedChoice->Append("superfast");
        speedChoice->Append("veryfast");
        speedChoice->Append("faster");
        speedChoice->Append("fast");
        speedChoice->Append("medium");
        speedChoice->Append("slow");
        speedChoice->Append("slower");
        speedChoice->Append("veryslow");
        speedChoice->SetSelection(2);
        videoGrid->Add(new wxStaticText(panel, wxID_ANY, "Speed:"), 0, wxALIGN_CENTER_VERTICAL);
        videoGrid->Add(speedChoice, 1, wxEXPAND);

        videoBitrateCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(120, -1));
        videoBitrateCtrl->SetHint("e.g. 2000k");
        videoGrid->Add(new wxStaticText(panel, wxID_ANY, "VBitrate:"), 0, wxALIGN_CENTER_VERTICAL);
        videoGrid->Add(videoBitrateCtrl, 1, wxEXPAND);

        // Threads selector with warning
        wxBoxSizer* threadsRow = new wxBoxSizer(wxHORIZONTAL);
        threadsChoice = new wxChoice(panel, wxID_ANY);
        threadsChoice->Append("Auto");
        unsigned int cpuCount = wxThread::GetCPUCount();
        if (cpuCount == 0) cpuCount = 8; // fallback
        for (unsigned int i = 1; i <= cpuCount; ++i) {
            threadsChoice->Append(wxString::Format("%u", i));
        }
        threadsChoice->SetSelection(0);
        threadsRow->Add(threadsChoice, 0, wxRIGHT, 8);
        wxStaticText* threadsWarn = new wxStaticText(panel, wxID_ANY, "Warning: high threads may reduce determinism/stability");
        threadsWarn->SetForegroundColour(wxColour(200, 60, 60));
        threadsRow->Add(threadsWarn, 0, wxALIGN_CENTER_VERTICAL);

        videoGrid->Add(new wxStaticText(panel, wxID_ANY, "Threads:"), 0, wxALIGN_CENTER_VERTICAL);
        videoGrid->Add(threadsRow, 1, wxEXPAND);

        videoBox->Add(videoGrid, 1, wxEXPAND | wxALL, 8);
        rootSizer->Add(videoBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

        // Audio options
        wxStaticBoxSizer* audioBox = new wxStaticBoxSizer(wxVERTICAL, panel, "Audio Options");
        wxFlexGridSizer* audioGrid = new wxFlexGridSizer(0, 2, 6, 8);
        audioGrid->AddGrowableCol(1, 1);
        audioCodecChoice = new wxChoice(panel, wxID_ANY);
        audioCodecChoice->Append("MP3 (libmp3lame)");
        audioCodecChoice->Append("WAV (pcm_s16le)");
        audioCodecChoice->Append("FLAC (flac)");
        audioCodecChoice->Append("Opus (libopus)");
        audioCodecChoice->Append("Ogg Vorbis (libvorbis)");
        audioCodecChoice->SetSelection(0);
        audioGrid->Add(new wxStaticText(panel, wxID_ANY, "Audio codec:"), 0, wxALIGN_CENTER_VERTICAL);
        audioGrid->Add(audioCodecChoice, 1, wxEXPAND);

        audioQualityChoice = new wxChoice(panel, wxID_ANY);
        audioGrid->Add(new wxStaticText(panel, wxID_ANY, "Quality:"), 0, wxALIGN_CENTER_VERTICAL);
        audioGrid->Add(audioQualityChoice, 1, wxEXPAND);

        audioBox->Add(audioGrid, 1, wxEXPAND | wxALL, 8);
        rootSizer->Add(audioBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

        // Actions row
        wxBoxSizer* actionsSizer = new wxBoxSizer(wxHORIZONTAL);
        convertButton = new wxButton(panel, wxID_ANY, "Convert");
        stopButton = new wxButton(panel, wxID_ANY, "Stop");
        actionsSizer->AddStretchSpacer(1);
        actionsSizer->Add(convertButton, 0, wxRIGHT, 8);
        actionsSizer->Add(stopButton, 0);
        rootSizer->Add(actionsSizer, 0, wxEXPAND | wxALL, 8);

        // Debug toggle and command box (same row)
        wxBoxSizer* debugSizer = new wxBoxSizer(wxHORIZONTAL);
        debugToggle = new wxCheckBox(panel, wxID_ANY, "Show command (debug)");
        debugSizer->Add(debugToggle, 0, wxRIGHT, 8);
        commandCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 64), wxTE_MULTILINE | wxTE_READONLY);
        commandCtrl->Hide();
        debugSizer->Add(commandCtrl, 1, wxEXPAND);
        rootSizer->Add(debugSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

        // Log output
        logCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        rootSizer->Add(logCtrl, 1, wxEXPAND | wxALL, 8);

        panel->SetSizer(rootSizer);

        Bind(wxEVT_BUTTON, &ConverterFrame::OnConvert, this, convertButton->GetId());
        Bind(wxEVT_BUTTON, &ConverterFrame::OnStop, this, stopButton->GetId());
        Bind(wxEVT_FILEPICKER_CHANGED, &ConverterFrame::OnInputChanged, this, inputPicker->GetId());
        Bind(wxEVT_END_PROCESS, &ConverterFrame::OnProcessTerminated, this);
        Bind(wxEVT_CLOSE_WINDOW, &ConverterFrame::OnClose, this);

        Bind(wxEVT_RADIOBOX, &ConverterFrame::OnOutputKindChanged, this, outputKind->GetId());
        Bind(wxEVT_CHOICE, &ConverterFrame::OnAudioCodecChanged, this, audioCodecChoice->GetId());
        Bind(wxEVT_CHOICE, &ConverterFrame::OnVideoCodecOrSpeedChanged, this, videoCodecChoice->GetId());
        Bind(wxEVT_CHOICE, &ConverterFrame::OnVideoCodecOrSpeedChanged, this, speedChoice->GetId());
        Bind(wxEVT_CHOICE, &ConverterFrame::OnVideoCodecOrSpeedChanged, this, threadsChoice->GetId());
        Bind(wxEVT_CHECKBOX, &ConverterFrame::OnDebugToggle, this, debugToggle->GetId());

        RefreshAudioQualityChoices();
        UpdateControls();
    }

private:
    static wxString Quote(const wxString& path) {
        wxString quoted = path;
        quoted.Replace("\"", "\\\"");
        return wxString::Format("\"%s\"", quoted);
    }

    static bool NeedsQuoting(const wxString& s) {
        return s.Find(' ') != wxNOT_FOUND || s.Find('\t') != wxNOT_FOUND || s.Find('"') != wxNOT_FOUND;
    }

    static wxString BuildDisplayCommand(const wxArrayString& args) {
        wxString cmd = "ffmpeg";
        for (const auto& a : args) {
            cmd += " ";
            cmd += NeedsQuoting(a) ? Quote(a) : a;
        }
        return cmd;
    }

    static bool ParseTimeToSeconds(const wxString& text, double& secondsOut) {
        if (text.IsEmpty()) return false;
        // Accept formats: SS[.ms], MM:SS[.ms], HH:MM:SS[.ms]
        wxArrayString parts = wxSplit(text, ':');
        double total = 0.0;
        bool ok = false;
        auto toDouble = [&](const wxString& s, double& d){
            wxString tmp = s; tmp.Trim(true).Trim(false);
            double val = 0.0; ok = tmp.ToDouble(&val); d = val; return ok; };
        if (parts.size() == 1) {
            double s; if (!toDouble(parts[0], s)) return false; total = s;
        } else if (parts.size() == 2) {
            double m, s; if (!toDouble(parts[0], m)) return false; if (!toDouble(parts[1], s)) return false; total = m * 60.0 + s;
        } else if (parts.size() == 3) {
            double h, m, s; if (!toDouble(parts[0], h)) return false; if (!toDouble(parts[1], m)) return false; if (!toDouble(parts[2], s)) return false; total = h * 3600.0 + m * 60.0 + s;
        } else {
            return false;
        }
        secondsOut = total; return true;
    }

    static wxString FormatSeconds(double seconds) {
        // Use a compact seconds representation with up to 3 decimals
        return wxString::Format("%.3f", seconds);
    }

    void AppendLog(const wxString& text) {
        logCtrl->AppendText(text + "\n");
    }

    void OnInputChanged(wxFileDirPickerEvent&) {
        const wxString inputPath = inputPicker->GetPath();
        if (inputPath.IsEmpty()) return;
        wxFileName inFile(inputPath);
        wxString newName = inFile.GetName() + "-converted";
        inFile.SetName(newName);
        outputPicker->SetPath(inFile.GetFullPath());
    }

    wxString SelectedVideoCodec() const {
        if (!videoCodecChoice) return "copy";
        const wxString label = videoCodecChoice->GetStringSelection();
        if (label.StartsWith("H.264")) return "libx264";
        if (label.StartsWith("HEVC")) return "libx265";
        if (label.StartsWith("VP9")) return "libvpx-vp9";
        if (label.StartsWith("AV1")) return "libaom-av1";
        if (label.StartsWith("Copy")) return "copy";
        return "copy";
    }

    int MapSpeedToCpuUsed(const wxString& speed) const {
        if (speed == "ultrafast") return 8;
        if (speed == "superfast") return 6;
        if (speed == "veryfast") return 5;
        if (speed == "faster") return 4;
        if (speed == "fast") return 3;
        if (speed == "medium") return 2;
        if (speed == "slow") return 1;
        // slower/veryslow
        return 0;
    }

    void OnVideoCodecOrSpeedChanged(wxCommandEvent&) {
        UpdateControls();
    }

    void RefreshAudioQualityChoices() {
        if (!audioQualityChoice || !audioCodecChoice) return;
        audioQualityChoice->Clear();
        const wxString ac = audioCodecChoice->GetStringSelection();
        if (ac.StartsWith("MP3")) {
            audioQualityChoice->Append("128k");
            audioQualityChoice->Append("192k");
            audioQualityChoice->Append("256k");
            audioQualityChoice->Append("320k");
            audioQualityChoice->SetSelection(1);
        } else if (ac.StartsWith("Opus")) {
            audioQualityChoice->Append("64k");
            audioQualityChoice->Append("96k");
            audioQualityChoice->Append("128k");
            audioQualityChoice->Append("160k");
            audioQualityChoice->SetSelection(2);
        } else if (ac.StartsWith("FLAC")) {
            audioQualityChoice->Append("Level 0 (fast)");
            audioQualityChoice->Append("Level 5 (default)");
            audioQualityChoice->Append("Level 8 (max)");
            audioQualityChoice->SetSelection(1);
        } else if (ac.StartsWith("WAV")) {
            audioQualityChoice->Append("PCM 16-bit");
            audioQualityChoice->SetSelection(0);
        } else if (ac.StartsWith("Ogg Vorbis")) {
            audioQualityChoice->Append("q1");
            audioQualityChoice->Append("q2");
            audioQualityChoice->Append("q3");
            audioQualityChoice->Append("q4");
            audioQualityChoice->Append("q5");
            audioQualityChoice->Append("q6");
            audioQualityChoice->SetSelection(3);
        }
    }

    void OnOutputKindChanged(wxCommandEvent&) {
        UpdateControls();
    }

    void OnAudioCodecChanged(wxCommandEvent&) {
        RefreshAudioQualityChoices();
        UpdateControls();
    }

    void OnDebugToggle(wxCommandEvent&) {
        if (!commandCtrl || !debugToggle) return;
        if (debugToggle->IsChecked()) commandCtrl->Show(); else commandCtrl->Hide();
        Layout();
    }

    void OnConvert(wxCommandEvent&) {
        if (converter.IsRunning()) {
            wxMessageBox("A conversion is already in progress.", "Busy", wxICON_INFORMATION | wxOK, this);
            return;
        }
        const wxString inputPath = inputPicker->GetPath();
        wxString outputPath = outputPicker->GetPath();
        if (inputPath.IsEmpty()) { wxMessageBox("Please select an input file.", "Missing input", wxICON_WARNING | wxOK, this); return; }
        if (outputPath.IsEmpty()) { wxFileName inFile(inputPath); inFile.SetName(inFile.GetName()+"-converted"); outputPath = inFile.GetFullPath(); outputPicker->SetPath(outputPath); }

        MediaConverterSettings s;
        s.inputPath = inputPath;
        s.outputPath = outputPath;
        s.startStr = startTimeCtrl->GetValue();
        s.endStr = endTimeCtrl->GetValue();
        s.accurateTrim = accurateTrim && accurateTrim->IsChecked();
        s.audioOnly = (outputKind && outputKind->GetSelection()==1);
        s.videoCodec = SelectedVideoCodec();
        s.videoSpeedPreset = speedChoice ? speedChoice->GetStringSelection() : "veryfast";
        s.videoBitrate = videoBitrateCtrl ? videoBitrateCtrl->GetValue() : wxString();
        s.threads = threadsChoice ? threadsChoice->GetStringSelection() : wxString();
        s.audioCodecLabel = audioCodecChoice ? audioCodecChoice->GetStringSelection() : wxString();
        s.audioQuality = audioQualityChoice ? audioQualityChoice->GetStringSelection() : wxString();

        if (commandCtrl) {
            commandCtrl->ChangeValue(converter.BuildDisplayCommand(s));
            if (debugToggle && debugToggle->IsChecked()) { commandCtrl->Show(); Layout(); }
        }

        converter.SetOnLog([this](const wxString& msg){ AppendLog(msg); });
        converter.SetOnFinished([this](int code){ AppendLog("ffmpeg finished with code " + wxString::Format("%d", code)); isConverting=false; UpdateControls(); });

        AppendLog("Starting conversion...");
        isConverting = converter.Start(s);
        UpdateControls();
    }

    void OnStop(wxCommandEvent&) { converter.Stop(); }

    void OnProcessTerminated(wxProcessEvent& event) { event.Skip(); }

    void OnClose(wxCloseEvent& event) { if (converter.IsRunning()) converter.Stop(); Destroy(); }

    void StopConversion() {}

    void CleanupProcessState() { isConverting = false; }

    void UpdateControls() {
        if (convertButton) convertButton->Enable(!isConverting);
        if (stopButton) stopButton->Enable(isConverting);
        if (inputPicker) inputPicker->Enable(!isConverting);
        if (outputPicker) outputPicker->Enable(!isConverting);
        const bool isAudio = outputKind && outputKind->GetSelection() == 1;
        if (videoCodecChoice) videoCodecChoice->Enable(!isAudio && !isConverting);
        if (audioCodecChoice) audioCodecChoice->Enable(isAudio && !isConverting);
        if (audioQualityChoice) audioQualityChoice->Enable(isAudio && !isConverting);
        if (speedChoice) speedChoice->Enable(!isAudio && !isConverting);
        if (videoBitrateCtrl) videoBitrateCtrl->Enable(!isAudio && !isConverting && videoCodecChoice && videoCodecChoice->GetStringSelection() != "Copy");
        if (threadsChoice) threadsChoice->Enable(!isAudio && !isConverting);
    }

    void StartIOTimer() {}
    void StopIOTimer() {}
    void OnIOTimer(wxTimerEvent&) {}
    void ReadProcessOutput() {}

    wxFilePickerCtrl* inputPicker {nullptr};
    wxFilePickerCtrl* outputPicker {nullptr};
    wxTextCtrl* startTimeCtrl {nullptr};
    wxTextCtrl* endTimeCtrl {nullptr};
    wxRadioBox* outputKind {nullptr};
    wxChoice* videoCodecChoice {nullptr};
    wxChoice* audioCodecChoice {nullptr};
    wxChoice* audioQualityChoice {nullptr};
    wxChoice* speedChoice {nullptr};
    wxTextCtrl* videoBitrateCtrl {nullptr};
    wxCheckBox* accurateTrim {nullptr};
    wxChoice* threadsChoice {nullptr};
    wxButton* convertButton {nullptr};
    wxButton* stopButton {nullptr};
    wxTextCtrl* logCtrl {nullptr};
    wxCheckBox* debugToggle {nullptr};
    wxTextCtrl* commandCtrl {nullptr};

    // Backend
    MediaConverter converter;
    bool isConverting {false};
};


