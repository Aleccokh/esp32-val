#include "ValentineHeartbeat.h"
#include <math.h>

using namespace ColorUtil;
using namespace Easing;
using namespace HeartbeatConfig;

ValentineHeartbeat valentineHeartbeat;

namespace {
constexpr uint8_t STAR_COUNT = 24;

struct Star {
    float x;
    float y;
    float phase;
};

Star stars[STAR_COUNT];

constexpr uint16_t HEART_ROWS[15] = {
    0b000111000111000,
    0b001111101111100,
    0b011111111111110,
    0b111111111111111,
    0b111111111111111,
    0b111111111111111,
    0b011111111111110,
    0b001111111111100,
    0b000111111111000,
    0b000011111110000,
    0b000001111100000,
    0b000000111000000,
    0b000000010000000,
    0b000000010000000,
    0b000000000000000
};

void initStars() {
    for (uint8_t i = 0; i < STAR_COUNT; i++) {
        stars[i].x = random(64);
        stars[i].y = random(64);
        stars[i].phase = random(628) / 100.0f;
    }
}
}

void ValentineHeartbeat::init(IDisplay* display) {
    _display = display;
    _state = HeartbeatState::INACTIVE;
    _currentScale = 1.0f;
    _brightness = 0.0f;
    _glowRadius = 0.0f;
    _glowAlpha = 0.0f;
}

void ValentineHeartbeat::start() {
    if (_display == nullptr) return;

    _state = HeartbeatState::FADE_IN;
    _stateStartTime = millis();
    _animStartTime = millis();
    _currentScale = 1.0f;
    _brightness = 0.0f;
    _glowRadius = 0.0f;
    _glowAlpha = 0.0f;
    initStars();
}

void ValentineHeartbeat::stop() {
    _state = HeartbeatState::INACTIVE;
    if (_display) _display->fillScreen(BG_BLACK);
}

bool ValentineHeartbeat::isRunning() const {
    return _state != HeartbeatState::INACTIVE;
}

void ValentineHeartbeat::transitionTo(HeartbeatState newState) {
    _state = newState;
    _stateStartTime = millis();
}

void ValentineHeartbeat::update() {
    if (_display == nullptr || _state == HeartbeatState::INACTIVE) return;

    uint32_t now = millis();
    if (_state == HeartbeatState::FADE_IN) {
        float p = (float)(now - _stateStartTime) / 900.0f;
        if (p >= 1.0f) {
            _brightness = 1.0f;
            transitionTo(HeartbeatState::BREATHE);
        } else {
            _brightness = easeInOutCubic(constrain(p, 0.0f, 1.0f));
        }
        return;
    }

    float beatT = ((now - _animStartTime) % 1600UL) / 1600.0f;
    float pulse = heartbeatPulse(beatT);
    _currentScale = 0.95f + pulse * 0.42f;
    _glowRadius = 2.0f + 24.0f * beatT;
    _glowAlpha = 0.15f + 0.85f * pulse;
    _brightness = 0.72f + 0.28f * pulse;
}

void ValentineHeartbeat::updateFadeIn() {}
void ValentineHeartbeat::updateBreathe() {}
void ValentineHeartbeat::updateBeat(bool isUp, bool isFirst) { (void)isUp; (void)isFirst; }
void ValentineHeartbeat::updateGlow() {}
void ValentineHeartbeat::updateRest() {}

void ValentineHeartbeat::render() {
    if (_display == nullptr || _state == HeartbeatState::INACTIVE) return;

    // High-contrast black background
    _display->fillScreen(BG_BLACK);

    // Starfield twinkle
    uint32_t t = millis();
    for (uint8_t i = 0; i < STAR_COUNT; i++) {
        float tw = 0.35f + 0.65f * (0.5f + 0.5f * sinf(stars[i].phase + t * 0.0035f + i));
        drawPixelSafe((int16_t)stars[i].x, (int16_t)stars[i].y, dimColor(SPARKLE, tw * 0.55f));
    }

    // ECG line baseline + pulse
    int16_t baseY = 50;
    _display->drawFastHLine(0, baseY, 64, dimColor(SPARKLE, 0.25f));
    int16_t sweep = (t / 24) % 64;
    for (int16_t x = 0; x < 64; x++) {
        int16_t rel = x - sweep;
        int16_t y = baseY;
        if (rel >= -12 && rel < -8) y = baseY - 2;
        else if (rel >= -8 && rel < -6) y = baseY + 4;
        else if (rel >= -6 && rel < -4) y = baseY - 10;
        else if (rel >= -4 && rel < -2) y = baseY + 6;
        else if (rel >= -2 && rel < 2) y = baseY - 1;

        if (rel >= -12 && rel < 2) {
            drawPixelSafe(x, y, dimColor(rgb565(0, 255, 180), 0.9f));
            drawPixelSafe(x, y - 1, dimColor(rgb565(0, 180, 255), 0.4f));
        }
    }

    if (_glowAlpha > 0.03f) drawGlowRings();
    drawHeart(_currentScale, _brightness);
}

void ValentineHeartbeat::drawHeart(float scale, float brightness) {
    int16_t cx = HEART_CENTER_X;
    int16_t cy = HEART_CENTER_Y - 4;
    int16_t pixelSize = (scale > 1.15f) ? 3 : 2;

    uint16_t edge = dimColor(SPARKLE, 0.9f * brightness);
    uint16_t fillA = dimColor(HEART_RED, brightness);
    uint16_t fillB = dimColor(CORAL, brightness);

    for (int row = 0; row < 15; row++) {
        uint16_t bits = HEART_ROWS[row];
        for (int col = 0; col < 15; col++) {
            if (!(bits & (1 << (14 - col)))) continue;

            int16_t x = cx + (col - 7) * pixelSize;
            int16_t y = cy + (row - 7) * pixelSize;
            uint16_t body = (row < 5 || col < 5) ? fillB : fillA;

            for (int py = 0; py < pixelSize; py++) {
                for (int px = 0; px < pixelSize; px++) {
                    drawPixelSafe(x + px, y + py, body);
                }
            }

            // Crisp edge highlights
            if (row == 0 || row == 14 || col == 0 || col == 14 || !(bits & (1 << (14 - max(0, col - 1))))) {
                drawPixelSafe(x, y, edge);
            }
        }
    }
}

void ValentineHeartbeat::drawGlowRings() {
    int16_t cx = HEART_CENTER_X;
    int16_t cy = HEART_CENTER_Y - 4;

    for (int ring = 0; ring < 4; ring++) {
        float rad = 10.0f + _glowRadius + ring * 3.0f;
        float alpha = _glowAlpha * (1.0f - ring * 0.22f);
        if (alpha < 0.05f) continue;

        uint16_t c = dimColor(SOFT_PINK, alpha * 0.4f);
        for (float a = 0; a < 6.28318f; a += 0.09f) {
            int16_t x = cx + (int16_t)(cosf(a) * rad);
            int16_t y = cy + (int16_t)(sinf(a) * rad * 0.8f);
            drawPixelSafe(x, y, c);
        }
    }
}

void ValentineHeartbeat::drawPixelSafe(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < 64 && y >= 0 && y < 64) {
        _display->drawPixel(x, y, color);
    }
}
