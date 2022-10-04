//
// Created by Alex on 30.05.2021.
//

#include "user-main.h"

#define IRQ_STATE()       LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_2)
#define TOGGLE_LED()    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13)

void UserSetup(void){
    LL_SPI_Enable(SPI2);
    NRF24_Init();
    WS2812_init();
    VESC_Init();
    SignalsInit();

    Boot();

    IWDGWorkSetup();
}

void UserLoop(void){

}

/**
 * First Boot function
 * Check if reset occurred because of IWDG
 * If it is, then blinking a led until everything will be fine
 * If it is not, then just waiting for the packet without
 * Waiting for the first interrupt from the NRF24L01 module
 * Receiving the first packet and enabling IWDG and NRF IRQ handler
 * IWDG will be enabled in the main.c after this function
 *
 */

void Boot(void){

    if(LL_RCC_IsActiveFlag_IWDGRST()){
        LL_RCC_ClearResetFlags();
        DEBUG_MESSAGE("IWDG Reset");
        while(IRQ_STATE()){
            TOGGLE_LED();
            LL_mDelay(250);
        }
    } else{
        while(IRQ_STATE());
    }

    uint8_t buffer[3] = {0};
    NRF24_Receive(buffer);

    Packet received = DecodePacketFromBytes(buffer);
    ManageReceivedPacket(received);

    NVIC_EnableIRQ(EXTI2_IRQn);
}

void IWDGWorkSetup(void){
    LL_IWDG_Enable(IWDG);
    LL_IWDG_EnableWriteAccess(IWDG);
    LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_256);
    LL_IWDG_SetReloadCounter(IWDG, 40);
    while (LL_IWDG_IsReady(IWDG) != 1);
    LL_IWDG_ReloadCounter(IWDG);
}
