#include "MediaConverter.h"

wxBEGIN_EVENT_TABLE(MediaConverter, wxEvtHandler)
    EVT_TIMER(wxID_ANY, MediaConverter::OnIOTimer)
    EVT_END_PROCESS(wxID_ANY, MediaConverter::OnProcessTerminated)
wxEND_EVENT_TABLE()

MediaConverter::MediaConverter() : ioTimer(this) {}

MediaConverter::~MediaConverter() {
    Stop();
}

static void Append(wxArrayString& arr, const wxString& a, const wxString& b) {
    arr.Add(a); arr.Add(b);
}

bool MediaConverter::ParseTimeToSeconds(const wxString& text, double& secondsOut) {
    if (text.IsEmpty()) return false;
    wxArrayString parts = wxSplit(text, ':');
    auto toDouble = [](const wxString& s, double& d){ wxString t=s; t.Trim(true).Trim(false); return t.ToDouble(&d); };
    double h=0,m=0,s=0;
    if (parts.size()==1) { if(!toDouble(parts[0], s)) return false; }
    else if (parts.size()==2) { if(!toDouble(parts[0], m)||!toDouble(parts[1], s)) return false; }
    else if (parts.size()==3) { if(!toDouble(parts[0], h)||!toDouble(parts[1], m)||!toDouble(parts[2], s)) return false; }
    else return false;
    secondsOut = h*3600 + m*60 + s; return true;
}

wxString MediaConverter::FormatSeconds(double seconds) {
    return wxString::Format("%.3f", seconds);
}

int MediaConverter::MapSpeedToCpuUsed(const wxString& speed) {
    if (speed=="ultrafast") return 8;
    if (speed=="superfast") return 6;
    if (speed=="veryfast") return 5;
    if (speed=="faster") return 4;
    if (speed=="fast") return 3;
    if (speed=="medium") return 2;
    if (speed=="slow") return 1;
    return 0;
}

bool MediaConverter::NeedsQuoting(const wxString& s) {
    return s.Find(' ')!=wxNOT_FOUND || s.Find('\t')!=wxNOT_FOUND || s.Find('"')!=wxNOT_FOUND;
}

wxString MediaConverter::Quote(const wxString& path) {
    wxString q = path; q.Replace("\"","\\\""); return wxString::Format("\"%s\"", q);
}

wxString MediaConverter::BuildDisplayCommand(const MediaConverterSettings& s) const {
    wxArrayString args; BuildArgs(s, args);
    wxString cmd = "ffmpeg";
    for (const auto& a: args) { cmd += " "; cmd += NeedsQuoting(a)? Quote(a): a; }
    return cmd;
}

void MediaConverter::BuildArgs(const MediaConverterSettings& s, wxArrayString& args) const {
    args.Clear();
    args.Add("-y");
    double startSecs=0, endSecs=0; bool hasStart=ParseTimeToSeconds(s.startStr,startSecs); bool hasEnd=ParseTimeToSeconds(s.endStr,endSecs);
    bool hasTrim = hasStart || hasEnd; bool accurate = s.accurateTrim && hasTrim; double dur=0; bool hasDur=false;
    if (hasStart && hasEnd && endSecs>startSecs) { dur = endSecs-startSecs; hasDur=true; }
    if (!accurate && hasStart) Append(args, "-ss", s.startStr);
    args.Add("-i"); args.Add(s.inputPath);
    if (accurate && hasStart) Append(args, "-ss", s.startStr);
    if (hasDur) Append(args, "-t", FormatSeconds(dur)); else if (!accurate && hasEnd) Append(args, "-to", s.endStr);

    if (s.audioOnly) {
        args.Add("-vn");
        const wxString& ac = s.audioCodecLabel;
        if (ac.StartsWith("MP3")) { Append(args, "-c:a", "libmp3lame"); Append(args, "-b:a", s.audioQuality); }
        else if (ac.StartsWith("WAV")) { Append(args, "-c:a", "pcm_s16le"); }
        else if (ac.StartsWith("FLAC")) { Append(args, "-c:a", "flac"); Append(args, "-compression_level", s.audioQuality); }
        else if (ac.StartsWith("Opus")) { Append(args, "-c:a", "libopus"); Append(args, "-b:a", s.audioQuality); }
        else if (ac.StartsWith("Ogg Vorbis")) { Append(args, "-c:a", "libvorbis"); Append(args, "-q:a", s.audioQuality); }
    } else {
        wxString vcodec = s.videoCodec;
        if (accurate && vcodec=="copy") vcodec = "libx264";
        Append(args, "-c:v", vcodec);
        if (vcodec=="libx264" || vcodec=="libx265") Append(args, "-preset", s.videoSpeedPreset);
        else if (vcodec=="libvpx-vp9") { Append(args, "-deadline", "good"); Append(args, "-cpu-used", wxString::Format("%d", MapSpeedToCpuUsed(s.videoSpeedPreset))); }
        else if (vcodec=="libaom-av1") { Append(args, "-cpu-used", wxString::Format("%d", MapSpeedToCpuUsed(s.videoSpeedPreset))); }
        if (!s.videoBitrate.IsEmpty() && vcodec!="copy") Append(args, "-b:v", s.videoBitrate);
        if (!s.threads.IsEmpty() && !s.threads.IsSameAs("Auto", false) && vcodec!="copy") Append(args, "-threads", s.threads);
        Append(args, "-c:a", "copy");
        wxFileName outFn(s.outputPath); if (outFn.GetExt().IsSameAs("mp4", false)) { Append(args, "-movflags", "+faststart"); }
    }
    args.Add(s.outputPath);
}

bool MediaConverter::Start(const MediaConverterSettings& settings) {
    if (isRunning) return false;
    if (settings.inputPath.IsEmpty() || settings.outputPath.IsEmpty()) return false;

    wxArrayString args; BuildArgs(settings, args);
    std::vector<const wxChar*> argv; argv.reserve(args.size()+2); argv.push_back(wxT("ffmpeg"));
    for (auto& a: args) argv.push_back(a.c_str()); argv.push_back(nullptr);

    process = new wxProcess(this);
    process->Redirect();
    pid = wxExecute(argv.data(), wxEXEC_ASYNC | wxEXEC_HIDE_CONSOLE | wxEXEC_MAKE_GROUP_LEADER, process);
    if (pid == 0) {
        delete process; process=nullptr; pid=0; isRunning=false; return false;
    }
    isRunning = true;
    ioTimer.Start(100);
    if (onLog) onLog("ffmpeg started (PID: " + wxString::Format("%ld", pid) + ")");
    return true;
}

void MediaConverter::Stop() {
    if (!isRunning || pid==0) return;
    wxKillError err; int rc = wxKill(pid, wxSIGTERM, &err, wxKILL_CHILDREN);
    if (rc!=0 || err!=wxKILL_OK) { wxKill(pid, wxSIGKILL, &err, wxKILL_CHILDREN); }
    isRunning = false;
}

void MediaConverter::OnIOTimer(wxTimerEvent&) {
    ReadProcessOutput();
}

void MediaConverter::ReadProcessOutput() {
    if (!process) return;
    auto readStream = [&](wxInputStream* s){
        if (!s) return;
        while (s->CanRead()) {
            char buffer[4096]; s->Read(buffer, sizeof(buffer)); size_t n = s->LastRead();
            if (n==0) break; wxString chunk = wxString::FromUTF8(buffer, static_cast<int>(n));
            if (onLog) onLog(chunk);
        }
    };
    readStream(process->GetInputStream());
    readStream(process->GetErrorStream());
}

void MediaConverter::OnProcessTerminated(wxProcessEvent& evt) {
    if (evt.GetPid()!=pid && pid!=0) { evt.Skip(); return; }
    ioTimer.Stop();
    ReadProcessOutput();
    int code = evt.GetExitCode();
    if (onFinished) onFinished(code);
    if (process) { delete process; process=nullptr; }
    pid=0; isRunning=false;
}


