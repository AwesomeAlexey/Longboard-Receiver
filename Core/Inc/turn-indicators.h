//
// Created by alex on 25.04.2021.
//

#ifndef RECEIVER_TURN_INDICATORS_H
#define RECEIVER_TURN_INDICATORS_H

#include <stdint.h>
#include "drivers.h"

/**
 * enum Phase shows in what phase the system at the moment
 *
 * 1 - Upward. Pixels fills in the side one by one from center to the side
 * Each pixel's brightness are increasing softly with 21 (0-20) levels
 *
 * 2 - Top delay. For better visual effect signals is waiting for some time in the full bright phase
 *
 * 3 - Downward. All pixels` brightness simultaneously decreasing with small delays between decreasing
 *
 * 4 - Bottom delay. As top delay, added for a better visual effect

 */
typedef enum{
    UPWARD,
    TOP_DELAY,
    DOWNWARD,
    BOTTOM_DELAY
} Phase;

/**
 * enum Side shows what side of the signals is active now - left, right, both, or none of them
 */
typedef enum{
    LEFT,
    RIGHT,
    BOTH,
    NONE
} Side;

/**
 * Config Structure holds the current state of the whole system
 *
 * .currentPix - number of the pixel that is being managing at the current moment
 * Number is set in the local coordinates (increasing from center to sides)
 * Because left and rights sides are same, the actual position of the pixel in the
 * global coordinates might be found later in the function SetLevel
 * Also, because of only one such field we can manage both sides simultaneously as well without any additions to the structure
 *
 * .currentLevel - level of the brightness of the current pixel.
 * Level is set in the range of 0-20 and mean only brightness level without affecting of the color
 * Actual R, G, and B colors is set in the SetPixelLevel as a proportional parts of the values defined in the signals.c
 * That values mean actual warm-yellow color on the maximum brightness
 *
 * .delayCycles - number of the cycles that must be skipped to make a delay without delaying an entire program
 * See ManegeSignals description in the signals.c for more details
 *
 * .phase - enum phase means the current phase (pretty obvious, yes)
 * .side - enum side means the current side of the signal (also pretty obvious)
 *
 * phase and side enums` description starts on the line 11 of this file
 */
typedef struct {
    int currentPix;
    int currentLevel;   // Brightness of the current pixel
    int delayCycles;    // Count of the cycles should be skipped
    Phase phase;
    Side side;

} ConfigStruct;

void SetIndicators(Side side);
void SetBreakLights(uint8_t level);

void SignalsInit(void);

#endif //RECEIVER_TURN_INDICATORS_H
