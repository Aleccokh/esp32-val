#include "SimulatedDisplay.h"

#include <fstream>
#include <algorithm>

SimulatedDisplay::SimulatedDisplay(int16_t width, int16_t height, int pixelScale)
    : _width(width), _height(height), _pixelScale(pixelScale), _framebuffer(width * height, 0) {}

void SimulatedDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;
    _framebuffer[y * _width + x] = color;
}

void SimulatedDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy;

    while (true) {
        drawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void SimulatedDisplay::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if (y < 0 || y >= _height || w <= 0) return;
    int16_t x0 = std::max<int16_t>(0, x);
    int16_t x1 = std::min<int16_t>(_width - 1, x + w - 1);
    for (int16_t px = x0; px <= x1; ++px) drawPixel(px, y, color);
}

void SimulatedDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);
    drawLine(x, y, x, y + h - 1, color);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, color);
}

void SimulatedDisplay::fillScreen(uint16_t color) {
    std::fill(_framebuffer.begin(), _framebuffer.end(), color);
}

void SimulatedDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (w <= 0 || h <= 0) return;
    int16_t x0 = std::max<int16_t>(0, x);
    int16_t y0 = std::max<int16_t>(0, y);
    int16_t x1 = std::min<int16_t>(_width - 1, x + w - 1);
    int16_t y1 = std::min<int16_t>(_height - 1, y + h - 1);

    for (int16_t py = y0; py <= y1; ++py) {
        for (int16_t px = x0; px <= x1; ++px) {
            drawPixel(px, py, color);
        }
    }
}

void SimulatedDisplay::clearScreen() {
    fillScreen(0);
}

void SimulatedDisplay::setCursor(int16_t x, int16_t y) {
    _cursorX = x;
    _cursorY = y;
}

void SimulatedDisplay::print(const char* text) {
    (void)text;
    // Intentionally no-op for now; animation rendering doesn't depend on text rasterization in simulator.
}

void SimulatedDisplay::setTextColor(uint16_t color) {
    _textColor = color;
}

void SimulatedDisplay::setTextWrap(bool wrap) {
    _textWrap = wrap;
}

void SimulatedDisplay::setTextSize(uint8_t size) {
    _textSize = size;
}

uint16_t SimulatedDisplay::color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint16_t SimulatedDisplay::getPixel(int16_t x, int16_t y) const {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return 0;
    return _framebuffer[y * _width + x];
}

void SimulatedDisplay::rgb565To888(uint16_t c, uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t r5 = (c >> 11) & 0x1F;
    uint8_t g6 = (c >> 5) & 0x3F;
    uint8_t b5 = c & 0x1F;

    r = (r5 * 255) / 31;
    g = (g6 * 255) / 63;
    b = (b5 * 255) / 31;
}

bool SimulatedDisplay::saveFramePPM(const std::string& filePath) const {
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) return false;

    int outW = _width * _pixelScale;
    int outH = _height * _pixelScale;

    file << "P6\n" << outW << " " << outH << "\n255\n";

    for (int16_t y = 0; y < _height; ++y) {
        for (int sy = 0; sy < _pixelScale; ++sy) {
            for (int16_t x = 0; x < _width; ++x) {
                uint8_t r, g, b;
                rgb565To888(getPixel(x, y), r, g, b);
                for (int sx = 0; sx < _pixelScale; ++sx) {
                    file.put(static_cast<char>(r));
                    file.put(static_cast<char>(g));
                    file.put(static_cast<char>(b));
                }
            }
        }
    }

    return true;
}
