#ifndef VALENTINE_LOVE_LETTER_H
#define VALENTINE_LOVE_LETTER_H

#include <Arduino.h>
#include "AnimationBase.h"

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * VALENTINE LOVE LETTER ANIMATION
 * ═══════════════════════════════════════════════════════════════════════════
 * 
 * EMOTIONAL DESCRIPTION:
 * ──────────────────────
 * A sealed envelope sits quietly, waiting to be opened. Its heart-shaped
 * wax seal glows softly, hinting at the love inside. Slowly, the envelope
 * opens, revealing a beautiful heart that rises gracefully into the air—
 * carrying feelings too deep for words. As the heart floats upward,
 * gentle confetti sparkles drift down like celebration, and the heart
 * pulses with warmth before the envelope gently closes, ready to send
 * its love again.
 * 
 * This represents the courage it takes to express love—and the joy
 * that follows when we do.
 * 
 * VISUAL JOURNEY:
 * ──────────────
 * 1. SEALED: Envelope appears, sealed with heart wax seal (2s)
 * 2. GLOW: Seal glows and pulses, building anticipation (2s)
 * 3. OPEN: Envelope flap opens gracefully (1.5s)
 * 4. REVEAL: Heart emerges from inside (1s)
 * 5. RISE: Heart floats up, growing slightly (3s)
 * 6. CELEBRATE: Confetti falls, heart pulses at top (3s)
 * 7. FADE: Heart gently fades (1.5s)
 * 8. CLOSE: Envelope closes, seal reappears (2s)
 * 9. REST: Brief pause before loop (1s)
 * 
 * STATE DIAGRAM:
 * ─────────────
 *   ┌──────────┐
 *   │  SEALED  │ ─────▶ Envelope fades in with seal
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │   GLOW   │ ─────▶ Seal pulses enchantingly
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │   OPEN   │ ─────▶ Flap lifts open
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │  REVEAL  │ ─────▶ Heart rises from envelope
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │   RISE   │ ─────▶ Heart floats to top
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │CELEBRATE │ ─────▶ Confetti + heart pulse
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │   FADE   │ ─────▶ Heart fades out
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │  CLOSE   │ ─────▶ Envelope closes
 *   └────┬─────┘
 *        ▼
 *   ┌──────────┐
 *   │   REST   │ ─────▶ Loop to GLOW
 *   └──────────┘
 * 
 * TIMING & PACING:
 * ────────────────
 * - Sealed: 2000ms - envelope materializes
 * - Glow: 2500ms - magical seal glow
 * - Open: 1500ms - flap lifts gracefully
 * - Reveal: 1000ms - heart peeks out
 * - Rise: 3500ms - float to top
 * - Celebrate: 3000ms - confetti joy
 * - Fade: 1500ms - gentle goodbye
 * - Close: 2000ms - flap returns
 * - Rest: 1500ms - breathing room
 * 
 * Total cycle: ~18 seconds
 */

// ============================================================================
// Animation States
// ============================================================================

enum class LoveLetterState : uint8_t {
    INACTIVE,
    SEALED,
    GLOW,
    OPEN,
    REVEAL,
    RISE,
    CELEBRATE,
    FADE_OUT,
    CLOSE,
    REST
};

// ============================================================================
// Configuration
// ============================================================================

namespace LoveLetterConfig {
    // Envelope position
    constexpr int16_t ENV_X = 12;
    constexpr int16_t ENV_Y = 30;
    constexpr int16_t ENV_WIDTH = 40;
    constexpr int16_t ENV_HEIGHT = 28;
    
    // Heart position
    constexpr int16_t HEART_START_Y = 35;
    constexpr int16_t HEART_END_Y = 8;
    constexpr int16_t HEART_X = 32;
    
    // Timing (ms)
    constexpr uint16_t SEALED_DURATION    = 2000;
    constexpr uint16_t GLOW_DURATION      = 2500;
    constexpr uint16_t OPEN_DURATION      = 1500;
    constexpr uint16_t REVEAL_DURATION    = 1000;
    constexpr uint16_t RISE_DURATION      = 3500;
    constexpr uint16_t CELEBRATE_DURATION = 3000;
    constexpr uint16_t FADE_DURATION      = 1500;
    constexpr uint16_t CLOSE_DURATION     = 2000;
    constexpr uint16_t REST_DURATION      = 1500;
    
    // Visual properties
    constexpr uint8_t CONFETTI_COUNT = 12;
}

// ============================================================================
// Animation Class
// ============================================================================

class ValentineLoveLetter : public IAnimation {
public:
    void init(IDisplay* display) override;
    void start() override;
    void stop() override;
    void update() override;
    void render() override;
    bool isRunning() const override;
    
private:
    IDisplay* _display = nullptr;
    LoveLetterState _state = LoveLetterState::INACTIVE;
    
    uint32_t _stateStartTime = 0;
    
    // Envelope state
    float _envBrightness;
    float _flapAngle;       // 0 = closed, 1 = fully open
    float _sealGlow;
    
    // Heart state
    float _heartY;
    float _heartScale;
    float _heartBrightness;
    
    // Confetti particles
    struct Confetti {
        float x, y;
        float speedY;
        float phase;
        uint16_t color;
        bool active;
    };
    Confetti _confetti[LoveLetterConfig::CONFETTI_COUNT];
    
    // State machine
    void transitionTo(LoveLetterState newState);
    
    // Update functions
    void updateSealed();
    void updateGlow();
    void updateOpen();
    void updateReveal();
    void updateRise();
    void updateCelebrate();
    void updateFade();
    void updateClose();
    void updateRest();
    
    // Rendering
    void drawEnvelope(bool drawSeal);
    void drawEnvelopeFlap(float openProgress);
    void drawSeal(float glow);
    void drawHeart(int16_t x, int16_t y, float scale, float brightness);
    void drawConfetti();
    void initConfetti();
    void updateConfettiPhysics();
    void drawPixelSafe(int16_t x, int16_t y, uint16_t color);
};

// Global instance
extern ValentineLoveLetter valentineLoveLetter;

#endif // VALENTINE_LOVE_LETTER_H
