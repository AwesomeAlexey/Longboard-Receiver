//
// Created by alex on 25.04.2021.
//

#include "turn-indicators.h"
#include "stm32f1xx_ll_tim.h"
#include "ws2812.h"

/** Values of Red, Green, and Blue channels` brightness
 * On the maximum brightness to set a warm yellow color
 */
#define RED_MAX     255
#define GREEN_MAX   112
#define BLUE_MAX    0

/// Count of the led in one turning side. Obviously, same for left and right side so defined only once
#define LED_IN_ONE_SIDE 8

///*  Prescaler = 119, Reload Value = 999 */


static void SetLevel(uint8_t pixel, uint8_t level);
static void SetPixelLevel(uint8_t pixel, uint8_t level);
static void SwitchOffSide(Side side);
static void ManageSignals(void);




/// Global static data structure set a current state of the signals. Fields are described in the signals.h
static ConfigStruct state = {
    .currentPix   = 0,
    .currentLevel = 0,
    .delayCycles  = 0,
    .phase        = UPWARD,
    .side         = NONE
};

/**
 * Initialization of the signals.
 * Setting a stop signal brightness (in 0-5 level notation)
 * And enable interruption from the timer to manage signals
 */
void SignalsInit(void){
    LL_TIM_EnableIT_UPDATE(TIM4);
//    LL_TIM_EnableCounter(TIM4);
    SetBreakLights(0); // 0-1-2-5
    SetIndicators(NONE);
}

/**
 * Switching off entire side (left, right or both)
 * Might be used at the switching of the signal or at the switching one signal to the other
 * @param side - enum type means that side must be switched off, enum is described in the signals.h
 * May be LEFT, RIGHT, BOTH or NONE value.
 * Call this function with NONE value is, obviously, meaningless
 */
static void SwitchOffSide(Side side){
    switch(side){
        case LEFT:
            for(int i = 0; i < LED_IN_ONE_SIDE; i++){
                SetPixelLevel(i, 0);
            }
            break;
        case RIGHT:
            for(int i = 0; i < LED_IN_ONE_SIDE; i++){
                SetPixelLevel(i + 16, 0);
            }
            break;
        case BOTH:
            for(int i = 0; i < LED_IN_ONE_SIDE; i++){
                SetPixelLevel(i, 0);
                SetPixelLevel(i + 16, 0);
            }
            break;
        case NONE:
        default:
            break;
    }
}

/**
 * Switch on signal (left, right or both) or switch off (if side == NONE)
 * If side is the same as current side in a global state structure then do nothing
 * If new side is a different than disabling timer, configuring switching off both signals
 * Configuring a new state for a new signal
 * And, if new side is not none then enabling timer
 *
 * @param side - side that should be switched on
 * enum described in the signals.h
 * May be LEFT, RIGHT, BOTH, or NONE value. NONE value means that signals must be switched off
 */
void SetIndicators(Side side) {

    if(state.side == side){
        return;
    }

    LL_TIM_DisableCounter(TIM4);

    state.side = side;

    SwitchOffSide(BOTH);
    WS2812_refresh();

    state.currentPix = 0;
    state.currentLevel = 0;
    state.delayCycles = 0;
    state.phase = UPWARD;

    if (state.side != NONE){
        LL_TIM_EnableCounter(TIM4);
    }
}

/**
 * Set stop signal brightness level
 * Receive level from 0 to 5
 *
 * @param level - level of the stop-signal brightness. May be 0-5
 */
void SetBreakLights(uint8_t level){

    if(level > 5)
        return;     // ASSERT LITE VERSION :)

    for(int i = LED_IN_ONE_SIDE; i < LED_IN_ONE_SIDE*2; i++){
        WS2812_setPixel(level*255/5, 0, 0, i);
    }
    WS2812_refresh();
}

/**
 * Set brightness level of the side pixel in 20-level notation
 *
 * Use defined values of the RGB channels that configure the color
 * Proportionally reduce that values into 20 degreeses
 *
 * @param pixel - pixel, brightness of which must be set
 * @param level - level of the brightness (0-20)
 */
static void SetPixelLevel(uint8_t pixel, uint8_t level){

    if(level > 20)
        return;     // ASSERT LITE VERSION :)

    WS2812_setPixel(level*RED_MAX/20,
                    level*GREEN_MAX/20,
                    level*BLUE_MAX/20,
                    pixel);
}

/**
 * Manage what exactly pixel in the global addressing space should be changed
 * Receive pixel position of the one side signal in the local coordinates
 * And call SetPixelLevel function with the exactly global address of the pixel using a .side field of the global state config
 *
 * For example, we want to set second pixel on the right signal
 * We have 24 pixels at all and 8 pixels in the each sides
 * Most left pixel is 0th, most right pixel is 23rd
 * So second pixel in the right signal (local coordinates begin in the center and go to the sides)
 * will be 23 - 8 + 2 = 17th pixel in the global addressing space
 *
 * Set brightness in a 0-20 level notation, more details in the description of the SetPixelLevel function
 * @param pixel - current pixel in the local coordinates that should be managed
 * @param level - new level of the brightness
 */
static void SetLevel(uint8_t pixel, uint8_t level){

    switch (state.side){
        case LEFT:
            SetPixelLevel(7 - pixel, level);
            break;
        case RIGHT:
            SetPixelLevel(pixel + 16, level);
            break;
        case BOTH:
            SetPixelLevel(7 - pixel, level);
            SetPixelLevel(pixel + 16, level);
            break;
        case NONE:
            break;
    }
}


/**
 * Handler of the signal
 *
 * Called by a timer interruption and manage signals using global state structure
 * Structure is described in the signals.h file
 *
 *
 * Have 3+ phases
 *
 * 1 - Upward. Pixels fills in the side one by one from center to the side
 * Each pixel's brightness are increasing softly with 21 (0-20) levels
 *
 * 2 - Top delay. For better visual signals is waiting for some time in the full bright phase
 * Made by skipping a few iterations number of which is set at the end of the UPWARD phase
 *
 * 3 - Downward. All pixels` brightness simultaneously decreasing with small delays between decreasing
 * Delay added to make it more soft and comfortable without changing any timer`s settings
 *
 * 4 - Bottom delay. This is the + of 3+ phases :)
 * Again, added for a better visual effect
 * Works just like top delay, but use less amount (which is set in the end of the DOWNWARD phase)
 *
 * After bottom delay everything happening again from the first phase
 *
 */
static void ManageSignals(void){
    switch (state.phase){
        case UPWARD:
            state.currentLevel++;
            if(state.currentLevel > 20){
                state.currentLevel = 0;
                state.currentPix++;
            }
            if(state.currentPix == LED_IN_ONE_SIDE){
                state.currentPix = 0;
                state.phase = TOP_DELAY;
                state.delayCycles = 50;
                return;
            }
            SetLevel(state.currentPix, state.currentLevel);
            break;

        case TOP_DELAY:
            state.delayCycles--;
            if(state.delayCycles <= 0){
                state.phase = DOWNWARD;
                state.currentLevel = 20;
                state.delayCycles = 2;
            }
            break;

        case DOWNWARD:
            state.delayCycles--;
            if(state.delayCycles <= 0){
                state.currentLevel--;
                if(state.currentLevel < 0){
                    state.phase = BOTTOM_DELAY;
                    state.delayCycles = 20;
                    return;
                }
                for(int i = 0; i < LED_IN_ONE_SIDE; i++) {
                    SetLevel(i, state.currentLevel);
                }
                state.delayCycles = 3;
            }
            break;

        case BOTTOM_DELAY:
            state.delayCycles--;
            if(state.delayCycles <= 0){
                state.phase = UPWARD;
                return;
            }
            break;
    }
    WS2812_refresh();
}

/**
 * Timer 4 update interrupt
 *
 * Occurs 533,33 times per second (every 1.875ms)
 * That time set in the Timer4 initialization in the main.c
 *
 * Must be chosen with all timings in mind:
 *
 * On every interrupt signals are being managed.
 * One filling contains:
 *
 * One side contain LED_IN_ON_SIDE led (8 in this particular case)
 * Brightness of each led must be increased in 21 levels (0-20)
 * After that delay occurred. Delay requires some iterations to be skipped
 * After that all led's brightness decreasing with some delay. As before, delay require a few iterations to be skipped
 * And then another delay occurred. As before, few iterations will be skipped
 *
 * So, for example, if we have 8 led in one side and period of iteration is 1.875 ms
 *
 * Brightness increasing:
 *  8 * 21 * 1.875 = 315 ms
 * Top delay, 50 cycles:
 *  50 * 1.875 = 93.75 ms
 * Brightness decreasing, 3 cycles for each level changing, + level changing itself:
 *  21 * 4 * 1.875 = 157.5 ms
 * Bottom delay, 20 cycles:
 *  20 * 1.875 = 37.5 ms
 *
 * Full period: 603.75 ms or about 0.6s.
 *
 * You can change delays using defined constants in the top of the file
 * (Or ~~magic numbers~~ in the code if I haven't already define that constants)
 *
 * Keeping that timing in the mind you can set the period of timer interruption that will fit your personal wishes
 *
 */
void TIM4_IRQHandler(void){
    LL_TIM_ClearFlag_UPDATE(TIM4);
    ManageSignals();
}

