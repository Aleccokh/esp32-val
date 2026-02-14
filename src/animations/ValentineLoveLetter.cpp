#include "ValentineLoveLetter.h"
#include <math.h>

using namespace ColorUtil;
using namespace Easing;

ValentineLoveLetter valentineLoveLetter;

namespace {
constexpr uint8_t FONT_HEIGHT = 6;
constexpr uint8_t CELL = 2;

constexpr uint8_t GLYPH_I[FONT_HEIGHT] = {
    0b1111,
    0b0110,
    0b0110,
    0b0110,
    0b0110,
    0b1111
};

constexpr uint8_t GLYPH_Y[FONT_HEIGHT] = {
    0b1001,
    0b1001,
    0b0110,
    0b0110,
    0b0110,
    0b0110
};

constexpr uint8_t GLYPH_O[FONT_HEIGHT] = {
    0b0110,
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b0110
};

constexpr uint8_t GLYPH_U[FONT_HEIGHT] = {
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b1001,
    0b0110
};

inline uint16_t hsvTo565(uint8_t h, uint8_t s, uint8_t v) {
    if (s == 0) {
        return rgb565(v, v, v);
    }

    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;

    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    uint8_t r, g, b;
    switch (region) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }

    return rgb565(r, g, b);
}

void drawDisc(IDisplay* display, int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    for (int16_t y = -r; y <= r; y++) {
        int16_t dx = (int16_t)sqrtf((float)(r * r - y * y));
        display->drawFastHLine(cx - dx, cy + y, dx * 2 + 1, color);
    }
}
}

void ValentineLoveLetter::init(IDisplay* display) {
    _display = display;
    _state = LoveLetterState::INACTIVE;
    _stateStartTime = 0;

    _envBrightness = 1.0f;
    _flapAngle = 0.0f;
    _sealGlow = 0.0f;

    _heartY = 31.0f;
    _heartScale = 1.0f;
    _heartBrightness = 1.0f;

    for (int i = 0; i < LoveLetterConfig::CONFETTI_COUNT; i++) {
        _confetti[i].active = false;
    }
}

void ValentineLoveLetter::start() {
    if (_display == nullptr) return;

    _state = LoveLetterState::SEALED;
    _stateStartTime = millis();
    _heartBrightness = 1.0f;
    _heartY = 31.0f;
    _heartScale = 1.0f;
}

void ValentineLoveLetter::stop() {
    _state = LoveLetterState::INACTIVE;
    if (_display) {
        _display->fillScreen(BG_BLACK);
    }
}

bool ValentineLoveLetter::isRunning() const {
    return _state != LoveLetterState::INACTIVE;
}

void ValentineLoveLetter::transitionTo(LoveLetterState newState) {
    _state = newState;
    _stateStartTime = millis();
}

void ValentineLoveLetter::update() {
    if (_display == nullptr || _state == LoveLetterState::INACTIVE) return;

    uint32_t now = millis();
    float beatT = (now % 1700UL) / 1700.0f;
    float beat = heartbeatPulse(beatT);

    _heartScale = 0.95f + 0.45f * beat;
    _heartBrightness = 0.9f + 0.1f * beat;
    _heartY = 31.0f;
}

void ValentineLoveLetter::updateSealed() {}
void ValentineLoveLetter::updateGlow() {}
void ValentineLoveLetter::updateOpen() {}
void ValentineLoveLetter::updateReveal() {}
void ValentineLoveLetter::updateRise() {}
void ValentineLoveLetter::updateCelebrate() {}
void ValentineLoveLetter::updateFade() {}
void ValentineLoveLetter::updateClose() {}
void ValentineLoveLetter::updateRest() {}

void ValentineLoveLetter::render() {
    if (_display == nullptr || _state == LoveLetterState::INACTIVE) return;

    uint32_t now = millis();

    const int16_t cardX = 1;
    const int16_t cardY = 17;
    const int16_t cardW = 62;
    const int16_t cardH = 30;

    const int16_t guardX0 = cardX - 2;
    const int16_t guardY0 = cardY - 2;
    const int16_t guardX1 = cardX + cardW + 1;
    const int16_t guardY1 = cardY + cardH + 1;

    // Dim diagonal rainbow background
    for (int16_t y = 0; y < 64; y++) {
        for (int16_t x = 0; x < 64; x++) {
            if (x >= guardX0 && x <= guardX1 && y >= guardY0 && y <= guardY1) {
                drawPixelSafe(x, y, BG_BLACK);
                continue;
            }
            uint8_t hue = (uint8_t)((x * 2 + y * 2 + (now / 32)) & 0xFF);
            uint16_t c = hsvTo565(hue, 160, 34);
            drawPixelSafe(x, y, c);
        }
    }

    // Slight dark overlay bands so text/heart stand out more
    for (int16_t y = 0; y < 64; y += 2) {
        _display->drawFastHLine(0, y, 64, dimColor(BG_BLACK, 0.18f));
    }

    // Text and heart layout: I ❤ YOU
    int16_t baseY = 22;
    int16_t xHeart = 21;

    // Solid black card behind text/heart for contrast
    _display->fillRect(cardX, cardY, cardW, cardH, BG_BLACK);
    _display->drawRect(cardX, cardY, cardW, cardH, BG_BLACK);
    _display->drawRect(cardX + 1, cardY + 1, cardW - 2, cardH - 2, BG_BLACK);

    drawEnvelope(true);
    drawEnvelopeFlap(0.0f);
    drawSeal(0.0f);

    // Heart in center (beats)
    drawHeart(xHeart + 6, baseY + 6, _heartScale, _heartBrightness);

    // Tiny sparkle accents
    if (((now / 140) % 2) == 0) {
        drawPixelSafe(xHeart + 1, baseY + 1, SPARKLE);
        drawPixelSafe(xHeart + 11, baseY + 13, SPARKLE);
    }
}

void ValentineLoveLetter::drawEnvelope(bool drawSeal) {
    (void)drawSeal;

    // Draw thick "I"
    int16_t x = 4;
    int16_t y = 22;
    uint16_t mainColor = SPARKLE;
    uint16_t outlineColor = rgb565(70, 120, 255);

    const int16_t glyphW = 4 * CELL;
    const int16_t glyphH = FONT_HEIGHT * CELL;
    uint8_t mask[24][16] = {0};

    for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            if (!(GLYPH_I[row] & (1 << (3 - col)))) continue;
            for (uint8_t sy = 0; sy < CELL; sy++) {
                for (uint8_t sx = 0; sx < CELL; sx++) {
                    mask[row * CELL + sy][col * CELL + sx] = 1;
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
            bool edge = false;
            for (int8_t oy = -1; oy <= 1 && !edge; oy++) {
                for (int8_t ox = -1; ox <= 1; ox++) {
                    if (ox == 0 && oy == 0) continue;
                    int16_t nx = px + ox;
                    int16_t ny = py + oy;
                    if (nx < 0 || ny < 0 || nx >= glyphW || ny >= glyphH) continue;
                    if (mask[ny][nx]) {
                        edge = true;
                        break;
                    }
                }
            }
            if (edge) {
                drawPixelSafe(x + px, y + py, outlineColor);
            }
        }
    }
}

void ValentineLoveLetter::drawEnvelopeFlap(float openProgress) {
    (void)openProgress;

    // Draw thick "YOU"
    const uint8_t* glyphs[3] = {GLYPH_Y, GLYPH_O, GLYPH_U};
    int16_t x = 35;
    int16_t y = 22;

    uint16_t mainColor = SPARKLE;
    uint16_t outlineColor = rgb565(70, 120, 255);

    for (int g = 0; g < 3; g++) {
        const uint8_t* glyph = glyphs[g];
        const int16_t glyphW = 4 * CELL;
        const int16_t glyphH = FONT_HEIGHT * CELL;
        uint8_t mask[24][16] = {0};

        for (uint8_t row = 0; row < FONT_HEIGHT; row++) {
            for (uint8_t col = 0; col < 4; col++) {
                if (!(glyph[row] & (1 << (3 - col)))) continue;
                for (uint8_t sy = 0; sy < CELL; sy++) {
                    for (uint8_t sx = 0; sx < CELL; sx++) {
                        mask[row * CELL + sy][col * CELL + sx] = 1;
                    }
                }
            }
        }

        int16_t gx = x + g * 10;
        for (int16_t py = 0; py < glyphH; py++) {
            for (int16_t px = 0; px < glyphW; px++) {
                if (mask[py][px]) {
                    drawPixelSafe(gx + px, y + py, mainColor);
                }
            }
        }

        for (int16_t py = 0; py < glyphH; py++) {
            for (int16_t px = 0; px < glyphW; px++) {
                if (mask[py][px]) continue;
                bool edge = false;
                for (int8_t oy = -1; oy <= 1 && !edge; oy++) {
                    for (int8_t ox = -1; ox <= 1; ox++) {
                        if (ox == 0 && oy == 0) continue;
                        int16_t nx = px + ox;
                        int16_t ny = py + oy;
                        if (nx < 0 || ny < 0 || nx >= glyphW || ny >= glyphH) continue;
                        if (mask[ny][nx]) {
                            edge = true;
                            break;
                        }
                    }
                }
                if (edge) {
                    drawPixelSafe(gx + px, y + py, outlineColor);
                }
            }
        }
    }
}

void ValentineLoveLetter::drawSeal(float glow) {
    (void)glow;
}

void ValentineLoveLetter::drawHeart(int16_t x, int16_t y, float scale, float brightness) {
    int16_t r = (int16_t)(5 * scale);

    uint16_t cMain = dimColor(rgb565(255, 0, 0), min(1.0f, brightness * 1.12f));
    uint16_t cHi = dimColor(rgb565(255, 96, 96), min(1.0f, brightness * 1.05f));

    // Heart body (two circles + triangle)
    drawDisc(_display, x - r / 2, y - r / 3, r, cMain);
    drawDisc(_display, x + r / 2, y - r / 3, r, cMain);

    int16_t topY = y;
    int16_t bottomY = y + (int16_t)(r * 1.7f);
    for (int16_t py = topY; py <= bottomY; py++) {
        float t = (float)(py - topY) / max(1, bottomY - topY);
        int16_t half = (int16_t)((1.0f - t) * r);
        _display->drawFastHLine(x - half, py, half * 2 + 1, cMain);
    }

    // Highlight
    drawDisc(_display, x - r / 2, y - r / 3, max(1, r / 3), cHi);
}

void ValentineLoveLetter::drawConfetti() {}
void ValentineLoveLetter::initConfetti() {}
void ValentineLoveLetter::updateConfettiPhysics() {}

void ValentineLoveLetter::drawPixelSafe(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < 64 && y >= 0 && y < 64) {
        _display->drawPixel(x, y, color);
    }
}
