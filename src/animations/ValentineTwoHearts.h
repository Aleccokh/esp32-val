#ifndef VALENTINE_TWO_HEARTS_H
#define VALENTINE_TWO_HEARTS_H

#include <Arduino.h>
#include "AnimationBase.h"

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * VALENTINE TWO HEARTS ANIMATION
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * EMOTIONAL DESCRIPTION:
 * ──────────────────────
 * Two hearts begin on opposite sides of the screen—separate, searching.
 * They notice each other and slowly drift closer, drawn by an invisible
 * force. As they meet in the center, they merge into one larger, brighter
 * heart that pulses with combined energy. After a tender moment together,
 * they gently separate and return to their starting positions, only to
 * begin the dance again—an eternal cycle of connection.
 * 
 * This represents the beautiful truth that love finds love.
 * 
 * VISUAL JOURNEY:
 * ──────────────
 * 1. APPEAR: Both hearts fade in on opposite sides (2s)
 * 2. NOTICE: Small pulse as they "see" each other (0.5s)
 * 3. APPROACH: Drift slowly toward center with gentle easing (3s)
 * 4. MERGE: Overlap and blend into one larger heart (1.5s)
 * 5. UNITY: Combined heart pulses with joy (3s)
 * 6. SEPARATE: Gently drift apart (2s)
 * 7. RETURN: Float back to starting positions (2s)
 * 8. PAUSE: Brief rest before next cycle (1s)
 * 
 * STATE DIAGRAM:
 * ─────────────
 *   ┌──────────┐
 *   │  APPEAR  │ ─────▶ Both hearts fade in at opposite corners
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │  NOTICE  │ ─────▶ Small excited pulse
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │ APPROACH │ ─────▶ Drift toward center with easing
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │  MERGE   │ ─────▶ Overlap, colors blend, size grows
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │  UNITY   │ ─────▶ Single heart pulses happily
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │ SEPARATE │ ─────▶ Slowly pull apart
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │  RETURN  │ ─────▶ Float back to start
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │  PAUSE   │ ─────▶ Rest, then loop to NOTICE
 *   └──────────┘
 * 
 * TIMING & PACING:
 * ────────────────
 * - Appear: 2000ms - gentle fade in
 * - Notice: 500ms - quick happy pulse
 * - Approach: 3500ms - slow, building anticipation
 * - Merge: 1500ms - magical moment
 * - Unity: 3000ms - celebrate together
 * - Separate: 2500ms - gentle release
 * - Return: 2000ms - back to origins
 * - Pause: 1500ms - breathing room
 * 
 * Total cycle: ~16 seconds for a complete loop
 */

// ============================================================================
// Animation States
// ============================================================================

enum class TwoHeartsState : uint8_t {
    INACTIVE,
    APPEAR,
    NOTICE,
    APPROACH,
    MERGE,
    UNITY,
    SEPARATE,
    RETURN_HOME,
    PAUSE
};

// ============================================================================
// Configuration
// ============================================================================

namespace TwoHeartsConfig {
    // Starting positions (left and right sides)
    constexpr int16_t LEFT_START_X  = 8;
    constexpr int16_t LEFT_START_Y  = 28;
    constexpr int16_t RIGHT_START_X = 56;
    constexpr int16_t RIGHT_START_Y = 28;
    
    // Center meeting point
    constexpr int16_t CENTER_X = 32;
    constexpr int16_t CENTER_Y = 28;
    
    // Heart sizes
    constexpr float SMALL_SCALE = 0.5f;   // Individual hearts
    constexpr float MERGED_SCALE = 0.9f;  // Combined heart
    
    // Timing (ms)
    constexpr uint16_t APPEAR_DURATION   = 2000;
    constexpr uint16_t NOTICE_DURATION   = 500;
    constexpr uint16_t APPROACH_DURATION = 3500;
    constexpr uint16_t MERGE_DURATION    = 1500;
    constexpr uint16_t UNITY_DURATION    = 3000;
    constexpr uint16_t SEPARATE_DURATION = 2500;
    constexpr uint16_t RETURN_DURATION   = 2000;
    constexpr uint16_t PAUSE_DURATION    = 1500;
}

// ============================================================================
// Animation Class
// ============================================================================

class ValentineTwoHearts : public IAnimation {
public:
    void init(IDisplay* display) override;
    void start() override;
    void stop() override;
    void update() override;
    void render() override;
    bool isRunning() const override;
    
private:
    IDisplay* _display = nullptr;
    TwoHeartsState _state = TwoHeartsState::INACTIVE;
    
    uint32_t _stateStartTime = 0;
    
    // Heart positions (float for smooth motion)
    float _leftX, _leftY;
    float _rightX, _rightY;
    
    // Heart properties
    float _leftScale, _rightScale;
    float _leftBrightness, _rightBrightness;
    float _mergeProgress;  // 0 = separate, 1 = fully merged
    
    // State machine
    void transitionTo(TwoHeartsState newState);
    
    // Update functions
    void updateAppear();
    void updateNotice();
    void updateApproach();
    void updateMerge();
    void updateUnity();
    void updateSeparate();
    void updateReturn();
    void updatePause();
    
    // Rendering
    void drawSmallHeart(int16_t cx, int16_t cy, float scale, uint16_t color, float brightness);
    void drawMergedHeart(float progress);
    void drawSparkles();
    void drawPixelSafe(int16_t x, int16_t y, uint16_t color);
};

// Global instance
extern ValentineTwoHearts valentineTwoHearts;

#endif // VALENTINE_TWO_HEARTS_H
