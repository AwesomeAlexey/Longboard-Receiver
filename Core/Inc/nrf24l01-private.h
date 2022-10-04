//
// Created by alex on 28.04.2021.
//

#ifndef RECEIVER_NRF24L01_PRIVATE_H
#define RECEIVER_NRF24L01_PRIVATE_H

//--------------------------------------

#include <stdint.h>
#include "drivers.h"
#include "main.h"


//--------------------------------------

#define CS_GPIO_PORT    GPIOA
#define CS_PIN          LL_GPIO_PIN_1
#define CS_ON()         LL_GPIO_ResetOutputPin(CS_GPIO_PORT, CS_PIN)
#define CS_OFF()        LL_GPIO_SetOutputPin(CS_GPIO_PORT, CS_PIN)

#define CE_GPIO_PORT    GPIOA
#define CE_PIN          LL_GPIO_PIN_0
#define CE_SET()        LL_GPIO_SetOutputPin(CE_GPIO_PORT, CE_PIN)
#define CE_RESET()      LL_GPIO_ResetOutputPin(CE_GPIO_PORT, CE_PIN)

#define IRQ_GPIO_PORT   GPIOA
#define IRQ_PIN         LL_GPIO_PIN_2
#define IRQ_STATE()       LL_GPIO_IsInputSet(IRQ_GPIO_PORT, IRQ_PIN)

#define LED_GPIO_PORT   GPIOC
#define LED_PIN         LL_GPIO_PIN_13
#define LED_ON()        LL_GPIO_ResetOutputPin(LED_GPIO_PORT, LED_PIN)
#define LED_OFF()       LL_GPIO_SetOutputPin(LED_GPIO_PORT, LED_PIN)
#define LED_TGL()       LL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN)

//--------------------------------------
// Addresses of registers (in form 0b000a aaaa for read and 0b001a aaa for write)

#define CONFIG_REG          0x00    // Config
#define EN_AA_REG           0x01    // Enable Auto Acknoledgment
#define EN_RX_ADDR_REG      0x02    // Enable RX addresses
#define SETUP_AW_REG        0x03    // Setup addresses width
#define SETUP_RETR_REG      0x04    // Setup auto retry
#define RF_CH_REG           0x05    // Frequency register address
#define RF_SETUP_REG        0x06    // RF Setup address ??
#define STATUS_REG          0x07    // Status register address
#define OBSERVE_TX_REG      0x08    // Transmit observe
#define RX_ADDR_P0_REG      0x0A    // Pipe 0 RX addr
#define RX_ADDR_P1_REG      0x0B    // Pipe 1 RX addr
#define TX_ADDR_REG         0x10    // TX Address
#define RX_PW_P0_REG        0x11    // Pipe 0 payload width
#define RX_PW_P1_REG        0x12    // Pipe 1 payload width
#define FIFO_STATUS_REG     0x17    // FIFO status
#define DYNPD_REG           0x1C    // WTF Is This???
#define FEATURE_REG         0x1D    // To toggle features

//--------------------------------------

// Standard values for initialization:

#define CNFG_INIT_VALUE     0x0A
#define EN_AA_INIT_VALUE    0x01
#define EN_RX_ADDR_VALUE    0x01
#define SETUP_AW_VALUE      0x01    // 3-bytes address
#define RETR_REG_VALUE      0x5F    // delay between retrying 1500 us, 15 times
#define FEATURE_VALUE       0x00
#define DYNPD_VALUE         0x00
#define STATUS_INIT_VALUE   0x70
#define RF_CH_REG_VALUE     50      // Frequency is 2450 Hz (2400 + RF_REG_VALUE)
#define RF_SETUP_VALUE      0x06




//--------------------------------------


// Commands:

#define ACTIVATE        0x50
#define RD_RX_PAYLOAD   0x61    // Read Received Payload
#define WR_TX_PAYLOAD   0xA0    // Write Transmit Payload
#define FLUSH_TX        0xE1    //
#define FLUSH_RX        0xE2    //

//--------------------------------------

//Status flags

#define PRIM_RX         0x00    // RX or TX control (1: PRX, 0: PTX)
#define PWR_UP          0x01    // 1: Power Up, 0: Power Down
#define RX_DR           0x40    // Data ready RX interrupt
#define TX_DS           0x20    // Data Sent TX interrupt
#define MAX_RT          0x10    // Max numbers or retransmits interrupt

//--------------------------------------

#define W_REG           0x20    // Write Register command mask

//--------------------------------------

// Function Prototypes:

uint8_t NRF24_ReadReg   (uint8_t addr);
void    NRF24_WriteReg  (uint8_t addr, uint8_t data);
void    NRF24_TglFeaturs(void);
void    NRF24_ReadBuf   (uint8_t addr, uint8_t *buffer, uint8_t bytes);
void    NRF24_WriteBuf  (uint8_t addr, uint8_t *buffer, uint8_t bytes);
void    NRF24_Enable_RX_Mode   (void);
void    NRF24_TX_Mode   (void);
void    NRF24_Transmit  (uint8_t addr, uint8_t *buffer, uint8_t bytes);
void    NRF24_FlushTX   (void);
void    NRF24_FlushRX   (void);



#endif //RECEIVER_NRF24L01_PRIVATE_H
