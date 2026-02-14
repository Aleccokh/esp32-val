#ifndef MOSQ1_ANIMATION_H
#define MOSQ1_ANIMATION_H

#include <Arduino.h>
#include "AnimationBase.h"

class Mosq1Animation : public IAnimation {
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

    void drawPixelSafe(int16_t x, int16_t y, uint16_t color);
    void drawStar(int16_t x, int16_t y, uint16_t color, bool sparkle);
    void drawShootingStar(int16_t x, int16_t y);
    void drawCrescent(int16_t cx, int16_t cy, uint16_t color);
    void fillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color);
    void drawMosque();
};

extern Mosq1Animation mosq1Animation;

#endif // MOSQ1_ANIMATION_H
