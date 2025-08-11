## zask - Media Converter

Simple cross‑platform media converter GUI built with wxWidgets that wraps FFmpeg.

### Features
- Input/output file pickers; output defaults to “-converted” in the same folder
- Trimming with -ss (start) and -to (end); accurate trim mode uses -t duration
- Video codecs: Copy, H.264 (libx264), HEVC (libx265), VP9 (libvpx‑vp9) [still very slow], AV1 (libaom‑av1)
- Video speed presets (preset/cpu-used), optional video bitrate, and threads selector
- Audio‑only mode: MP3, WAV (PCM), FLAC (levels), Ogg Vorbis (q1–q6), Opus
- Live FFmpeg output in a log box; Stop button to terminate; optional debug command view

## Dependencies (thank you!)
- wxWidgets 3.2+ (UI framework)
- FFmpeg (media processing)
- CMake 3.16+ and a C++17 compiler 
- Optional for cross‑compile: MinGW‑w64 toolchain 

## Install dependencies

### Arch 
```bash
sudo pacman -S cmake make gcc wxwidgets-gtk3 ffmpeg
# Optional (for Windows cross‑compile)
sudo pacman -S mingw-w64-gcc
```

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y cmake make g++ libwxgtk3.2-dev ffmpeg
```

### Windows (native build) via MSYS2 
*need instructions here*

## Build and run (Linux/macOS)
Convenience Makefile wraps CMake.
```bash
# Build
make

# Run
make run

# Clean
make clean

# Install (optional; change prefix via CMake if desired)
make install
```

- The output binary name includes platform/arch, e.g. `media-converter-linux-x86_64`.

## Windows cross‑compile from Arch (MinGW‑w64)
```bash
make windows
```

Notes:
- Ensure ffmpeg is in Windows PATH

## How to use
1) Select an input file.
2) (Optional) Set output file; otherwise it defaults to the same folder with “-converted”.
3) Choose Output: Video or Audio.
   - Video: select codec, preset/speed, (optional) bitrate, (optional) threads.
   - Audio: pick codec (MP3/WAV/FLAC/Vorbis/Opus) and quality/level.
4) (Optional) Enter Start (-ss) and End (-to) times. Enable “Accurate trim” for precise cuts; this may re‑encode.
5) Click Convert. Use Stop to abort. See FFmpeg logs in the log box.
6) Enable “Show command (debug)” to view the exact FFmpeg command.

### Time format examples
- `30` or `30.5` (seconds)
- `02:15` (MM:SS)
- `00:02:15.250` (HH:MM:SS.xxx)

## Packaging notes
- MP4 outputs add `-movflags +faststart` for better streaming.
- Accurate trim uses `-t` (duration) after `-i` to avoid tail remnants.



