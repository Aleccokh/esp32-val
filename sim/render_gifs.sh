#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [[ -d "/c/msys64/ucrt64/bin" ]]; then
  export PATH="/c/msys64/ucrt64/bin:$PATH"
fi

if [[ -x "./.venv/Scripts/python.exe" ]]; then
  PYTHON_CMD="./.venv/Scripts/python.exe"
elif command -v python3 >/dev/null 2>&1; then
  PYTHON_CMD="python3"
else
  PYTHON_CMD="python"
fi

PIO_CMD="${PIO_CMD:-platformio}"
PROGRAM_EXE=".pio/build/native/program.exe"

if [[ "$PIO_CMD" == "platformio" ]] && ! command -v platformio >/dev/null 2>&1; then
  if [[ -n "${USERPROFILE:-}" ]] && command -v cygpath >/dev/null 2>&1; then
    USERPROFILE_UNIX="$(cygpath -u "$USERPROFILE")"
    PIO_CANDIDATE="${USERPROFILE_UNIX}/.platformio/penv/Scripts/platformio.exe"
    if [[ -x "$PIO_CANDIDATE" ]]; then
      PIO_CMD="$PIO_CANDIDATE"
    fi
  elif [[ -x "$HOME/.platformio/penv/Scripts/platformio.exe" ]]; then
    PIO_CMD="$HOME/.platformio/penv/Scripts/platformio.exe"
  fi
fi

if ! "$PIO_CMD" --version >/dev/null 2>&1; then
  echo "PlatformIO CLI not found. Put it in PATH or set PIO_CMD=/full/path/to/platformio(.exe)."
  exit 1
fi

if [[ $# -gt 0 ]]; then
  ANIMS=("$@")
else
  ANIMS=(val1 val2 val3 val4 val5 chick1 mosq1)
fi

echo "Building native simulator..."
"$PIO_CMD" run -e native

if [[ ! -x "$PROGRAM_EXE" ]]; then
  echo "Simulator executable not found: $PROGRAM_EXE"
  exit 1
fi

mkdir -p sim_frames

for anim in "${ANIMS[@]}"; do
  echo "Rendering frames for $anim..."
  rm -f "sim_frames/${anim}_"*.ppm
  "$PROGRAM_EXE" "$anim"

  echo "Converting $anim frames to GIF..."
  "$PYTHON_CMD" sim/ppm_to_gif.py "$anim"
  echo "Created sim_preview_${anim}.gif"
  echo
 done

echo "All done. Generated GIF files:"
for anim in "${ANIMS[@]}"; do
  if [[ -f "sim_preview_${anim}.gif" ]]; then
    echo "- sim_preview_${anim}.gif"
  fi
done
