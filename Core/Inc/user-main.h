//
// Created by Alex on 30.05.2021.
//

#ifndef RECEIVER_USER_MAIN_H
#define RECEIVER_USER_MAIN_H

#include "drivers.h"

#include "turn-indicators.h"
#include "packet-manager.h"
#include "nrf24l01.h"
#include "ws2812.h"
#include "vesc.h"
#include "usart.h"

void Boot(void);
void UserSetup(void);
void UserLoop(void);
void IWDGWorkSetup(void);



#endif //RECEIVER_USER_MAIN_H
