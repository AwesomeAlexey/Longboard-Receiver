//
// Created by alex on 24.03.2021.
//

#ifndef RECEIVER_NRF24L01_H
#define RECEIVER_NRF24L01_H

#include <stdint.h>
#include "drivers.h"


void    NRF24_Init     (void);
uint8_t NRF24_Send     (uint8_t *buffer);
uint8_t NRF24_Receive  (uint8_t *buffer);

#endif //RECEIVER_NRF24L01_H
