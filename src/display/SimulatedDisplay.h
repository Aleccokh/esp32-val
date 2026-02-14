#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "IDisplay.h"

class SimulatedDisplay : public IDisplay {
public:
    SimulatedDisplay(int16_t width = 64, int16_t height = 64, int pixelScale = 8);

    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override;
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;

    void fillScreen(uint16_t color) override;
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    void clearScreen() override;

    void setCursor(int16_t x, int16_t y) override;
    void print(const char* text) override;
    void setTextColor(uint16_t color) override;
    void setTextWrap(bool wrap) override;
    void setTextSize(uint8_t size) override;

    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) override;

    bool saveFramePPM(const std::string& filePath) const;

private:
    int16_t _width;
    int16_t _height;
    int _pixelScale;
    std::vector<uint16_t> _framebuffer;

    int16_t _cursorX = 0;
    int16_t _cursorY = 0;
    uint16_t _textColor = 0xFFFF;
    bool _textWrap = false;
    uint8_t _textSize = 1;

    uint16_t getPixel(int16_t x, int16_t y) const;
    static void rgb565To888(uint16_t c, uint8_t& r, uint8_t& g, uint8_t& b);
};
