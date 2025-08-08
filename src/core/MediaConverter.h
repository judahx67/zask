#pragma once

#include <wx/wx.h>
#include <wx/process.h>
#include <wx/filename.h>
#include <wx/timer.h>
#include <functional>
#include <vector>

struct MediaConverterSettings {
    // IO
    wxString inputPath;
    wxString outputPath;

    // Trim
    wxString startStr;  // may be empty
    wxString endStr;    // may be empty
    bool accurateTrim {true};

    // Mode: false = Video, true = Audio
    bool audioOnly {false};

    // Video
    wxString videoCodec;       // e.g. copy, libx264, libx265, libvpx-vp9, libaom-av1
    wxString videoSpeedPreset; // e.g. veryfast
    wxString videoBitrate;     // e.g. 2000k (empty = auto)
    wxString threads;          // "Auto" or number

    // Audio
    wxString audioCodecLabel;  // UI label used to decide mapping
    wxString audioQuality;     // bitrate or q-value descriptor based on codec
};

class MediaConverter : public wxEvtHandler {
public:
    MediaConverter();
    ~MediaConverter() override;

    // Callbacks
    void SetOnLog(std::function<void(const wxString&)> cb) { onLog = std::move(cb); }
    void SetOnFinished(std::function<void(int)> cb) { onFinished = std::move(cb); }

    bool Start(const MediaConverterSettings& settings);
    void Stop();
    bool IsRunning() const { return isRunning; }

    wxString BuildDisplayCommand(const MediaConverterSettings& settings) const;

private:
    static bool ParseTimeToSeconds(const wxString& text, double& secondsOut);
    static wxString FormatSeconds(double seconds);
    static int MapSpeedToCpuUsed(const wxString& speed);
    static bool NeedsQuoting(const wxString& s);
    static wxString Quote(const wxString& path);

    void BuildArgs(const MediaConverterSettings& s, wxArrayString& args) const;

    void OnIOTimer(wxTimerEvent&);
    void OnProcessTerminated(wxProcessEvent& evt);
    void ReadProcessOutput();

private:
    std::function<void(const wxString&)> onLog;
    std::function<void(int)> onFinished;

    wxProcess* process {nullptr};
    long pid {0};
    bool isRunning {false};
    wxTimer ioTimer;

    wxDECLARE_EVENT_TABLE();
};


