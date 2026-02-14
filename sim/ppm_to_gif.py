from pathlib import Path
import sys
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
frames_dir = ROOT / "sim_frames"

prefix = "val4"
if len(sys.argv) > 1 and sys.argv[1].strip():
    prefix = sys.argv[1].strip().lower()

out_gif = ROOT / f"sim_preview_{prefix}.gif"

if not frames_dir.exists():
    raise SystemExit(f"Frames folder not found: {frames_dir}")

frame_paths = sorted(frames_dir.glob(f"{prefix}_*.ppm"))
if not frame_paths:
    raise SystemExit(f"No PPM frames found in {frames_dir} for prefix '{prefix}'")

frames = [Image.open(p).convert("P", palette=Image.ADAPTIVE) for p in frame_paths]

# 12 fps from every-3-frame capture at ~30 fps simulation
frame_duration_ms = 83

frames[0].save(
    out_gif,
    save_all=True,
    append_images=frames[1:],
    duration=frame_duration_ms,
    loop=0,
    optimize=False,
)

print(f"GIF created: {out_gif}")
print(f"Frames used: {len(frame_paths)}")
