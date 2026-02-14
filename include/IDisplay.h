#pragma once

#include <Arduino.h>

class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
    virtual void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) = 0;
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) = 0;
    virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;

    virtual void fillScreen(uint16_t color) = 0;
    virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
    virtual void clearScreen() = 0;

    virtual void setCursor(int16_t x, int16_t y) = 0;
    virtual void print(const char* text) = 0;
    virtual void setTextColor(uint16_t color) = 0;
    virtual void setTextWrap(bool wrap) = 0;
    virtual void setTextSize(uint8_t size) = 0;

    virtual uint16_t color565(uint8_t r, uint8_t g, uint8_t b) = 0;
};
