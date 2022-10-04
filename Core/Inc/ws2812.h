//
// Created by alex on 21.04.2021.
//

#ifndef RECEIVER_WS2812_H
#define RECEIVER_WS2812_H

#include "stdint.h"

void WS2812_setPixel(uint8_t rPix, uint8_t gPix, uint8_t bPix, uint16_t position);
void WS2812_refresh (void);
void WS2812_init    (void);


#endif //RECEIVER_WS2812_H
