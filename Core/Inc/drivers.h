//
// Created by Alex on 30.05.2021.
//

#ifndef RECEIVER_DRIVERS_H
#define RECEIVER_DRIVERS_H

#include "stm32f1xx.h"
#include "stm32f1xx_ll_dma.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_iwdg.h"
#include "stm32f1xx_ll_spi.h"
#include "stm32f1xx_ll_tim.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_utils.h"
#include "stm32f1xx_ll_rcc.h"

/// These defines required because we want to be able to blink LED from anywhere :)

#define LED_GPIO_PORT   GPIOC
#define LED_PIN         LL_GPIO_PIN_13
#define LED_ON()        LL_GPIO_ResetOutputPin(LED_GPIO_PORT, LED_PIN)
#define LED_OFF()       LL_GPIO_SetOutputPin(LED_GPIO_PORT, LED_PIN)
#define LED_TGL()       LL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN)



#endif //RECEIVER_DRIVERS_H
