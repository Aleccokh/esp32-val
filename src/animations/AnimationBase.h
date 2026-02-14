#ifndef ANIMATION_BASE_H
#define ANIMATION_BASE_H

#include <Arduino.h>
#include "IDisplay.h"

/**
 * AnimationBase.h - Shared utilities for Valentine animations
 * 
 * Provides:
 * - Common easing functions for smooth, organic motion
 * - Color utilities for RGB565 format
 * - Timing helpers
 * 
 * All animations should use these for consistent feel.
 */

// ============================================================================
// Easing Functions (0.0 to 1.0 input, 0.0 to 1.0 output)
// These create smooth, organic motion instead of robotic linear movement
// ============================================================================

namespace Easing {
    // Soft start, linear end - gentle fade in
    inline float easeInQuad(float t) {
        return t * t;
    }
    
    // Linear start, soft end - gentle fade out
    inline float easeOutQuad(float t) {
        return t * (2.0f - t);
    }
    
    // Soft start and end - most natural for position
    inline float easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }
    
    // Even softer, more "breathing" feel
    inline float easeInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3) / 2.0f;
    }
    
    // Cubic ease in only (slow start)
    inline float easeInCubic(float t) {
        return t * t * t;
    }
    
    // Cubic ease out only (slow end)
    inline float easeOutCubic(float t) {
        return 1.0f - powf(1.0f - t, 3);
    }
    
    // Heartbeat-style: quick rise, slow fall
    inline float heartbeatPulse(float t) {
        // Two-bump heartbeat curve
        if (t < 0.15f) {
            return easeOutQuad(t / 0.15f) * 0.7f;
        } else if (t < 0.25f) {
            return 0.7f - easeInQuad((t - 0.15f) / 0.1f) * 0.3f;
        } else if (t < 0.4f) {
            return 0.4f + easeOutQuad((t - 0.25f) / 0.15f) * 0.6f;
        } else {
            return 1.0f - easeInOutCubic((t - 0.4f) / 0.6f);
        }
    }
    
    // Gentle breathing: slow in, pause, slow out
    inline float breathe(float t) {
        return (1.0f - cosf(t * 3.14159f)) / 2.0f;
    }
    
    // Bouncy settle (for landing moments)
    inline float easeOutBounce(float t) {
        if (t < 0.5f) {
            return easeOutQuad(t * 2.0f);
        } else if (t < 0.75f) {
            float bt = (t - 0.5f) * 4.0f;
            return 1.0f - 0.15f * (1.0f - bt * bt);
        } else {
            return 1.0f;
        }
    }
}

// ============================================================================
// Color Utilities (RGB565 format for LED matrix)
// ============================================================================

namespace ColorUtil {
    // Create RGB565 from 8-bit components
    inline uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    
    // Dim a color by factor (0.0 = black, 1.0 = full)
    inline uint16_t dimColor(uint16_t color, float factor) {
        if (factor <= 0.0f) return 0x0000;
        if (factor >= 1.0f) return color;
        
        uint8_t r = ((color >> 11) & 0x1F);
        uint8_t g = ((color >> 5) & 0x3F);
        uint8_t b = (color & 0x1F);
        
        r = (uint8_t)(r * factor);
        g = (uint8_t)(g * factor);
        b = (uint8_t)(b * factor);
        
        return (r << 11) | (g << 5) | b;
    }
    
    // Blend two colors (0.0 = color1, 1.0 = color2)
    inline uint16_t blendColor(uint16_t color1, uint16_t color2, float t) {
        if (t <= 0.0f) return color1;
        if (t >= 1.0f) return color2;
        
        uint8_t r1 = (color1 >> 11) & 0x1F;
        uint8_t g1 = (color1 >> 5) & 0x3F;
        uint8_t b1 = color1 & 0x1F;
        
        uint8_t r2 = (color2 >> 11) & 0x1F;
        uint8_t g2 = (color2 >> 5) & 0x3F;
        uint8_t b2 = color2 & 0x1F;
        
        uint8_t r = r1 + (int8_t)((r2 - r1) * t);
        uint8_t g = g1 + (int8_t)((g2 - g1) * t);
        uint8_t b = b1 + (int8_t)((b2 - b1) * t);
        
        return (r << 11) | (g << 5) | b;
    }
    
    // Valentine color palette
    constexpr uint16_t DEEP_RED     = 0xC000;  // Rich romantic red
    constexpr uint16_t HEART_RED    = 0xF800;  // Classic heart red
    constexpr uint16_t SOFT_PINK    = 0xFB2C;  // Gentle pink
    constexpr uint16_t BLUSH_PINK   = 0xFDF9;  // Very soft pink
    constexpr uint16_t WARM_WHITE   = 0xFFF8;  // Cream white
    constexpr uint16_t SPARKLE      = 0xFFFF;  // Pure white
    constexpr uint16_t CORAL        = 0xFC10;  // Warm coral
    constexpr uint16_t MAGENTA      = 0xF81F;  // Transparent key
    constexpr uint16_t BG_BLACK     = 0x0000;  // Background
    constexpr uint16_t ENVELOPE_TAN = 0xED55;  // Paper/envelope color
    constexpr uint16_t SEAL_RED     = 0xB000;  // Wax seal
}

// ============================================================================
// Common Animation Interface
// ============================================================================

class IAnimation {
public:
    virtual ~IAnimation() = default;
    
    virtual void init(IDisplay* display) = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual bool isRunning() const = 0;
};

#endif // ANIMATION_BASE_H
