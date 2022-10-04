//
// Created by alex on 21.04.2021.
//

/**
 * WS2812 it is a chips for addressable led
 * It use a 1-wire protocol there bits are encoding with the following timings:
 *
 * "1" encoding with:
 * 0.9us of high state and 0.35us of low state
 *
 * "0" encoding with:
 * 0.35us of high state and 0.9us of low state
 *
 * Pulse goes one by one for each pixel. After the last pulse for the last pixel delay of minimum 50us of low state is required
 *
 * Colors encoding in the order Green-Red-Blue
 * MSB goes first.
 * So, encoding of the 1 pixel color looks like:
 *
 * G7-G6-...-G1-G0-R7-R6-...-R1-R0-B7-B6-...B1-B0-(50us delay)
 *
 * Chips are connected in serial, so every chip takes "his values" and send other bites to the next one
 * 50ms delay it`s a signal for the chip that transmit is ended, so, again, it must be only after the last bit
 *
 * But, for example, you may want to change the matrix state before all the pixels is already written
 * So you can put that delay in the beginning of the transmit
 * Now every time you will transmit values the delay will be here as well
 *
 * For more information about these chips, please, read a documentation
 */

#include <stm32f1xx_ll_utils.h>
#include "ws2812.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_tim.h"

/**
 * Timing of the high and low side of the pulses
 */

#define HIGH_LEV    53
#define LOW_LEV     21
/**
 * Length of the delay. It may be set as a 0 bits in the array of the pulse.
 * With keeping one pulse period in mind (~1.25us) we can just set it as a 40 empty pulses.
 * 45 is used to be sure
 *
 */
#define DELAY_LEN   45

/**
 * Count of the led in the matrix (linear array)
 */
#define LED_COUNT 24

/**
 * Length of the array that will contain pulses` values
 * 24 bit for every pixel + empty pulses for the delay
 */
#define ARRAY_LEN (LED_COUNT*24+DELAY_LEN)

/**
 * Macro function checking if bit is set in the gaven register (byte)
 * Is being used to find out what pulse width should be set for the particular bit
 */
#define CheckBit(reg, bit) ((reg) & (1 << (bit)))

/**
 * Array contains all pulse wight values for every bit (including delay in the beginning)
 */
static uint16_t matrixArray[ARRAY_LEN] = {0};

/**
 * Set the color of the particular pixel.
 * Function checks every bit in the color values and set the corresponding pulse width in the global array
 *
 * @param rPix - red channel 8-bit value
 * @param gPix - green channel 8-bit value
 * @param bPix - blue channel 8-bit value
 * @param position - position of the pixel in the array, starts from 0
 */
void WS2812_setPixel(uint8_t rPix, uint8_t gPix, uint8_t bPix, uint16_t position){
    for(uint8_t i=0;i<8;i++){
        matrixArray[DELAY_LEN + position*24 + i +  8] = (CheckBit(rPix, 7 - i)) ? HIGH_LEV : LOW_LEV;
        matrixArray[DELAY_LEN + position*24 + i +  0] = (CheckBit(gPix, 7 - i)) ? HIGH_LEV : LOW_LEV;
        matrixArray[DELAY_LEN + position*24 + i + 16] = (CheckBit(bPix, 7 - i)) ? HIGH_LEV : LOW_LEV;
    }
}

/**
 * Refreshing the matrix
 *
 * setPixel function doesn't send any data to a physical matrix - it's only updating values in the array
 * To change the actual colors you should send the pulse sequence to the matrix
 * Timer3 is making PWM signal with the corresponding pulse width and DMA is updating duty cycle values for each bit
 * Algorithm is described with details in the description of WS2812_Init function
 *
 */
void WS2812_refresh (void){
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, ARRAY_LEN);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_TIM_CC_EnableChannel(TIM3, LL_TIM_CHANNEL_CH1);
}

/**
 * WS2812 protocol initialization
 *
 * The algorithm of sending different pulses using pwm is the following:
 *
 * Timer3 is generating PWM signal on the 1st channel
 * Every timer`s period timer makes a request to DMA for a new value for the duty cycle of the 1st channel
 * Then array is run out DMA is making an interrupt - transmit is completed
 * In the interrupt DMA and timer is disabling
 * For the new transmit you need to update a data length value of the DMA and enable both Timer and DMA
 *
 *  In the initialization all peripherals properties is set,
 *  array is filled with zero brightness (not zero pulses` width) and matrix is refreshed
 *
 */
void WS2812_init (void){
    LL_TIM_DisableCounter(TIM3);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, ARRAY_LEN);
    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_3,
                           (uint32_t) &matrixArray,
                           (uint32_t) &(TIM3->CCR1),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH
                           );
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_CIRCULAR);

    LL_TIM_EnableDMAReq_CC4(TIM3);

    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_DisableIT_TE(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_ClearFlag_TC3(DMA1);
    LL_DMA_ClearFlag_TE3(DMA1);
    LL_DMA_ClearFlag_HT3(DMA1);

    for(uint32_t i = 0; i < LED_COUNT; i++){
        WS2812_setPixel(0, 0, 0, i);
    }

    LL_TIM_EnableCounter(TIM3);
    WS2812_refresh();

}

/**
 * On the end of the transmit DMA generate interrupt
 * Here transmit is ended, both Timer and DMA is disabled
 */

void DMA1_Channel3_IRQHandler(void){
    if(LL_DMA_IsActiveFlag_TC3(DMA1)){
        LL_DMA_ClearFlag_TC3(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
        LL_TIM_CC_DisableChannel(TIM3, LL_TIM_CHANNEL_CH1);
    }
}


