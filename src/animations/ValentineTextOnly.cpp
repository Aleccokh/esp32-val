#include "ValentineTextOnly.h"
#include <math.h>

using namespace ColorUtil;
using namespace Easing;

ValentineTextOnly valentineTextOnly;

namespace {
constexpr uint8_t GLYPH_I[6] = {
    0b1111,
    0b0110,
    0b0110,
    0b0110,
    0b0110,
    0b1111
};

constexpr uint8_t GLYPH_Y[6] = {
    0b1001,
    0b1001,
    0b0110,
    0b0110,
    0b0110,
    0b0110
};

constexpr uint8_t GLYPH_O[6] = {
    0b0110,
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b0110
};

constexpr uint8_t GLYPH_U[6] = {
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b0110
};
}

void ValentineTextOnly::init(IDisplay* display) {
    _display = display;
    _running = false;
    _heartScale = 1.0f;
    _heartBrightness = 1.0f;
}

void ValentineTextOnly::start() {
    if (_display == nullptr) return;
    _running = true;
}

void ValentineTextOnly::stop() {
    _running = false;
    if (_display) {
        _display->fillScreen(BG_BLACK);
    }
}

bool ValentineTextOnly::isRunning() const {
    return _running;
}

void ValentineTextOnly::update() {
    if (!_running) return;

    // Slow pulse with hold at initial position (loop timing tweak)
    static const float scaleKeys[] = {
        1.06f, 1.07f, 1.08f, 1.09f, 1.10f, 1.11f, 1.12f, 1.11f,
        1.10f, 1.09f, 1.08f, 1.07f, 1.06f, 1.07f, 1.08f, 1.09f
    };

    constexpr uint8_t keyCount = sizeof(scaleKeys) / sizeof(scaleKeys[0]);
    constexpr float holdMs = 1800.0f;   // stay at initial pose longer
    constexpr float pulseMs = 3800.0f;  // keep pulse smooth
    constexpr float cycleMs = holdMs + pulseMs;

    float timeInCycle = fmodf((float)millis(), cycleMs);

    if (timeInCycle < holdMs) {
        _heartScale = scaleKeys[0];
        _heartBrightness = 0.90f;
        return;
    }

    float pulsePhase = (timeInCycle - holdMs) / pulseMs; // 0..1
    float keyPos = pulsePhase * keyCount;
    int idx0 = (int)floorf(keyPos) % keyCount;
    int idx1 = (idx0 + 1) % keyCount;
    float localT = keyPos - floorf(keyPos);
    float easedT = easeInOutQuad(localT);

    _heartScale = scaleKeys[idx0] + (scaleKeys[idx1] - scaleKeys[idx0]) * easedT;
    _heartBrightness = 0.90f + 0.10f * ((_heartScale - 1.06f) / (1.12f - 1.06f));
}

void ValentineTextOnly::render() {
    if (!_running || _display == nullptr) return;

    _display->fillScreen(BG_BLACK);

    // 2px red border around the full text/heart composition
    const int16_t boxX = 2;
    const int16_t boxY = 18;
    const int16_t boxW = 60;
    const int16_t boxH = 29;
    uint16_t borderColor = HEART_RED;
    _display->drawRect(boxX, boxY, boxW, boxH, borderColor);
    _display->drawRect(boxX + 1, boxY + 1, boxW - 2, boxH - 2, borderColor);

    const uint8_t cell = 2;
    const int16_t y = 22;

    uint16_t textMain = SPARKLE;
    uint16_t textOutline = dimColor(SOFT_PINK, 0.95f);

    drawGlyph4x6(GLYPH_I, 6, y, textMain, textOutline, cell);
    drawHeart(24, y + 8, _heartScale, _heartBrightness);

    drawGlyph4x6(GLYPH_Y, 31, y, textMain, textOutline, cell);
    drawGlyph4x6(GLYPH_O, 41, y, textMain, textOutline, cell);
    drawGlyph4x6(GLYPH_U, 51, y, textMain, textOutline, cell);
}

void ValentineTextOnly::drawGlyph4x6(const uint8_t glyph[6], int16_t x, int16_t y, uint16_t mainColor, uint16_t outlineColor, uint8_t cell) {
    const int16_t glyphW = 4 * cell;
    const int16_t glyphH = 6 * cell;

    constexpr int16_t MAX_W = 16;
    constexpr int16_t MAX_H = 24;
    if (glyphW > MAX_W || glyphH > MAX_H) return;

    uint8_t mask[MAX_H][MAX_W] = {0};

    for (uint8_t row = 0; row < 6; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            if (!(glyph[row] & (1 << (3 - col)))) continue;
            for (uint8_t sy = 0; sy < cell; sy++) {
                for (uint8_t sx = 0; sx < cell; sx++) {
                    int16_t mx = col * cell + sx;
                    int16_t my = row * cell + sy;
                    mask[my][mx] = 1;
                }
            }
        }
    }

    for (int16_t py = 0; py < glyphH; py++) {
        for (int16_t px = 0; px < glyphW; px++) {
            if (mask[py][px]) {
                drawPixelSafe(x + px, y + py, mainColor);
            }
        }
    }

    for (int16_t py = 0; py < glyphH; py++) {
        for (int16_t px = 0; px < glyphW; px++) {
            if (mask[py][px]) continue;

            bool isEdge = false;
            for (int8_t oy = -1; oy <= 1 && !isEdge; oy++) {
                for (int8_t ox = -1; ox <= 1; ox++) {
                    if (ox == 0 && oy == 0) continue;
                    int16_t nx = px + ox;
                    int16_t ny = py + oy;
                    if (nx < 0 || ny < 0 || nx >= glyphW || ny >= glyphH) continue;
                    if (mask[ny][nx]) {
                        isEdge = true;
                        break;
                    }
                }
            }

            if (isEdge) {
                drawPixelSafe(x + px, y + py, outlineColor);
            }
        }
    }
}

void ValentineTextOnly::drawHeart(int16_t cx, int16_t cy, float scale, float brightness) {
    // Fuller "big" classic heart base shape (13x12)
    static const uint16_t HEART_ROWS[12] = {
        0b0001100011000,
        0b0011110111100,
        0b0111111111110,
        0b1111111111111,
        0b1111111111111,
        0b1111111111111,
        0b0111111111110,
        0b0011111111100,
        0b0001111111000,
        0b0000111110000,
        0b0000011100000,
        0b0000001000000
    };

    const int16_t baseW = 13;
    const int16_t baseH = 12;

    int16_t outW = max<int16_t>(8, (int16_t)roundf(baseW * scale));
    int16_t outH = max<int16_t>(7, (int16_t)roundf(baseH * scale));
    int16_t x0 = cx - outW / 2;
    int16_t y0 = cy - outH / 2;

    uint16_t cMain = dimColor(HEART_RED, brightness);
    uint16_t cWhite = dimColor(SPARKLE, brightness * 0.95f);

    // Scale by mapping output pixel back into base heart mask
    for (int16_t oy = 0; oy < outH; oy++) {
        int16_t srcY = min<int16_t>(baseH - 1, (int16_t)floorf(((oy + 0.5f) / outH) * baseH));
        uint16_t bits = HEART_ROWS[srcY];

        for (int16_t ox = 0; ox < outW; ox++) {
            int16_t srcX = min<int16_t>(baseW - 1, (int16_t)floorf(((ox + 0.5f) / outW) * baseW));
            if (!(bits & (1 << (12 - srcX)))) continue;
            drawPixelSafe(x0 + ox, y0 + oy, cMain);
        }
    }

    // Small white light inside the heart
    int16_t hx = x0 + max<int16_t>(1, (int16_t)(outW * 0.30f));
    int16_t hy = y0 + max<int16_t>(1, (int16_t)(outH * 0.25f));
    drawPixelSafe(hx, hy, cWhite);
    drawPixelSafe(hx + 1, hy, cWhite);
    drawPixelSafe(hx, hy + 1, dimColor(cWhite, 0.8f));
}

void ValentineTextOnly::drawPixelSafe(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < 64 && y >= 0 && y < 64) {
        _display->drawPixel(x, y, color);
    }
}
