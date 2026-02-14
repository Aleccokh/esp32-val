#include "Chick1Animation.h"
#include <math.h>

using namespace ColorUtil;

Chick1Animation chick1Animation;

namespace {
const uint16_t BG = BG_BLACK;
const uint16_t OUTLINE = rgb565(96, 34, 34);

const uint16_t CHICK_MAIN = rgb565(255, 235, 0);
const uint16_t CHICK_LIGHT = rgb565(255, 248, 130);
const uint16_t CHICK_SHADE = rgb565(255, 198, 40);
const uint16_t BEAK_MAIN = rgb565(255, 145, 22);
const uint16_t BEAK_DARK = rgb565(220, 92, 8);
const uint16_t WHITE = SPARKLE;
const uint16_t BLACK = BG_BLACK;
}

void Chick1Animation::init(IDisplay* display) {
    _display = display;
    _running = false;
    _blink = false;
    _bob = 0.0f;
}

void Chick1Animation::start() {
    if (_display == nullptr) return;
    _running = true;
}

void Chick1Animation::stop() {
    _running = false;
    if (_display) {
        _display->fillScreen(BG);
    }
}

bool Chick1Animation::isRunning() const {
    return _running;
}

void Chick1Animation::update() {
    if (!_running) return;

    _bob = 0.0f;

    const uint32_t blinkCycle = 4200;
    const uint32_t blinkMs = 90;
    _blink = (millis() % blinkCycle) < blinkMs;
}

void Chick1Animation::render() {
    if (!_running || _display == nullptr) return;

    _display->fillScreen(BG);

    int16_t cx = 32;
    int16_t cy = 32 + (int16_t)roundf(_bob);

    drawChickFront(cx, cy);
}

void Chick1Animation::fillEllipse(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t color) {
    if (rx <= 0 || ry <= 0) return;
    for (int16_t y = -ry; y <= ry; y++) {
        float fy = (float)y / (float)ry;
        float fx = sqrtf(fmaxf(0.0f, 1.0f - fy * fy));
        int16_t halfW = (int16_t)floorf((float)rx * fx);
        for (int16_t x = -halfW; x <= halfW; x++) {
            drawPixelSafe(cx + x, cy + y, color);
        }
    }
}

void Chick1Animation::drawChickFront(int16_t cx, int16_t cy) {
    // Simple cute front-facing chick body
    fillEllipse(cx, cy + 4, 12, 10, CHICK_MAIN);
    fillEllipse(cx, cy - 5, 10, 8, CHICK_MAIN);
    fillEllipse(cx, cy -10, 4, 2, CHICK_LIGHT);
    fillEllipse(cx - 5, cy - 3, 2, 1, CHICK_LIGHT);
    fillEllipse(cx + 5, cy - 3, 2, 1, CHICK_LIGHT);
    fillEllipse(cx, cy + 10, 7, 2, CHICK_SHADE);

    // Side wings
    fillEllipse(cx - 13, cy + 2, 5, 4, CHICK_MAIN);
    fillEllipse(cx + 13, cy + 2, 5, 4, CHICK_MAIN);
    fillEllipse(cx - 14, cy + 2, 3, 2, CHICK_LIGHT);
    fillEllipse(cx + 14, cy + 2, 3, 2, CHICK_LIGHT);

    // Strong eye isolation mask: wider black band to block neighboring yellow bleed
    _display->fillRect(cx - 11, cy - 8, 22, 7, BG_BLACK);

    if (_blink) {
        _display->drawFastHLine(cx - 6, cy - 5, 5, OUTLINE);
        _display->drawFastHLine(cx + 2, cy - 5, 5, OUTLINE);
    } else {
        // Open eyes: black eyes with tiny white highlight dot
        fillEllipse(cx - 4, cy - 5, 3, 2, BLACK);
        fillEllipse(cx + 4, cy - 5, 3, 2, BLACK);
        drawPixelSafe(cx - 6, cy - 6, WHITE);
        drawPixelSafe(cx + 2, cy - 6, WHITE);
    }

    // Clear beak (top + bottom)
    fillEllipse(cx, cy - 1, 4, 2, BEAK_MAIN);
    _display->drawFastHLine(cx - 3, cy - 1, 7, BEAK_DARK);
    _display->drawFastHLine(cx - 2, cy, 5, BEAK_DARK);

    // Legs + little feet
    _display->drawLine(cx - 3, cy + 13, cx - 3, cy + 17, BEAK_DARK);
    _display->drawLine(cx + 3, cy + 13, cx + 3, cy + 17, BEAK_DARK);
    _display->drawFastHLine(cx - 6, cy + 17, 5, BEAK_MAIN);
    _display->drawFastHLine(cx + 2, cy + 17, 5, BEAK_MAIN);
}

void Chick1Animation::drawPixelSafe(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < 64 && y >= 0 && y < 64) {
        _display->drawPixel(x, y, color);
    }
}
