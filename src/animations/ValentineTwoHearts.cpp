#include "ValentineTwoHearts.h"
#include <math.h>

using namespace ColorUtil;
using namespace Easing;

ValentineTwoHearts valentineTwoHearts;

namespace {
constexpr int16_t ROSE_CX = 32;
constexpr int16_t ROSE_CY = 34;
constexpr int16_t STEM_BOTTOM_Y = 63;

void drawDisc(IDisplay* display, int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    for (int16_t y = -r; y <= r; y++) {
        int16_t xr = (int16_t)sqrtf((float)(r * r - y * y));
        display->drawFastHLine(cx - xr, cy + y, xr * 2 + 1, color);
    }
}
}

void ValentineTwoHearts::init(IDisplay* display) {
    _display = display;
    _state = TwoHeartsState::INACTIVE;
    _leftX = ROSE_CX;
    _leftY = ROSE_CY;
    _rightX = ROSE_CX;
    _rightY = ROSE_CY;
    _leftScale = 0.2f;
    _rightScale = 0.2f;
    _leftBrightness = 0.0f;
    _rightBrightness = 0.0f;
    _mergeProgress = 0.0f;
}

void ValentineTwoHearts::start() {
    if (_display == nullptr) return;
    _state = TwoHeartsState::APPEAR;
    _stateStartTime = millis();
    _leftScale = 0.2f;
    _rightScale = 0.2f;
    _leftBrightness = 0.0f;
    _rightBrightness = 0.0f;
    _mergeProgress = 0.0f;
}

void ValentineTwoHearts::stop() {
    _state = TwoHeartsState::INACTIVE;
    if (_display) _display->fillScreen(BG_BLACK);
}

bool ValentineTwoHearts::isRunning() const {
    return _state != TwoHeartsState::INACTIVE;
}

void ValentineTwoHearts::transitionTo(TwoHeartsState newState) {
    _state = newState;
    _stateStartTime = millis();
}

void ValentineTwoHearts::update() {
    if (_display == nullptr || _state == TwoHeartsState::INACTIVE) return;

    switch (_state) {
        case TwoHeartsState::APPEAR: updateAppear(); break;
        case TwoHeartsState::NOTICE: updateNotice(); break;
        case TwoHeartsState::APPROACH: updateApproach(); break;
        case TwoHeartsState::MERGE: updateMerge(); break;
        case TwoHeartsState::UNITY: updateUnity(); break;
        case TwoHeartsState::SEPARATE: updateSeparate(); break;
        case TwoHeartsState::RETURN_HOME: updateReturn(); break;
        case TwoHeartsState::PAUSE: updatePause(); break;
        default: break;
    }
}

void ValentineTwoHearts::updateAppear() {
    float p = (float)(millis() - _stateStartTime) / 1000.0f;
    if (p >= 1.0f) {
        _leftBrightness = 1.0f;
        _leftScale = 0.45f;
        transitionTo(TwoHeartsState::NOTICE);
        return;
    }
    _leftBrightness = easeInOutCubic(constrain(p, 0.0f, 1.0f));
}

void ValentineTwoHearts::updateNotice() {
    float p = (float)(millis() - _stateStartTime) / 800.0f;
    if (p >= 1.0f) {
        transitionTo(TwoHeartsState::APPROACH);
        return;
    }
    _leftScale = 0.45f + 0.05f * sinf(p * 6.28318f);
}

void ValentineTwoHearts::updateApproach() {
    float p = (float)(millis() - _stateStartTime) / 2200.0f;
    if (p >= 1.0f) {
        _mergeProgress = 1.0f;
        transitionTo(TwoHeartsState::MERGE);
        return;
    }
    _mergeProgress = easeOutCubic(constrain(p, 0.0f, 1.0f));
    _leftScale = 0.45f + 0.55f * _mergeProgress;
}

void ValentineTwoHearts::updateMerge() {
    float p = (float)(millis() - _stateStartTime) / 1700.0f;
    if (p >= 1.0f) {
        transitionTo(TwoHeartsState::UNITY);
        return;
    }
    _rightScale = 1.0f + 0.14f * sinf(p * 6.28318f);
}

void ValentineTwoHearts::updateUnity() {
    float p = (float)(millis() - _stateStartTime) / 2600.0f;
    if (p >= 1.0f) {
        transitionTo(TwoHeartsState::SEPARATE);
        return;
    }
    _rightBrightness = 0.8f + 0.2f * sinf(p * 8.0f * 3.14159f);
}

void ValentineTwoHearts::updateSeparate() {
    float p = (float)(millis() - _stateStartTime) / 1600.0f;
    if (p >= 1.0f) {
        transitionTo(TwoHeartsState::RETURN_HOME);
        return;
    }
    _leftScale = 1.0f - 0.45f * easeInOutCubic(constrain(p, 0.0f, 1.0f));
}

void ValentineTwoHearts::updateReturn() {
    float p = (float)(millis() - _stateStartTime) / 900.0f;
    if (p >= 1.0f) {
        transitionTo(TwoHeartsState::PAUSE);
        return;
    }
    _leftBrightness = 1.0f - 0.4f * p;
}

void ValentineTwoHearts::updatePause() {
    if (millis() - _stateStartTime > 900) {
        transitionTo(TwoHeartsState::NOTICE);
    }
}

void ValentineTwoHearts::render() {
    if (_display == nullptr || _state == TwoHeartsState::INACTIVE) return;

    // Night garden background
    _display->fillScreen(BG_BLACK);
    for (int y = 40; y < 64; y++) {
        uint16_t c = dimColor(rgb565(0, 120, 40), 0.25f + ((y - 40) / 24.0f) * 0.25f);
        _display->drawFastHLine(0, y, 64, c);
    }

    // Stem grows with progress
    int16_t stemTop = STEM_BOTTOM_Y - (int16_t)(24 * _leftScale);
    _display->drawLine(ROSE_CX, STEM_BOTTOM_Y, ROSE_CX, stemTop, dimColor(rgb565(0, 255, 80), 0.9f));
    _display->drawLine(ROSE_CX - 1, STEM_BOTTOM_Y, ROSE_CX - 1, stemTop, dimColor(rgb565(0, 200, 70), 0.55f));

    // Leaves
    _display->drawLine(ROSE_CX, 52, ROSE_CX - 7, 56, dimColor(rgb565(0, 230, 80), 0.8f));
    _display->drawLine(ROSE_CX, 49, ROSE_CX + 8, 53, dimColor(rgb565(0, 230, 80), 0.8f));

    drawMergedHeart(_mergeProgress);
    drawSparkles();
}

void ValentineTwoHearts::drawSmallHeart(int16_t cx, int16_t cy, float scale, uint16_t baseColor, float brightness) {
    (void)cx;
    (void)cy;
    (void)scale;
    (void)baseColor;
    (void)brightness;
}

void ValentineTwoHearts::drawMergedHeart(float progress) {
    float bloom = _leftScale;
    int16_t petalBase = 3 + (int16_t)(5 * bloom);
    uint16_t deep = dimColor(HEART_RED, _leftBrightness);
    uint16_t light = dimColor(CORAL, _leftBrightness);

    // Outer petals
    for (int i = 0; i < 8; i++) {
        float a = (i / 8.0f) * 6.28318f + millis() * 0.0006f;
        int16_t px = ROSE_CX + (int16_t)(cosf(a) * (4 + petalBase));
        int16_t py = ROSE_CY + (int16_t)(sinf(a) * (3 + petalBase * 0.8f));
        drawDisc(_display, px, py, 2 + (int16_t)(bloom * 2), (i % 2 == 0) ? deep : light);
    }

    // Inner swirl
    for (int i = 0; i < 12; i++) {
        float s = i / 11.0f;
        float a = s * 7.5f + millis() * 0.0013f;
        float r = s * (3.0f + petalBase * 0.7f);
        int16_t px = ROSE_CX + (int16_t)(cosf(a) * r);
        int16_t py = ROSE_CY + (int16_t)(sinf(a) * r * 0.75f);
        drawDisc(_display, px, py, 1 + (int16_t)(progress * 2), dimColor(blendColor(deep, light, s), 0.9f));
    }

    // Core highlight
    drawDisc(_display, ROSE_CX, ROSE_CY, 2 + (int16_t)(_rightScale * 1.5f), dimColor(WARM_WHITE, 0.75f + 0.2f * _rightBrightness));
}

void ValentineTwoHearts::drawSparkles() {
    uint32_t t = millis();
    for (int i = 0; i < 10; i++) {
        float a = i * 0.62f + t * 0.002f;
        float r = 8.0f + 7.0f * (0.5f + 0.5f * sinf(t * 0.0013f + i));
        int16_t x = ROSE_CX + (int16_t)(cosf(a) * r);
        int16_t y = ROSE_CY - 4 + (int16_t)(sinf(a) * r * 0.7f);
        if (((t / 120) + i) % 3 == 0) {
            drawPixelSafe(x, y, dimColor(SPARKLE, 0.75f));
        }
    }
}

void ValentineTwoHearts::drawPixelSafe(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && x < 64 && y >= 0 && y < 64) {
        _display->drawPixel(x, y, color);
    }
}
