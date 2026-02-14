# Egg & Chick Hatching Animation

## Overview

A cute, expressive LED animation for 64x64 RGB matrix panels showing an egg that cracks progressively, a chick emerging, and blowing a kiss with a floating heart.

## Animation Flow & States

| State | Duration | Description |
|-------|----------|-------------|
| **EGG_IDLE** | ~1.5s | Egg appears centered, wobbles gently (±1px) |
| **CRACK_STAGE_1** | ~0.8s | First small crack appears, egg shakes more |
| **CRACK_STAGE_2** | ~0.8s | Crack spreads, branches appear |
| **CRACK_STAGE_3** | ~0.8s | Large cracks, violent shaking (±3px) |
| **HATCHING** | ~0.6s | Top of egg breaks open |
| **CHICK_EMERGE** | ~1.0s | Chick pops up from broken shell |
| **CHICK_LOOK** | ~1.2s | Chick looks around with small bounce |
| **CHICK_KISS** | ~0.4s | Chick's "kiss" pose |
| **HEART_FLOAT** | ~2.5s | Heart floats up, fades, sparkles trail |
| **COMPLETE** | - | Animation done, ready for next trigger |

**Total Duration:** ~8-9 seconds

## Pixel Art Layout (64×64 Grid)

```
    0         16        32        48        63
    |          |         |         |         |
  0 +----------+---------+---------+---------+
    |                                        |
  8 |          ┌─────────────────┐           |  <- Chick head (32×28)
    |          │   Chick Head    │           |     Position: (16, 8)
 16 |          │    w/ eyes      │           |
    |          │   and beak      │           |
 24 |          └────────┬────────┘           |
    |                   │                    |
 28 |     ╔═════════════╧═════════════╗      |  <- Broken shell top
    |     ║                           ║      |
 36 |     ║   Broken Egg Shell        ║      |  <- Egg body (40×48)
    |     ║   (bottom portion)        ║      |     Position: (12, 10)
 48 |     ║                           ║      |
    |     ╚═══════════════════════════╝      |
 56 |                              ♥         |  <- Heart (8×8)
    |                           floats up    |     Starts: (44, 20)
 63 +----------------------------------------+
```

## Memory Usage

| Component | Size | Storage |
|-----------|------|---------|
| Egg sprite (40×48) | 3,840 bytes | PROGMEM |
| Chick sprite (32×28) | 1,792 bytes | PROGMEM |
| Heart sprite (8×8) | 128 bytes | PROGMEM |
| Crack patterns | ~120 bytes | PROGMEM |
| State variables | ~20 bytes | RAM |
| **Total** | **~5.9 KB PROGMEM, <100 bytes RAM** | |

## Files

```
src/
├── AnimationBase.h              # Common animation structures
├── animations/
│   ├── EggChickAnimation.h      # Animation header
│   └── EggChickAnimation.cpp    # Implementation
└── main.cpp                     # Integration point
```

## Integration into main.cpp

### 1. Add Include
```cpp
#include "animations/EggChickAnimation.h"
```

### 2. Add MQTT Topic
```cpp
const char *MQTT_TOPIC_ANIMATION = "esp32/rgb/animation";
```

### 3. Subscribe to Topic (in connectMQTT)
```cpp
mqttClient.subscribe(MQTT_TOPIC_ANIMATION);
```

### 4. Handle Animation Commands (in parseCommand or mqttCallback)
```cpp
void parseCommand(const String &command) {
    if (command == "animation/egg_chick" || command == "egg" || command == "hatch") {
        eggChickAnimation.init(display);
        eggChickAnimation.start();
        return;
    }
    
    // ... existing command handling
}
```

### 5. Update Loop
```cpp
void loop() {
    // ... existing code ...
    
    // Update animation if running
    if (eggChickAnimation.isRunning()) {
        eggChickAnimation.update();
        eggChickAnimation.render();
    }
    
    // ... rest of loop ...
}
```

## Complete main.cpp Integration Example

```cpp
// At top of file, add include:
#include "animations/EggChickAnimation.h"

// In setup(), after display initialization:
eggChickAnimation.init(display);

// In parseCommand(), add animation trigger:
void parseCommand(const String &command) {
    // Animation commands
    if (command == "egg" || command == "hatch" || command == "animation/egg_chick") {
        eggChickAnimation.start();
        return;
    }
    if (command == "stop") {
        eggChickAnimation.stop();
        drawText(currentText, 255, 255, 255);  // Restore display
        return;
    }
    
    // ... existing TEXT: handling ...
}

// In loop(), add animation update:
void loop() {
    // ... existing WiFi/MQTT checks ...
    
    // Run animation if active (takes priority over text)
    if (eggChickAnimation.isRunning()) {
        eggChickAnimation.update();
        eggChickAnimation.render();
        delay(1);
        return;  // Skip other display updates while animating
    }
    
    // Handle animation completion
    static bool wasRunning = false;
    if (wasRunning && eggChickAnimation.isComplete()) {
        wasRunning = false;
        drawText(currentText, 255, 255, 255);  // Restore text
    }
    if (eggChickAnimation.isRunning()) wasRunning = true;
    
    // ... rest of existing loop ...
}
```

## MQTT Trigger Examples

Send any of these to `esp32/rgb/text` (or dedicated animation topic):

- `egg` - Start egg hatching animation
- `hatch` - Start egg hatching animation  
- `animation/egg_chick` - Start egg hatching animation
- `stop` - Stop animation and return to text display

## Color Palette

| Element | Color | RGB565 Code | Description |
|---------|-------|-------------|-------------|
| Egg Main | Creamy White | `0xFFDE` | Main egg body |
| Egg Highlight | Pure White | `0xFFFF` | Top reflection |
| Egg Shadow | Light Grey | `0xE71C` | Edge shading |
| Egg Speckles | Tan/Brown | `0xC618` | Natural spots |
| Crack Lines | Dark Grey | `0x4208` | Visible cracks |
| Chick Body | Bright Yellow | `0xFFE0` | Main color |
| Chick Highlight | Light Yellow | `0xFFF0` | Face highlight |
| Chick Beak | Orange | `0xFD20` | Cute beak |
| Chick Eyes | Black | `0x0000` | Big cute eyes |
| Chick Cheeks | Pink | `0xFB2C` | Blush spots |
| Heart | Bright Red | `0xF800` | Love heart |
| Sparkles | White | `0xFFFF` | Floating dots |

## Technical Notes

### Performance
- Uses `millis()` based timing - no `delay()` calls
- State machine ensures predictable frame timing
- PROGMEM storage keeps RAM usage minimal
- Direct pixel drawing minimizes overdraw

### Flicker Prevention
- Full screen clear once per frame (in render)
- Draw back-to-front (shell → chick → heart)
- Consistent frame timing (~50-150ms per state)

### Extensibility
The `EggChickAnimation` class can be used as a template for other animations:
- Inherit the state machine pattern
- Use PROGMEM for sprite storage
- Implement `init()`, `start()`, `stop()`, `update()`, `render()` interface
