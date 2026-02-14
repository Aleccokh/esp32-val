# ESP32S3 OTA Setup (GitHub Releases)

This project is configured to perform OTA updates from GitHub Release assets.

## 1) Configure local secrets with `.env`

Copy `.env.example` to `.env` and fill in your real values.

The firmware now reads credentials via generated `include/Secrets.h` at build time.
This file is gitignored, so your secrets do not go to GitHub.

## 2) How OTA lookup works on the device

The ESP32 checks these URLs:

- `https://github.com/<owner>/<repo>/releases/latest/download/version.txt`
- `https://github.com/<owner>/<repo>/releases/latest/download/firmware.bin`

It compares `version.txt` with the local firmware version (`FW_VERSION`).

- Local version default is set in `platformio.ini` as `0.0.0`.
- Repository root includes `version.txt` (initial baseline value).
- CI build injects tag version (for example `v1.2.0`) into `FW_VERSION`.

## 3) Configure GitHub repository secrets

In GitHub, add these repository secrets:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `MQTT_HOST`
- `MQTT_PORT`
- `MQTT_USERNAME`
- `MQTT_PASSWORD`
- `MQTT_TOPIC_TEXT`

`GITHUB_OWNER` and `GITHUB_REPO` are taken automatically from the repository metadata in CI.

## 4) Create OTA releases from GitHub

A workflow is provided at `.github/workflows/ota-release.yml`.

It runs when you push a tag like:

```bash
git tag v1.0.0
git push origin v1.0.0
```

The workflow will:

1. Build firmware for `4d_systems_esp32s3_gen4_r8n16`
2. Upload release assets:
   - `firmware.bin`
   - `version.txt` (contains tag without `v`, e.g. `1.0.0`)

During release build, the workflow writes the tag value into `version.txt` and publishes it.

## 5) Trigger OTA on device

- Automatic periodic check every 6 hours.
- Manual serial command: `OTA:CHECK`

## 6) Notes

- Current OTA uses TLS with `setInsecure()` for convenience.
- For production, switch to certificate pinning / root CA validation.
