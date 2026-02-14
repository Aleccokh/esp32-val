#ifndef EGG_CHICK_ANIMATION_H
#define EGG_CHICK_ANIMATION_H

#include <Arduino.h>
#include "IDisplay.h"

/**
 * Egg Chick Hatching Animation
 * 
 * A cute, expressive animation showing:
 * 1. Large egg wobbling gently
 * 2. Progressive cracking (3 stages)
 * 3. Chick emerging from the shell
 * 4. Chick blowing a kiss with floating heart
 * 
 * Optimized for 64x64 RGB LED matrix (P3)
 * Uses state machine with millis-based timing (no delays)
 * 
 * Memory: ~8KB PROGMEM for sprites, <100 bytes RAM state
 */

// ============================================================================
// Animation States
// ============================================================================

enum class EggChickState : uint8_t {
    INACTIVE,           // Animation not running
    EGG_IDLE,           // Egg wobbling gently
    CRACK_STAGE_1,      // First small crack appears
    CRACK_STAGE_2,      // Crack spreads, more cracks appear
    CRACK_STAGE_3,      // Large crack, egg shakes violently
    HATCHING,           // Top of egg opens
    CHICK_EMERGE,       // Chick pops up from egg
    CHICK_LOOK,         // Chick looks around happily
    CHICK_KISS,         // Chick blows a kiss
    HEART_FLOAT,        // Heart floats up
    COMPLETE            // Animation finished
};

// ============================================================================
// Configuration
// ============================================================================

namespace EggChickConfig {
    // Timing (milliseconds) - Adjusted for slower, more deliberate pacing
    constexpr uint16_t WOBBLE_INTERVAL    = 300;   // Slower wobble
    constexpr uint16_t CRACK_DELAY        = 1500;  // Longer crack stages (let the suspense build)
    constexpr uint16_t SHAKE_INTERVAL     = 100;   // Less frantic shake
    constexpr uint16_t EMERGE_DELAY       = 1500;  // Chick takes time to pop out
    constexpr uint16_t LOOK_INTERVAL      = 600;   // Slower looking around
    constexpr uint16_t KISS_DELAY         = 1000;  // Hold the kiss pose
    constexpr uint16_t HEART_INTERVAL     = 120;   // Slower heart float
    
    // Total animation duration (extended)
    constexpr uint16_t TOTAL_DURATION_MS  = 12000;
    
    // Sprite dimensions
    constexpr uint8_t EGG_WIDTH           = 40;
    constexpr uint8_t EGG_HEIGHT          = 48;
    constexpr uint8_t CHICK_WIDTH         = 32;
    constexpr uint8_t CHICK_HEIGHT        = 28;
    constexpr uint8_t HEART_SIZE          = 8;
    
    // Positioning (centered on 64x64)
    constexpr int8_t EGG_X                = 12;
    constexpr int8_t EGG_Y                = 10;
    constexpr int8_t CHICK_X              = 16;
    constexpr int8_t CHICK_Y              = 8;
    constexpr int8_t HEART_START_X        = 44;
    constexpr int8_t HEART_START_Y        = 20;
}

// ============================================================================
// Animation Class
// ============================================================================

class EggChickAnimation {
public:
    /**
     * Initialize the animation system
     * @param display Pointer to the matrix display driver
     */
    void init(IDisplay* display);
    
    /**
     * Start the animation from the beginning
     * Call this when MQTT trigger is received
     */
    void start();
    
    /**
     * Stop the animation immediately
     */
    void stop();
    
    /**
     * Update animation state (call every frame in main loop)
     * Uses internal timing - safe to call at any rate
     */
    void update();
    
    /**
     * Render current animation frame to display
     * Call after update() in render loop
     */
    void render();
    
    /**
     * Check if animation is currently running
     */
    bool isRunning() const;
    
    /**
     * Check if animation has completed
     */
    bool isComplete() const;
    
    /**
     * Get current animation state (for debugging)
     */
    EggChickState getState() const;
    
private:
    // Display reference
    IDisplay* _display = nullptr;
    
    // State machine
    EggChickState _state = EggChickState::INACTIVE;
    uint32_t _stateStartTime = 0;
    uint32_t _lastFrameTime = 0;
    
    // Animation sub-state
    uint8_t _frameIndex = 0;
    int8_t _wobbleOffset = 0;
    uint8_t _crackFrame = 0;
    uint8_t _chickFrame = 0;
    int8_t _heartY = 0;
    uint8_t _heartAlpha = 255;
    
    // Internal methods
    void transitionTo(EggChickState newState);
    void updateIdle();
    void updateCrack();
    void updateHatching();
    void updateChickEmerge();
    void updateChickLook();
    void updateChickKiss();
    void updateHeartFloat();
    
    // Render helpers
    void drawEgg(int8_t xOffset, int8_t yOffset, uint8_t crackStage);
    void drawChick(int8_t xOffset, int8_t yOffset, uint8_t frame);
    void drawHeart(int16_t x, int16_t y, uint8_t brightness);
    void drawSprite(const uint16_t* sprite, int16_t x, int16_t y, 
                    uint8_t width, uint8_t height);
    void drawCrackOverlay(uint8_t stage);
    void clearScreen();
};

// ============================================================================
// Global Instance
// ============================================================================

extern EggChickAnimation eggChickAnimation;

#endif // EGG_CHICK_ANIMATION_H
