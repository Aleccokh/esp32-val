import pathlib
import re
import sys

def resolve_project_dir() -> pathlib.Path:
    try:
        Import("env")
        return pathlib.Path(env["PROJECT_DIR"]).resolve()
    except Exception:
        pass

    if "__file__" in globals():
        return pathlib.Path(__file__).resolve().parent.parent

    return pathlib.Path.cwd().resolve()


PROJECT_DIR = resolve_project_dir()
ENV_FILE = PROJECT_DIR / ".env"
OUT_FILE = PROJECT_DIR / "include" / "Secrets.h"

REQUIRED_KEYS = [
    "WIFI_SSID",
    "WIFI_PASSWORD",
    "MQTT_HOST",
    "MQTT_PORT",
    "MQTT_USERNAME",
    "MQTT_PASSWORD",
    "GITHUB_OWNER",
    "GITHUB_REPO",
]

OPTIONAL_DEFAULTS = {
    "MQTT_TOPIC_TEXT": "esp32/rgb/text",
}


def parse_env_file(path: pathlib.Path):
    values = {}
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if line.startswith("export "):
            line = line[7:].strip()
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()
        if len(value) >= 2 and ((value[0] == '"' and value[-1] == '"') or (value[0] == "'" and value[-1] == "'")):
            value = value[1:-1]
        values[key] = value
    return values


def escape_cpp_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def validate(values):
    missing = [key for key in REQUIRED_KEYS if not values.get(key)]
    if missing:
        print("[secrets] Missing required keys in .env:", ", ".join(missing))
        print("[secrets] Copy .env.example to .env and fill in values.")
        sys.exit(1)

    if not re.fullmatch(r"\d+", values["MQTT_PORT"]):
        print("[secrets] MQTT_PORT must be an integer.")
        sys.exit(1)


def write_header(values):
    content = """#pragma once
#include <stdint.h>

static const char *WIFI_SSID = \"{wifi_ssid}\";
static const char *WIFI_PASSWORD = \"{wifi_password}\";
static const char *MQTT_HOST = \"{mqtt_host}\";
static const uint16_t MQTT_PORT = {mqtt_port};
static const char *MQTT_USERNAME = \"{mqtt_username}\";
static const char *MQTT_PASSWORD = \"{mqtt_password}\";
static const char *MQTT_TOPIC_TEXT = \"{mqtt_topic}\";
static const char *GITHUB_OWNER = \"{github_owner}\";
static const char *GITHUB_REPO = \"{github_repo}\";
""".format(
        wifi_ssid=escape_cpp_string(values["WIFI_SSID"]),
        wifi_password=escape_cpp_string(values["WIFI_PASSWORD"]),
        mqtt_host=escape_cpp_string(values["MQTT_HOST"]),
        mqtt_port=values["MQTT_PORT"],
        mqtt_username=escape_cpp_string(values["MQTT_USERNAME"]),
        mqtt_password=escape_cpp_string(values["MQTT_PASSWORD"]),
        mqtt_topic=escape_cpp_string(values["MQTT_TOPIC_TEXT"]),
        github_owner=escape_cpp_string(values["GITHUB_OWNER"]),
        github_repo=escape_cpp_string(values["GITHUB_REPO"]),
    )

    OUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    OUT_FILE.write_text(content, encoding="utf-8")
    print(f"[secrets] Generated {OUT_FILE}")


def main():
    if not ENV_FILE.exists():
        print("[secrets] .env file not found at project root.")
        print("[secrets] Copy .env.example to .env and fill in values.")
        sys.exit(1)

    values = parse_env_file(ENV_FILE)
    for key, default_value in OPTIONAL_DEFAULTS.items():
        if not values.get(key):
            values[key] = default_value

    validate(values)
    write_header(values)


if __name__ == "__main__":
    main()
