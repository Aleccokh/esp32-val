#pragma once

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "IDisplay.h"

class Hub75Display : public IDisplay {
public:
    explicit Hub75Display(MatrixPanel_I2S_DMA* matrix) : _matrix(matrix) {}

    void drawPixel(int16_t x, int16_t y, uint16_t color) override { _matrix->drawPixel(x, y, color); }
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override { _matrix->drawLine(x0, y0, x1, y1, color); }
    void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) override { _matrix->drawFastHLine(x, y, w, color); }
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override { _matrix->drawRect(x, y, w, h, color); }

    void fillScreen(uint16_t color) override { _matrix->fillScreen(color); }
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override { _matrix->fillRect(x, y, w, h, color); }
    void clearScreen() override { _matrix->clearScreen(); }

    void setCursor(int16_t x, int16_t y) override { _matrix->setCursor(x, y); }
    void print(const char* text) override { _matrix->print(text); }
    void setTextColor(uint16_t color) override { _matrix->setTextColor(color); }
    void setTextWrap(bool wrap) override { _matrix->setTextWrap(wrap); }
    void setTextSize(uint8_t size) override { _matrix->setTextSize(size); }

    uint16_t color565(uint8_t r, uint8_t g, uint8_t b) override { return _matrix->color565(r, g, b); }

private:
    MatrixPanel_I2S_DMA* _matrix;
};
