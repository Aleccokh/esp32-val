#ifndef VALENTINE_TEXT_CHICK_H
#define VALENTINE_TEXT_CHICK_H

#include <Arduino.h>
#include "AnimationBase.h"

class ValentineTextChick : public IAnimation {
public:
    void init(IDisplay* display) override;
    void start() override;
    void stop() override;
    void update() override;
    void render() override;
    bool isRunning() const override;

private:
    IDisplay* _display = nullptr;
    bool _running = false;

    float _heartScale = 1.0f;
    float _heartBrightness = 1.0f;

    float _chickX = 0.0f;
    bool _facingRight = true;
    bool _stepA = true;
    float _chickBob = 0.0f;

    void drawPixelSafe(int16_t x, int16_t y, uint16_t color);
    void drawHeart(int16_t cx, int16_t cy, float scale, float brightness);
    void drawGlyph4x6(const uint8_t glyph[6], int16_t x, int16_t y, uint16_t mainColor, uint16_t outlineColor, uint8_t cell);
    void drawChick(int16_t x, int16_t y, bool facingRight, bool stepA);
};

extern ValentineTextChick valentineTextChick;

#endif // VALENTINE_TEXT_CHICK_H
