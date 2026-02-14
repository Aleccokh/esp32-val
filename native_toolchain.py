Import("env")

import os
from pathlib import Path

# Prefer MSYS2 UCRT64 toolchain
candidates = [
    Path(r"C:/msys64/ucrt64/bin"),
    Path(r"C:/msys64/mingw64/bin"),
]

for candidate in candidates:
    if (candidate / "g++.exe").exists() and (candidate / "gcc.exe").exists():
        current_path = env["ENV"].get("PATH", "")
        env["ENV"]["PATH"] = str(candidate) + os.pathsep + current_path
        env["ENV"]["CC"] = str(candidate / "gcc.exe")
        env["ENV"]["CXX"] = str(candidate / "g++.exe")
        print(f"Using native toolchain from: {candidate}")
        break
else:
    print("Warning: No MSYS2 gcc/g++ found. Install mingw-w64-ucrt-x86_64-gcc in MSYS2.")
