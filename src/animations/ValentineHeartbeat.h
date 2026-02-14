#ifndef VALENTINE_HEARTBEAT_H
#define VALENTINE_HEARTBEAT_H

#include <Arduino.h>
#include "AnimationBase.h"

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * VALENTINE HEARTBEAT ANIMATION
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * EMOTIONAL DESCRIPTION:
 * ──────────────────────
 * A single large heart, centered and commanding, that beats with the rhythm
 * of love. Each pulse radiates a soft glow that spreads outward like warmth
 * from the heart. The effect is hypnotic, intimate, and deeply romantic—
 * like feeling someone's heartbeat against your chest.
 * 
 * VISUAL JOURNEY:
 * ──────────────
 * 1. FADE IN: Heart materializes from darkness (2s)
 * 2. BREATHE: Gentle size oscillation, calm (4s loop)
 * 3. HEARTBEAT: Thump-thump pattern with glow rings (continuous)
 * 4. GLOW: Soft radial gradient emanates with each beat
 * 
 * STATE DIAGRAM:
 * ─────────────
 *   ┌─────────┐
 *   │ FADE_IN │ (2s)
 *   └────┬────┘
 *        ▼
 *   ┌─────────┐     ┌──────────┐
 *   │ BREATHE │────▶│ BEAT_UP  │
 *   └────┬────┘     └────┬─────┘
 *        ▲               ▼
 *        │          ┌──────────┐
 *        │          │ BEAT_DOWN│
 *        │          └────┬─────┘
 *        │               ▼
 *        │          ┌──────────┐
 *        └──────────│ GLOW_OUT │
 *                   └──────────┘
 * 
 * TIMING & PACING:
 * ────────────────
 * - Fade in: 2000ms (very slow, gentle appearance)
 * - Breathe cycle: 3000ms (calm, meditative)
 * - Heartbeat: 800ms total (thump-thump rhythm)
 *   - First beat: 150ms up, 100ms down
 *   - Pause: 100ms
 *   - Second beat: 200ms up, 150ms down
 *   - Rest: 1000ms before next cycle
 * - Glow ring expansion: 600ms with fade
 * 
 * The timing mimics a real resting heartbeat (~60-70 BPM) for authenticity.
 */

// ============================================================================
// Animation States
// ============================================================================

enum class HeartbeatState : uint8_t {
    INACTIVE,       // Not running
    FADE_IN,        // Heart appearing
    BREATHE,        // Calm breathing
    BEAT_UP_1,      // First thump rise
    BEAT_DOWN_1,    // First thump fall
    BEAT_UP_2,      // Second thump rise
    BEAT_DOWN_2,    // Second thump fall
    GLOW_OUT,       // Radial glow expanding
    REST            // Pause before next beat
};

// ============================================================================
// Configuration
// ============================================================================

namespace HeartbeatConfig {
    // Heart properties
    constexpr int16_t HEART_CENTER_X = 32;
    constexpr int16_t HEART_CENTER_Y = 30;
    constexpr float   BASE_SCALE     = 1.0f;   // Normal size
    constexpr float   BEAT_SCALE     = 1.15f;  // Expanded during beat
    constexpr float   BREATHE_RANGE  = 0.05f;  // Breathing amplitude
    
    // Timing (ms)
    constexpr uint16_t FADE_IN_DURATION   = 2000;
    constexpr uint16_t BREATHE_CYCLE      = 3000;
    constexpr uint16_t BEAT1_UP           = 120;
    constexpr uint16_t BEAT1_DOWN         = 80;
    constexpr uint16_t BEAT_PAUSE         = 80;
    constexpr uint16_t BEAT2_UP           = 150;
    constexpr uint16_t BEAT2_DOWN         = 120;
    constexpr uint16_t GLOW_DURATION      = 500;
    constexpr uint16_t REST_DURATION      = 1200;
    
    // Glow properties
    constexpr uint8_t  GLOW_RINGS         = 4;
    constexpr float    GLOW_RING_SPACING  = 3.0f;
}

// ============================================================================
// Animation Class
// ============================================================================

class ValentineHeartbeat : public IAnimation {
public:
    void init(IDisplay* display) override;
    void start() override;
    void stop() override;
    void update() override;
    void render() override;
    bool isRunning() const override;
    
private:
    IDisplay* _display = nullptr;
    HeartbeatState _state = HeartbeatState::INACTIVE;
    
    uint32_t _stateStartTime = 0;
    uint32_t _animStartTime = 0;
    
    float _currentScale = 1.0f;
    float _brightness = 1.0f;
    float _glowRadius = 0.0f;
    float _glowAlpha = 0.0f;
    
    // State transitions
    void transitionTo(HeartbeatState newState);
    
    // Update functions for each state
    void updateFadeIn();
    void updateBreathe();
    void updateBeat(bool isUp, bool isFirst);
    void updateGlow();
    void updateRest();
    
    // Rendering helpers
    void drawHeart(float scale, float brightness);
    void drawGlowRings();
    void drawPixelSafe(int16_t x, int16_t y, uint16_t color);
};

// Global instance
extern ValentineHeartbeat valentineHeartbeat;

#endif // VALENTINE_HEARTBEAT_H
