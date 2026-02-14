#include "Mosq1Animation.h"
#include <math.h>

using namespace ColorUtil;

Mosq1Animation mosq1Animation;

namespace {
const uint16_t SKY_BLACK = rgb565(0, 0, 0);
const uint16_t MOSQUE_WHITE = rgb565(248, 248, 255);
const uint16_t MOSQUE_BLUE = rgb565(40, 120, 255);
const uint16_t MOSQUE_BLUE_DARK = rgb565(18, 64, 168);
const uint16_t STAR_WHITE = rgb565(255, 255, 255);
const uint16_t STAR_SOFT = rgb565(170, 210, 255);
const uint16_t MOON_WHITE = rgb565(245, 248, 255);
}

void Mosq1Animation::init(IDisplay* display) {
    _display = display;
    _running = false;
}

void Mosq1Animation::start() {
    if (_display == nullptr) return;
    _running = true;
}

void Mosq1Animation::stop() {
    _running = false;
    if (_display) {
        _display->fillScreen(SKY_BLACK);
    }
}

bool Mosq1Animation::isRunning() const {
    return _running;
}

void Mosq1Animation::update() {
    if (!_running) return;
}

void Mosq1Animation::render() {
    if (!_running || _display == nullptr) return;

    _display->fillScreen(SKY_BLACK);

    uint32_t t = millis();

    // One sophisticated shooting star: pause + constant-speed arc sweep
    constexpr uint32_t cycleMs = 3600;
    constexpr uint32_t activeMs = 1450;
    uint32_t local = t % cycleMs;
    if (local < activeMs) {
        float p = (float)local / (float)activeMs;

        // Arc trajectory (high-left -> slight rise -> lower-right), constant speed in parameter space
        int16_t sx = (int16_t)(-10 + p * 86.0f);
        float yArc = 6.0f + p * 11.0f - 5.0f * sinf(p * 3.14159f);
        int16_t sy = (int16_t)yArc;
        drawShootingStar(sx, sy);
    }

    drawMosque();

    // Ramadan text under mosque
    _display->setTextSize(1);
    _display->setTextWrap(false);
    _display->setTextColor(rgb565(220, 235, 255));
    _display->setCursor(11, 54);
    _display->print("RAMADAN");
}

void Mosq1Animation::drawMosque() {
    // Base platform
    _display->fillRect(8, 47, 48, 4, MOSQUE_BLUE_DARK);

    // Main body
    _display->fillRect(16, 34, 32, 13, MOSQUE_WHITE);
    _display->drawRect(16, 34, 32, 13, MOSQUE_BLUE);

    // Central dome
    fillCircle(32, 32, 8, MOSQUE_BLUE);
    fillCircle(32, 32, 5, MOSQUE_WHITE);
    drawPixelSafe(32, 23, MOSQUE_BLUE_DARK);

    // Side mini domes
    fillCircle(22, 35, 3, MOSQUE_BLUE);
    fillCircle(42, 35, 3, MOSQUE_BLUE);

    // Pillars / minarets
    _display->fillRect(10, 24, 6, 23, MOSQUE_WHITE);
    _display->drawRect(10, 24, 6, 23, MOSQUE_BLUE);
    _display->fillRect(48, 24, 6, 23, MOSQUE_WHITE);
    _display->drawRect(48, 24, 6, 23, MOSQUE_BLUE);

    // Minaret caps
    fillCircle(13, 23, 3, MOSQUE_BLUE);
    fillCircle(51, 23, 3, MOSQUE_BLUE);

    // Arched door + windows accents
    _display->fillRect(29, 39, 6, 8, MOSQUE_BLUE_DARK);
    _display->fillRect(20, 39, 4, 5, MOSQUE_BLUE);
    _display->fillRect(40, 39, 4, 5, MOSQUE_BLUE);

    // Half moon (crescent) on right pillar top
    drawCrescent(51, 18, MOON_WHITE);
}

void Mosq1Animation::drawStar(int16_t x, int16_t y, uint16_t color, bool sparkle) {
    drawPixelSafe(x, y, color);
    drawPixelSafe(x - 1, y, color);
    drawPixelSafe(x + 1, y, color);
    drawPixelSafe(x, y - 1, color);
    drawPixelSafe(x, y + 1, color);

    if (sparkle) {
        drawPixelSafe(x - 1, y - 1, color);
        drawPixelSafe(x + 1, y - 1, color);
        drawPixelSafe(x - 1, y + 1, color);
        drawPixelSafe(x + 1, y + 1, color);
    }
}

void Mosq1Animation::drawShootingStar(int16_t x, int16_t y) {
    uint32_t t = millis();

    // Pulsing head brightness gives a sparkling comet core
    float pulse = 0.82f + 0.18f * (0.5f + 0.5f * sinf((float)t * 0.025f));
    uint16_t headCore = dimColor(STAR_WHITE, pulse);
    uint16_t headGlow = dimColor(STAR_SOFT, 0.72f + 0.10f * sinf((float)t * 0.018f));

    // Comet head + glow halo
    drawPixelSafe(x, y, headCore);
    drawPixelSafe(x - 1, y, headGlow);
    drawPixelSafe(x + 1, y, headGlow);
    drawPixelSafe(x, y - 1, headGlow);
    drawPixelSafe(x, y + 1, headGlow);
    drawPixelSafe(x - 1, y - 1, dimColor(headGlow, 0.70f));
    drawPixelSafe(x + 1, y - 1, dimColor(headGlow, 0.60f));
    drawPixelSafe(x - 1, y + 1, dimColor(headGlow, 0.60f));
    drawPixelSafe(x + 1, y + 1, dimColor(headGlow, 0.45f));

    // Trail follows the same arc path by sampling earlier positions in time
    constexpr uint32_t cycleMs = 3600;
    constexpr uint32_t activeMs = 1450;
    float pHead = (float)(millis() % cycleMs) / (float)activeMs;
    if (pHead > 1.0f) pHead = 1.0f;

    for (int16_t i = 1; i <= 14; i++) {
        float back = i * 0.028f;
        float pt = pHead - back;
        if (pt < 0.0f) break;

        int16_t tx = (int16_t)(-10 + pt * 86.0f);
        int16_t ty = (int16_t)(6.0f + pt * 11.0f - 5.0f * sinf(pt * 3.14159f));

        float f = 1.0f - ((float)i / 14.0f);
        uint16_t base = (i < 4) ? STAR_WHITE : STAR_SOFT;
        uint16_t c = dimColor(base, 0.22f + 0.78f * f);

        drawPixelSafe(tx, ty, c);

        if ((i % 2) == 0 && i < 12) {
            drawPixelSafe(tx - 1, ty, dimColor(c, 0.65f));
        }

        if ((i % 4) == 0) {
            drawPixelSafe(tx - 1, ty + 1, dimColor(STAR_SOFT, 0.30f * f));
        }
    }
}

void Mosq1Animation::drawCrescent(int16_t cx, int16_t cy, uint16_t color) {
    fillCircle(cx, cy, 3, color);
    fillCircle(cx + 1, cy, 2, SKY_BLACK);
}

void Mosq1Animation::fillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    for (int16_t y = -r; y <= r; y++) {
        int16_t dx = (int16_t)sqrtf((float)(r * r - y * y));
        _display->drawFastHLine(cx - dx, cy + y, dx * 2 + 1, color);
    }
}

void Mosq1Animation::drawPixelSafe(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < 64 && y >= 0 && y < 64) {
        _display->drawPixel(x, y, color);
    }
}
