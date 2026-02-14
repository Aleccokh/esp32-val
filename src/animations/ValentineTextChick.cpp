#include "ValentineTextChick.h"
#include <math.h>

using namespace ColorUtil;
using namespace Easing;

ValentineTextChick valentineTextChick;

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

void ValentineTextChick::init(IDisplay* display) {
    _display = display;
    _running = false;
    _heartScale = 1.0f;
    _heartBrightness = 1.0f;
    _chickX = 6.0f;
    _facingRight = true;
    _stepA = true;
    _chickBob = 0.0f;
}

void ValentineTextChick::start() {
    if (_display == nullptr) return;
    _running = true;
}

void ValentineTextChick::stop() {
    _running = false;
    if (_display) {
        _display->fillScreen(BG_BLACK);
    }
}

bool ValentineTextChick::isRunning() const {
    return _running;
}

void ValentineTextChick::update() {
    if (!_running) return;

    static const float scaleKeys[] = {
        1.06f, 1.07f, 1.08f, 1.09f, 1.10f, 1.11f, 1.12f, 1.11f,
        1.10f, 1.09f, 1.08f, 1.07f, 1.06f, 1.07f, 1.08f, 1.09f
    };

    constexpr uint8_t keyCount = sizeof(scaleKeys) / sizeof(scaleKeys[0]);
    constexpr float holdMs = 1800.0f;
    constexpr float pulseMs = 3800.0f;
    constexpr float cycleMs = holdMs + pulseMs;

    float timeInCycle = fmodf((float)millis(), cycleMs);

    if (timeInCycle < holdMs) {
        _heartScale = scaleKeys[0];
        _heartBrightness = 0.90f;
    } else {
        float pulsePhase = (timeInCycle - holdMs) / pulseMs;
        float keyPos = pulsePhase * keyCount;
        int idx0 = (int)floorf(keyPos) % keyCount;
        int idx1 = (idx0 + 1) % keyCount;
        float localT = keyPos - floorf(keyPos);
        float easedT = easeInOutQuad(localT);

        _heartScale = scaleKeys[idx0] + (scaleKeys[idx1] - scaleKeys[idx0]) * easedT;
        _heartBrightness = 0.90f + 0.10f * ((_heartScale - 1.06f) / (1.12f - 1.06f));
    }

    constexpr float chickMinX = 6.0f;
    constexpr float chickMaxX = 52.0f;
    constexpr float walkOneWayMs = 5000.0f;
    constexpr float turnPauseMs = 280.0f;
    constexpr float walkCycleMs = walkOneWayMs + turnPauseMs + walkOneWayMs + turnPauseMs;

    float t = fmodf((float)millis(), walkCycleMs);

    if (t < walkOneWayMs) {
        float p = easeInOutQuad(t / walkOneWayMs);
        _chickX = chickMinX + (chickMaxX - chickMinX) * p;
        _facingRight = true;
    } else if (t < walkOneWayMs + turnPauseMs) {
        _chickX = chickMaxX;
        _facingRight = false;
    } else if (t < walkOneWayMs + turnPauseMs + walkOneWayMs) {
        float t2 = t - walkOneWayMs - turnPauseMs;
        float p = easeInOutQuad(t2 / walkOneWayMs);
        _chickX = chickMaxX - (chickMaxX - chickMinX) * p;
        _facingRight = false;
    } else {
        _chickX = chickMinX;
        _facingRight = true;
    }

    float walkT = fmodf((float)millis(), 360.0f) / 360.0f;
    _stepA = walkT < 0.5f;
    _chickBob = sinf(walkT * 6.2831853f) * 0.35f;
}

void ValentineTextChick::render() {
    if (!_running || _display == nullptr) return;

    _display->fillScreen(BG_BLACK);

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

    drawChick((int16_t)roundf(_chickX), 12 + (int16_t)roundf(_chickBob), _facingRight, _stepA);
}

void ValentineTextChick::drawGlyph4x6(const uint8_t glyph[6], int16_t x, int16_t y, uint16_t mainColor, uint16_t outlineColor, uint8_t cell) {
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

void ValentineTextChick::drawHeart(int16_t cx, int16_t cy, float scale, float brightness) {
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

    for (int16_t oy = 0; oy < outH; oy++) {
        int16_t srcY = min<int16_t>(baseH - 1, (int16_t)floorf(((oy + 0.5f) / outH) * baseH));
        uint16_t bits = HEART_ROWS[srcY];

        for (int16_t ox = 0; ox < outW; ox++) {
            int16_t srcX = min<int16_t>(baseW - 1, (int16_t)floorf(((ox + 0.5f) / outW) * baseW));
            if (!(bits & (1 << (12 - srcX)))) continue;
            drawPixelSafe(x0 + ox, y0 + oy, cMain);
        }
    }

    int16_t hx = x0 + max<int16_t>(1, (int16_t)(outW * 0.30f));
    int16_t hy = y0 + max<int16_t>(1, (int16_t)(outH * 0.25f));
    drawPixelSafe(hx, hy, cWhite);
    drawPixelSafe(hx + 1, hy, cWhite);
    drawPixelSafe(hx, hy + 1, dimColor(cWhite, 0.8f));
}

void ValentineTextChick::drawChick(int16_t x, int16_t y, bool facingRight, bool stepA) {
    uint16_t yellow = rgb565(255, 255, 0);
    uint16_t yellowShade = rgb565(255, 228, 0);
    uint16_t orange = rgb565(255, 150, 40);
    uint16_t white = SPARKLE;
    uint16_t black = BG_BLACK;

    for (int8_t oy = 0; oy < 5; oy++) {
        for (int8_t ox = 0; ox < 7; ox++) {
            int16_t dx = ox - 3;
            int16_t dy = oy - 2;
            if (dx * dx + dy * dy <= 10) {
                drawPixelSafe(x + ox, y + oy, yellow);
            }
        }
    }

    drawPixelSafe(x + 2, y + 3, yellowShade);
    drawPixelSafe(x + 3, y + 3, yellowShade);
    drawPixelSafe(x + 4, y + 3, yellowShade);

    int16_t eyeX = facingRight ? x + 4 : x + 2;
    drawPixelSafe(eyeX, y + 1, black);
    drawPixelSafe(eyeX - (facingRight ? 1 : -1), y + 1, white);

    if (facingRight) {
        drawPixelSafe(x + 6, y + 2, orange);
        drawPixelSafe(x + 7, y + 2, orange);
        drawPixelSafe(x + 6, y + 3, orange);
    } else {
        drawPixelSafe(x + 0, y + 2, orange);
        drawPixelSafe(x - 1, y + 2, orange);
        drawPixelSafe(x + 0, y + 3, orange);
    }

    int16_t legY = y + 5;
    if (stepA) {
        drawPixelSafe(x + 2, legY, orange);
        drawPixelSafe(x + 4, legY + 1, orange);
    } else {
        drawPixelSafe(x + 2, legY + 1, orange);
        drawPixelSafe(x + 4, legY, orange);
    }
}

void ValentineTextChick::drawPixelSafe(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < 64 && y >= 0 && y < 64) {
        _display->drawPixel(x, y, color);
    }
}
