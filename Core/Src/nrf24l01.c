//
// Created by alex on 24.03.2021.
//

#include "nrf24l01.h"
#include "nrf24l01-private.h"
#include "packet-manager.h"

#include "drivers.h"

#define DELAY_TIME  5000 // Delay time in microseconds for module`s waking up etc

#define TX_ADDR_WIDTH 3     /// Width of the address, may be 3-5 bytes
#define TX_PAYLOAD_WIDTH 3  /// Count of payload bytes, may be 1-32 bytes
uint8_t txAddress[TX_ADDR_WIDTH] = {0xb3, 0xb4, 0x01};  /// Address of transmitter
                                                        /// Must be the same on receiver

/** Delay microseconds function
 *
 * @param delay - count on microseconds
 */
static void MicroDelay(volatile uint32_t delay){

    delay *= (SystemCoreClock / 1000000) / 9;
    while(delay--);

}

/** Initialization of the NRF24L01 module on SPI1
 *
 */
void NRF24_Init(void){
    CE_RESET();
    LED_ON();
    MicroDelay(DELAY_TIME);
    NRF24_WriteReg(CONFIG_REG, CNFG_INIT_VALUE);
    MicroDelay(DELAY_TIME);
    NRF24_WriteReg(EN_AA_REG, EN_AA_INIT_VALUE);
    NRF24_WriteReg(EN_RX_ADDR_REG, EN_RX_ADDR_VALUE);
    NRF24_WriteReg(SETUP_AW_REG, SETUP_AW_VALUE); // 3bytes address
    NRF24_WriteReg(SETUP_RETR_REG, RETR_REG_VALUE); // 1500 us, 15 times
    NRF24_TglFeaturs();
    NRF24_WriteReg(FEATURE_REG, FEATURE_VALUE);
    NRF24_WriteReg(DYNPD_REG, DYNPD_VALUE);
    NRF24_WriteReg(STATUS_REG, STATUS_INIT_VALUE);
    NRF24_WriteReg(RF_CH_REG, RF_CH_REG_VALUE);
    NRF24_WriteReg(RF_SETUP_REG, RF_SETUP_VALUE);
    NRF24_WriteBuf(TX_ADDR_REG, txAddress, TX_ADDR_WIDTH);
    NRF24_WriteBuf(RX_ADDR_P0_REG, txAddress, TX_ADDR_WIDTH);
    NRF24_WriteReg(RX_PW_P0_REG, TX_PAYLOAD_WIDTH);
    NRF24_Enable_RX_Mode();
    LED_OFF();

}

/** Basic SPI transaction function
 *
 * @param data - value that will be transmitted
 * @return - read value from SPI (may be unused)
 */
uint8_t SPI_TransmitReceive(uint8_t data){
    while(!(SPI2->SR & SPI_SR_TXE_Msk));
    LL_SPI_TransmitData8(SPI2, data);
    while(!(SPI2->SR & SPI_SR_RXNE_Msk));
    volatile uint8_t receive = SPI2->DR;
    return receive;
}

/** Reading value of some register
 *
 * @param addr - address of the register on the module
 * @return - read value
 */
uint8_t NRF24_ReadReg(uint8_t addr){
    CS_ON();

    uint8_t data = SPI_TransmitReceive(addr);

    if(addr != STATUS_REG){
        data = SPI_TransmitReceive(0xff);
    }

    CS_OFF();
    return data;
}

/** Writing value to the register.
 *
 * @param addr - address of the register (on the module)
 * @param data - value must be wrote to the register
 */
void NRF24_WriteReg(uint8_t addr, uint8_t data){

    CS_ON();
    addr |= W_REG;

    SPI_TransmitReceive(addr);
    SPI_TransmitReceive(data);

    CS_OFF();
}

/** Function that not really necessary but it`s better to use it. No one really knows what exactly that command do
 *
 */
void NRF24_TglFeaturs(void){
    uint8_t data = ACTIVATE;
    CS_ON();
    SPI_TransmitReceive(data);
    MicroDelay(1);
    data = 0x73; // Some data that do something that need to be done :D
    SPI_TransmitReceive(data);
    CS_OFF();
}

/** Reading the buffer on the module.
 *
 * @param addr - address of the buffer on the module
 * @param buffer - pointer on the local buffer which will get values
 * @param bytes - count of the bytes (length of the buffer)
 *
 */
void NRF24_ReadBuf (uint8_t addr, uint8_t *buffer, uint8_t bytes){
    CS_ON();
    SPI_TransmitReceive(addr);

    for(uint8_t i = 0; i < bytes; i++){
        buffer[i] = SPI_TransmitReceive(0xff);
    }
    CS_OFF();
}

/** Writing a buffer in the module
 *
 * @param addr - address of the buffer on the module
 * @param buffer - pointer on the local buffer with values must be wrote
 * @param bytes - count of bytes must be wrote (length of the buffer)
 */
void NRF24_WriteBuf (uint8_t addr, uint8_t *buffer, uint8_t bytes){

    addr |= W_REG;
    CS_ON();
    SPI_TransmitReceive(addr);
    for (uint8_t i = 0; i < bytes; i++){
        SPI_TransmitReceive(buffer[i]);
    }
    CS_OFF();
}

/** Clear transmit buffer on the module
 *
 */
void NRF24_FlushTX (void){
    uint8_t data = FLUSH_TX;
    CS_ON();
    SPI_TransmitReceive(data);
    MicroDelay(1);
    CS_OFF();
}

/** Clear receive buffer on the module
 *
 */
void NRF24_FlushRX (void){
    uint8_t data = FLUSH_RX;
    CS_ON();
    SPI_TransmitReceive(data);
    MicroDelay(1);
    CS_OFF();
}

/** Enabling receiving mode of the module
 *
 */
void NRF24_Enable_RX_Mode(void){

    uint8_t config =NRF24_ReadReg(CONFIG_REG);

    config |= (1 << PWR_UP) | (1 << PRIM_RX);
    NRF24_WriteReg(CONFIG_REG, config);
    CE_SET();
    MicroDelay(150);
    NRF24_FlushRX();
    NRF24_FlushTX();
}

/** Enabling transmit mode of the module
 *
 */
void NRF24_Enable_TX_Mode(void){
    NRF24_WriteBuf(TX_ADDR_REG, txAddress, TX_ADDR_WIDTH);
    CE_RESET();
    NRF24_FlushRX();
    NRF24_FlushTX();
}

/** Sending bytes to the receiver. Doesn't require count of bytes must be sent because payload width is already defined
 *
 * @param buffer - pointer on the local buffer with values must be sent
 * @return - number of retrying send a packet
 */
uint8_t NRF24_Send(uint8_t *buffer){

    NRF24_Enable_TX_Mode();
    uint8_t reg = NRF24_ReadReg(CONFIG_REG);
    reg |=  (1 << PWR_UP);
    reg &= ~(1 << PRIM_RX);

    NRF24_WriteReg(CONFIG_REG, reg);
    MicroDelay(150);

    NRF24_WriteBuf(WR_TX_PAYLOAD, buffer, TX_PAYLOAD_WIDTH);

    CE_SET();
    MicroDelay(15);
    CE_RESET();

    while(LL_GPIO_IsInputPinSet(IRQ_GPIO_PORT, IRQ_PIN)){

    };

    uint8_t status = NRF24_ReadReg(STATUS_REG);
    if(status & TX_DS){
        LED_TGL();
        NRF24_WriteReg(STATUS_REG, 0x20);
    }
    else if (status & MAX_RT){
        NRF24_WriteReg(STATUS_REG, 0x10);
        NRF24_FlushTX();
    }
    uint8_t observe = NRF24_ReadReg(OBSERVE_TX_REG);
    NRF24_Enable_RX_Mode();
    return observe;
}

/** Receiving packet with waiting a packet in a function
 *
 * @param buffer - pointer on the local buffer which will take received bytes
 * @return - error code. -1 means bad behaviour (timeout), 0 means everything is okay
 */
uint8_t NRF24_Receive(uint8_t *buffer){

    uint8_t error = -1;
    while(LL_GPIO_IsInputPinSet(IRQ_GPIO_PORT, IRQ_PIN));

        uint8_t status = NRF24_ReadReg(STATUS_REG);
        if (status & 0x40) {
            NRF24_ReadBuf(RD_RX_PAYLOAD, buffer, TX_PAYLOAD_WIDTH);
            NRF24_WriteReg(STATUS_REG, 0x40);
            error = 0;
        }
    return error;
}

/** Receiving packet with external trigger interrupt
 * Don`t send a received buffer to the caller (because it is being called from the interrupt handler)
 * But send a received packet to the packet handler from the manager.c
 *
 */

void NRF24_EXTI_Receive(void){
    LED_TGL();
    uint8_t buffer[3] = {0};
    uint8_t status = NRF24_ReadReg(STATUS_REG);
    if (status & 0x40) {
        LL_IWDG_ReloadCounter(IWDG);
        NRF24_ReadBuf(RD_RX_PAYLOAD, buffer, TX_PAYLOAD_WIDTH);
        NRF24_WriteReg(STATUS_REG, 0x40);
        Packet received = DecodePacketFromBytes(buffer);
        ManageReceivedPacket(received);
    }
}

/** External trigger interrupt handler
 * Called then packet is received because of falling edge on the IRQ pin of the module
 * Must be disabled in the TX mode
 */
void EXTI2_IRQHandler(void)
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
        NRF24_EXTI_Receive();
    }
}
