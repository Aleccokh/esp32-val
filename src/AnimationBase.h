#ifndef ANIMATION_BASE_H
#define ANIMATION_BASE_H

#include <Arduino.h>

/**
 * AnimationBase.h
 * 
 * Common animation structures and utilities for RGB LED matrix animations.
 * Designed for ESP32-S3 with 64x64 RGB panels.
 */

// Panel dimensions
#define MATRIX_WIDTH  64
#define MATRIX_HEIGHT 64

// Transparent color (magenta - not visible on typical content)
#define TRANSPARENT_COLOR 0xF81F

/**
 * Frame-based animation data structure
 * Used for sprite-sheet style animations with PROGMEM storage
 */
struct AnimationData {
    const uint16_t** frames;    // Array of frame pointers (PROGMEM)
    uint8_t frameCount;         // Number of frames
    uint8_t frameWidth;         // Width of each frame
    uint8_t frameHeight;        // Height of each frame
    uint16_t frameDelayMs;      // Delay between frames in milliseconds
    bool loop;                  // Whether animation should loop
    bool useTransparency;       // Whether to use transparent color
};

/**
 * Animation state for state-machine based animations
 */
enum class AnimationState : uint8_t {
    IDLE,
    RUNNING,
    PAUSED,
    COMPLETE
};

/**
 * Utility: Convert RGB888 to RGB565
 */
inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/**
 * Utility: Check if color is transparent
 */
inline bool isTransparent(uint16_t color) {
    return color == TRANSPARENT_COLOR;
}

#endif // ANIMATION_BASE_H
