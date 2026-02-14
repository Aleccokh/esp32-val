#include "EggChickAnimation.h"

/**
 * Egg Chick Animation Implementation
 * 
 * Pixel Art Strategy:
 * - Egg: Large oval shape (~40x48) with cream/white coloring, brown speckles
 * - Cracks: Dark lines that progressively appear across the egg
 * - Chick: Cute yellow bird with orange beak, big eyes, pink cheeks
 * - Heart: Simple 8x8 red heart that floats up and fades
 * 
 * Performance Notes:
 * - All sprites stored in PROGMEM (flash) to save RAM
 * - No dynamic allocation in render loop
 * - Uses direct pixel drawing for smooth animation
 * - Optimized for 64x64 matrix with minimal overdraw
 */

// ============================================================================
// Color Palette (RGB565)
// ============================================================================

namespace Colors {
    // Transparent
    constexpr uint16_t TRANS    = 0xF81F;  // Magenta (key color)
    
    // Egg colors
    constexpr uint16_t EGG_MAIN = 0xFFDE;  // Creamy white
    constexpr uint16_t EGG_LITE = 0xFFFF;  // Pure white (highlight)
    constexpr uint16_t EGG_SHAD = 0xE71C;  // Light grey shadow
    constexpr uint16_t EGG_SPEC = 0xC618;  // Speckles (tan/brown)
    constexpr uint16_t CRACK    = 0x4208;  // Dark grey crack lines
    
    // Chick colors
    constexpr uint16_t CHICK_YEL= 0xFFE0;  // Bright yellow body
    constexpr uint16_t CHICK_LIT= 0xFFF0;  // Light yellow highlight
    constexpr uint16_t CHICK_ORG= 0xFD20;  // Orange beak
    constexpr uint16_t CHICK_DRK= 0xE520;  // Dark yellow shadow
    constexpr uint16_t CHICK_EYE= 0x0000;  // Black eyes
    constexpr uint16_t CHICK_CHK= 0xFB2C;  // Pink cheeks
    constexpr uint16_t CHICK_WHT= 0xFFFF;  // Eye whites
    
    // Heart colors
    constexpr uint16_t HEART_RED= 0xF800;  // Bright red
    constexpr uint16_t HEART_PNK= 0xF81F;  // Pink
    constexpr uint16_t HEART_LIT= 0xFC10;  // Light red/coral
    
    // Background
    constexpr uint16_t BG       = 0x0000;  // Black
    
    // Sparkle
    constexpr uint16_t SPARKLE  = 0xFFFF;  // White
}

using namespace Colors;

// ============================================================================
// Egg Sprite Data (40x48 - stored compactly)
// Full pristine egg without cracks
// ============================================================================

static const uint16_t eggSprite[40*48] PROGMEM = {
    // Row 0-3: Top curve of egg (narrower)
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 4-7: Upper egg body
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 8-15: Middle egg (widest part)
    TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,
    TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,
    TRANS,EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,
    // Row 16-23: Middle-lower egg
    EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,
    EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_SPEC,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,
    TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,
    TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,
    // Row 24-31: Lower egg (narrowing)
    TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,
    TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 32-39: Bottom narrowing
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 40-47: Bottom of egg
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_LITE,EGG_LITE,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_MAIN,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,EGG_SHAD,EGG_SHAD,EGG_MAIN,EGG_MAIN,EGG_SHAD,EGG_SHAD,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS
};

// ============================================================================
// Chick Sprite (32x28) - Cute baby chick with big eyes
// ============================================================================

static const uint16_t chickSprite[32*28] PROGMEM = {
    // Row 0-3: Top of head (tuft of feathers)
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_YEL,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_LIT,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 4-7: Upper head
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 8-11: Eyes row
    TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_WHT,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_WHT,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_WHT,CHICK_EYE,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_WHT,CHICK_EYE,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_EYE,CHICK_EYE,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_EYE,CHICK_EYE,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_WHT,CHICK_WHT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,
    // Row 12-15: Cheeks and beak
    TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_CHK,CHICK_CHK,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_CHK,CHICK_CHK,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_CHK,CHICK_CHK,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_CHK,CHICK_CHK,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 16-19: Lower face and neck
    TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_ORG,CHICK_ORG,CHICK_ORG,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 20-23: Body
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_LIT,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_YEL,CHICK_DRK,CHICK_DRK,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_DRK,CHICK_DRK,CHICK_YEL,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,CHICK_YEL,CHICK_DRK,CHICK_DRK,CHICK_DRK,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_DRK,CHICK_DRK,CHICK_DRK,CHICK_YEL,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    // Row 24-27: Wings (folded)
    TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_DRK,CHICK_DRK,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_DRK,CHICK_DRK,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_DRK,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_DRK,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_DRK,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_DRK,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,CHICK_DRK,CHICK_DRK,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_YEL,CHICK_DRK,CHICK_DRK,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS
};

// ============================================================================
// Heart Sprite (8x8) - Simple cute heart
// ============================================================================

static const uint16_t heartSprite[8*8] PROGMEM = {
    TRANS,HEART_RED,HEART_RED,TRANS,TRANS,HEART_RED,HEART_RED,TRANS,
    HEART_RED,HEART_LIT,HEART_RED,HEART_RED,HEART_RED,HEART_LIT,HEART_RED,HEART_RED,
    HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,
    HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,
    TRANS,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,HEART_RED,TRANS,
    TRANS,TRANS,HEART_RED,HEART_RED,HEART_RED,HEART_RED,TRANS,TRANS,
    TRANS,TRANS,TRANS,HEART_RED,HEART_RED,TRANS,TRANS,TRANS,
    TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS,TRANS
};

// ============================================================================
// Crack Patterns (drawn as line segments)
// Format: {startX, startY, endX, endY}
// ============================================================================

static const int8_t crackStage1[][4] = {
    {20, 10, 18, 16},   // Top start
    {18, 16, 22, 22},   // Zigzag
    {22, 22, 19, 28},   // Deep crack
    {-1, -1, -1, -1}
};

static const int8_t crackStage2[][4] = {
    {20, 10, 18, 16},
    {18, 16, 22, 22},
    {22, 22, 19, 28},
    
    // Branch Left
    {18, 16, 12, 18},
    {12, 18, 8, 15},
    
    // Branch Right
    {22, 22, 28, 20},
    {28, 20, 32, 24},
    {-1, -1, -1, -1}
};

static const int8_t crackStage3[][4] = {
    // Stage 1 & 2
    {20, 10, 18, 16},
    {18, 16, 22, 22},
    {22, 22, 19, 28},
    {18, 16, 12, 18},
    {12, 18, 8, 15},
    {22, 22, 28, 20},
    {28, 20, 32, 24},
    
    // Chaos lines
    {19, 28, 14, 32},
    {19, 28, 24, 34},
    {12, 18, 10, 24},
    {28, 20, 30, 14},
    {32, 24, 36, 22},
    
    // Horizontal shatter
    {8, 25, 32, 25},
    {-1, -1, -1, -1}
};

// ============================================================================
// Global Instance
// ============================================================================

EggChickAnimation eggChickAnimation;

// ============================================================================
// Implementation
// ============================================================================

void EggChickAnimation::init(IDisplay* display) {
    _display = display;
    _state = EggChickState::INACTIVE;
}

void EggChickAnimation::start() {
    if (_display == nullptr) return;
    
    _state = EggChickState::EGG_IDLE;
    _stateStartTime = millis();
    _lastFrameTime = millis();
    _frameIndex = 0;
    _wobbleOffset = 0;
    _crackFrame = 0;
    _chickFrame = 0;
    _heartY = 0;
    _heartAlpha = 255;
}

void EggChickAnimation::stop() {
    _state = EggChickState::INACTIVE;
    if (_display) {
        _display->fillScreen(BG);
    }
}

bool EggChickAnimation::isRunning() const {
    return _state != EggChickState::INACTIVE && _state != EggChickState::COMPLETE;
}

bool EggChickAnimation::isComplete() const {
    return _state == EggChickState::COMPLETE;
}

EggChickState EggChickAnimation::getState() const {
    return _state;
}

void EggChickAnimation::transitionTo(EggChickState newState) {
    _state = newState;
    _stateStartTime = millis();
    _frameIndex = 0;
}

void EggChickAnimation::update() {
    if (_display == nullptr || _state == EggChickState::INACTIVE) return;
    
    uint32_t now = millis();
    uint32_t stateElapsed = now - _stateStartTime;
    
    switch (_state) {
        case EggChickState::EGG_IDLE:
            updateIdle();
            // Transition to cracking after wobbling for a bit
            if (stateElapsed > 1500) {
                transitionTo(EggChickState::CRACK_STAGE_1);
            }
            break;
            
        case EggChickState::CRACK_STAGE_1:
        case EggChickState::CRACK_STAGE_2:
        case EggChickState::CRACK_STAGE_3:
            updateCrack();
            break;
            
        case EggChickState::HATCHING:
            updateHatching();
            break;
            
        case EggChickState::CHICK_EMERGE:
            updateChickEmerge();
            break;
            
        case EggChickState::CHICK_LOOK:
            updateChickLook();
            break;
            
        case EggChickState::CHICK_KISS:
            updateChickKiss();
            break;
            
        case EggChickState::HEART_FLOAT:
            updateHeartFloat();
            break;
            
        case EggChickState::COMPLETE:
            // Stay complete until reset
            break;
            
        default:
            break;
    }
}

void EggChickAnimation::updateIdle() {
    uint32_t now = millis();
    if (now - _lastFrameTime >= EggChickConfig::WOBBLE_INTERVAL) {
        _lastFrameTime = now;
        _frameIndex = (_frameIndex + 1) % 4;
        
        // Gentle wobble pattern: 0, 1, 0, -1
        static const int8_t wobblePattern[] = {0, 1, 0, -1};
        _wobbleOffset = wobblePattern[_frameIndex];
    }
}

void EggChickAnimation::updateCrack() {
    uint32_t now = millis();
    uint32_t stateElapsed = now - _stateStartTime;
    
    // Shake more violently as cracks progress
    uint16_t shakeInterval = EggChickConfig::SHAKE_INTERVAL;
    if (_state == EggChickState::CRACK_STAGE_3) {
        shakeInterval = 30; // Faster shake
    }
    
    if (now - _lastFrameTime >= shakeInterval) {
        _lastFrameTime = now;
        _frameIndex = (_frameIndex + 1) % 4;
        
        // More violent shake pattern for cracking
        static const int8_t shakePattern[] = {0, 2, 0, -2};
        _wobbleOffset = shakePattern[_frameIndex];
        
        // Even more violent for stage 3
        if (_state == EggChickState::CRACK_STAGE_3) {
            static const int8_t violentShake[] = {0, 3, -1, -3};
            _wobbleOffset = violentShake[_frameIndex];
        }
    }
    
    // Progress through crack stages
    if (stateElapsed > EggChickConfig::CRACK_DELAY) {
        switch (_state) {
            case EggChickState::CRACK_STAGE_1:
                transitionTo(EggChickState::CRACK_STAGE_2);
                break;
            case EggChickState::CRACK_STAGE_2:
                transitionTo(EggChickState::CRACK_STAGE_3);
                break;
            case EggChickState::CRACK_STAGE_3:
                transitionTo(EggChickState::HATCHING);
                _wobbleOffset = 0;
                break;
            default:
                break;
        }
    }
}

void EggChickAnimation::updateHatching() {
    uint32_t stateElapsed = millis() - _stateStartTime;
    
    // Quick transition to chick emerging
    if (stateElapsed > EggChickConfig::EMERGE_DELAY) {
        transitionTo(EggChickState::CHICK_EMERGE);
    }
}

void EggChickAnimation::updateChickEmerge() {
    uint32_t now = millis();
    uint32_t stateElapsed = now - _stateStartTime;
    
    // Animate chick popping up
    if (now - _lastFrameTime >= 100) {
        _lastFrameTime = now;
        if (_chickFrame < 10) {
            _chickFrame++;
        }
    }
    
    if (stateElapsed > 1000) {
        transitionTo(EggChickState::CHICK_LOOK);
        _chickFrame = 10;  // Fully emerged
    }
}

void EggChickAnimation::updateChickLook() {
    uint32_t now = millis();
    uint32_t stateElapsed = now - _stateStartTime;
    
    // Simple look animation (head tilt simulation)
    if (now - _lastFrameTime >= EggChickConfig::LOOK_INTERVAL) {
        _lastFrameTime = now;
        _frameIndex = (_frameIndex + 1) % 4;
    }
    
    if (stateElapsed > 1200) {
        transitionTo(EggChickState::CHICK_KISS);
    }
}

void EggChickAnimation::updateChickKiss() {
    uint32_t stateElapsed = millis() - _stateStartTime;
    
    if (stateElapsed > EggChickConfig::KISS_DELAY) {
        transitionTo(EggChickState::HEART_FLOAT);
        _heartY = 0;
        _heartAlpha = 255;
    }
}

void EggChickAnimation::updateHeartFloat() {
    uint32_t now = millis();
    uint32_t stateElapsed = now - _stateStartTime;
    
    // Slower, majestic float
    if (now - _lastFrameTime >= EggChickConfig::HEART_INTERVAL) {
        _lastFrameTime = now;
        _heartY++; // Float up
        
        // Pulse alpha for "gift" feeling
        if (stateElapsed < 2000) {
            _heartAlpha = 255;
        } else {
            // Gradual fade out after 2 seconds
            if (_heartAlpha > 10) _heartAlpha -= 5;
        }
    }
    
    // Complete after heart has floated away and fade is done
    if (stateElapsed > 5000) {
        transitionTo(EggChickState::COMPLETE);
    }
}

void EggChickAnimation::render() {
    if (_display == nullptr || _state == EggChickState::INACTIVE) return;
    
    // Clear screen
    clearScreen();

    // Flash white on new crack events to emphasize force (Visual Impact)
    if ((_state == EggChickState::CRACK_STAGE_1 || 
         _state == EggChickState::CRACK_STAGE_2 || 
         _state == EggChickState::CRACK_STAGE_3 ||
         _state == EggChickState::HATCHING) && 
        (millis() - _stateStartTime < 60)) {
        _display->fillScreen(0xFFFF); // White flash
        return;
    }
    
    // Determine what to draw based on state
    uint8_t crackStage = 0;
    bool showChick = false;
    bool showHeart = false;
    
    switch (_state) {
        case EggChickState::EGG_IDLE:
            crackStage = 0;
            break;
            
        case EggChickState::CRACK_STAGE_1:
            crackStage = 1;
            break;
            
        case EggChickState::CRACK_STAGE_2:
            crackStage = 2;
            break;
            
        case EggChickState::CRACK_STAGE_3:
            crackStage = 3;
            break;
            
        case EggChickState::HATCHING:
            crackStage = 3;
            break;
            
        case EggChickState::CHICK_EMERGE:
        case EggChickState::CHICK_LOOK:
        case EggChickState::CHICK_KISS:
            showChick = true;
            break;
            
        case EggChickState::HEART_FLOAT:
            showChick = true;
            showHeart = true;
            break;
            
        case EggChickState::COMPLETE:
            showChick = true;
            break;
            
        default:
            break;
    }
    
    // Draw egg (if not showing chick)
    if (!showChick) {
        drawEgg(_wobbleOffset, 0, crackStage);
    } else {
        // Draw broken egg shell bottom
        int16_t shellY = EggChickConfig::EGG_Y + 26;
        int16_t shellX = EggChickConfig::EGG_X;
        
        // Simple broken shell - just bottom portion of egg
        for (int y = 26; y < 48; y++) {
            for (int x = 0; x < 40; x++) {
                uint16_t color = pgm_read_word(&eggSprite[y * 40 + x]);
                if (color != TRANS) {
                    _display->drawPixel(shellX + x, EggChickConfig::EGG_Y + y, color);
                }
            }
        }
        
        // Draw jagged top edge of broken shell
        int16_t jaggedY = EggChickConfig::EGG_Y + 26;
        for (int16_t x = 6; x < 34; x++) {
            int8_t offset = (x % 4 < 2) ? 0 : -2;
            _display->drawPixel(shellX + x, jaggedY + offset, EGG_MAIN);
            _display->drawPixel(shellX + x, jaggedY + offset + 1, EGG_SHAD);
        }
        
        // Draw chick
        int16_t chickY = EggChickConfig::CHICK_Y;
        
        // Animate emergence
        if (_state == EggChickState::CHICK_EMERGE) {
            chickY = EggChickConfig::CHICK_Y + (10 - _chickFrame) * 2;
        }
        
        // Small bounce for look animation
        if (_state == EggChickState::CHICK_LOOK) {
            static const int8_t lookBounce[] = {0, -1, 0, 1};
            chickY += lookBounce[_frameIndex];
        }
        
        drawChick(0, chickY, 0);
        
        // Draw heart if needed
        if (showHeart) {
            int16_t heartX = EggChickConfig::HEART_START_X;
            int16_t heartY = EggChickConfig::HEART_START_Y - _heartY;
            
            // Slight side-to-side float
            heartX += (_heartY % 6) < 3 ? 1 : -1;
            
            drawHeart(heartX, heartY, _heartAlpha);

            // "Gift" confetti / love atmosphere particles
            uint32_t t = millis();
            
            // Only show confetti if alpha is high (animation active)
            if (_heartAlpha > 50) {
                // Little shimmering dots
                if ((t / 300) % 2 == 0) _display->drawPixel(heartX - 8, heartY + 2, HEART_PNK);
                if ((t / 250) % 2 == 0) _display->drawPixel(heartX + 10, heartY + 4, HEART_RED);
                if ((t / 400) % 2 == 0) _display->drawPixel(heartX - 3, heartY - 5, SPARKLE);
                
                // Rising sparkles background
                int16_t pY1 = (t / 60) % 24;
                int16_t pY2 = ((t + 100) / 70) % 24;
                _display->drawPixel(heartX + 6, heartY + 8 + pY1, (t%200<100)?HEART_LIT:TRANS);
                _display->drawPixel(heartX - 6, heartY + 4 + pY2, (t%200>100)?HEART_PNK:TRANS);
            }
        }
    }
}

void EggChickAnimation::clearScreen() {
    _display->fillScreen(BG);
}

void EggChickAnimation::drawSprite(const uint16_t* sprite, int16_t x, int16_t y,
                                    uint8_t width, uint8_t height) {
    for (uint8_t py = 0; py < height; py++) {
        int16_t screenY = y + py;
        if (screenY < 0 || screenY >= 64) continue;
        
        for (uint8_t px = 0; px < width; px++) {
            int16_t screenX = x + px;
            if (screenX < 0 || screenX >= 64) continue;
            
            uint16_t color = pgm_read_word(&sprite[py * width + px]);
            if (color != TRANS) {
                _display->drawPixel(screenX, screenY, color);
            }
        }
    }
}

void EggChickAnimation::drawEgg(int8_t xOffset, int8_t yOffset, uint8_t crackStage) {
    int16_t x = EggChickConfig::EGG_X + xOffset;
    int16_t y = EggChickConfig::EGG_Y + yOffset;
    
    // Draw egg sprite
    drawSprite(eggSprite, x, y, 40, 48);
    
    // Draw crack overlays
    if (crackStage > 0) {
        drawCrackOverlay(crackStage);
    }
}

void EggChickAnimation::drawCrackOverlay(uint8_t stage) {
    const int8_t (*cracks)[4] = nullptr;
    
    switch (stage) {
        case 1: cracks = crackStage1; break;
        case 2: cracks = crackStage2; break;
        case 3: cracks = crackStage3; break;
        default: return;
    }
    
    int16_t baseX = EggChickConfig::EGG_X + _wobbleOffset;
    int16_t baseY = EggChickConfig::EGG_Y;
    
    // Draw crack lines
    for (int i = 0; cracks[i][0] != -1; i++) {
        int16_t x1 = baseX + cracks[i][0];
        int16_t y1 = baseY + cracks[i][1];
        int16_t x2 = baseX + cracks[i][2];
        int16_t y2 = baseY + cracks[i][3];
        
        // High visibility cracks (black core, grey edge)
        _display->drawLine(x1, y1, x2, y2, 0x0000);
        
        // Thicker lines for visibility
        _display->drawLine(x1+1, y1, x2+1, y2, 0x0000);
        _display->drawLine(x1, y1+1, x2, y2+1, 0x0000);
    }
}

void EggChickAnimation::drawChick(int8_t xOffset, int8_t yOffset, uint8_t frame) {
    int16_t x = EggChickConfig::CHICK_X + xOffset;
    int16_t y = yOffset;  // yOffset is already the full Y position
    
    drawSprite(chickSprite, x, y, 32, 28);
}

void EggChickAnimation::drawHeart(int16_t x, int16_t y, uint8_t brightness) {
    // Simple heart with optional dimming
    for (uint8_t py = 0; py < 8; py++) {
        for (uint8_t px = 0; px < 8; px++) {
            uint16_t color = pgm_read_word(&heartSprite[py * 8 + px]);
            if (color != TRANS) {
                // Apply brightness reduction
                if (brightness < 255) {
                    uint8_t r = ((color >> 11) & 0x1F) * brightness / 255;
                    uint8_t g = ((color >> 5) & 0x3F) * brightness / 255;
                    uint8_t b = (color & 0x1F) * brightness / 255;
                    color = (r << 11) | (g << 5) | b;
                }
                
                int16_t screenX = x + px;
                int16_t screenY = y + py;
                
                if (screenX >= 0 && screenX < 64 && screenY >= 0 && screenY < 64) {
                    _display->drawPixel(screenX, screenY, color);
                }
            }
        }
    }
    
    // Add sparkles around heart
    if (brightness > 128) {
        uint32_t t = millis() / 100;
        for (int i = 0; i < 3; i++) {
            int8_t sx = x + 4 + ((t + i * 7) % 12) - 6;
            int8_t sy = y + 4 + ((t + i * 11) % 10) - 5;
            if (sx >= 0 && sx < 64 && sy >= 0 && sy < 64) {
                _display->drawPixel(sx, sy, SPARKLE);
            }
        }
    }
}
