#ifndef CHICK1_ANIMATION_H
#define CHICK1_ANIMATION_H

#include <Arduino.h>
#include "AnimationBase.h"

class Chick1Animation : public IAnimation {
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
    bool _blink = false;
    float _bob = 0.0f;

    void drawPixelSafe(int16_t x, int16_t y, uint16_t color);
    void fillEllipse(int16_t cx, int16_t cy, int16_t rx, int16_t ry, uint16_t color);
    void drawChickFront(int16_t cx, int16_t cy);
};

extern Chick1Animation chick1Animation;

#endif // CHICK1_ANIMATION_H
