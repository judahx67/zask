steps=(
  "2025-08-08T10:00:00Z|Set up CMake project with wxWidgets"
  "2025-08-08T10:20:00Z|Add input/output pickers and basic ffmpeg conversion"
  "2025-08-08T10:40:00Z|Add trimming"
  "2025-08-08T11:10:00Z|Add Stop"
  "2025-08-08T11:30:00Z|Show ffmpeg output in the log"
  "2025-08-08T12:00:00Z|Add accurate trim mode"
  "2025-08-08T12:30:00Z|Add encoding speed presets, video bitrate, and Vorbis quality"
  "2025-08-08T13:20:00Z|Add a debug toggle that shows the ffmpeg command"
  "2025-08-08T13:50:00Z|Refactor: separate UI and conversion logic"
  "2025-08-08T14:10:00Z|Add end-user Makefile and Windows toolchain configuration"
)
for entry in "${steps[@]}"; do
  IFS="|" read -r when msg <<< "$entry"
  GIT_AUTHOR_DATE="$when" GIT_COMMITTER_DATE="$when" git commit --allow-empty -m "$msg"
done

git add -A
git commit -m "Current working state"