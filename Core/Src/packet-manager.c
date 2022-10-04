//
// Created by Alex on 30.05.2021.
//

#include "packet-manager.h"
#include "nrf24l01.h"
#include "vesc.h"

//-----------------------------------------------------

#define SIDE_DELTA_VALUE 1000 // Value presents how far you have to move joystick to enable turn indicator

#define FLUCTUATION_DELTA 10        // Delta for vertical fluctuation around the center
                                    // Required because of ADC value fluctuation

#define ACCELERATION_THRESHOLD  ((255 / 2) + FLUCTUATION_DELTA)
#define BREAKING_THRESHOLD         ((255 / 2) - FLUCTUATION_DELTA)

#define FULL_BREAK_THRESHOLD    (255 / 4)


#define LEFT_SIDE_THRESHOLD     ((4095 / 2) - SIDE_DELTA_VALUE) // Left side threshold
#define RIGHT_SIDE_THRESHOLD    ((4095 / 2) + SIDE_DELTA_VALUE) // Right side threshold

//-----------------------------------------------------

static volatile Packet globalPacket = {
        .ppm = 128,
        .side = NONE,
        .breakLightLevel = 2
};

void SetPPM(uint8_t newPPM){

    globalPacket.ppm = newPPM;

}

void SetIndicatorsInPacket(Side newSide){

    globalPacket.side = newSide;

}

void SetupPacket(uint16_t yAxisValue, uint16_t xAxisValue){

    uint8_t ppmValue = yAxisValue >> 4;  // ADC value  is 12-bit, we just take major 8-bit
    globalPacket.ppm = ppmValue;

    if(ppmValue > ACCELERATION_THRESHOLD){
        SetBreakLights(1);
    }
    else if(ppmValue < FULL_BREAK_THRESHOLD){
        SetBreakLights(3);
    }
    else{
        SetBreakLights(2); // Not full breaking, but breaking anyway
    }

    if(xAxisValue < LEFT_SIDE_THRESHOLD){
        globalPacket.side = LEFT;
    }
    else if (xAxisValue > RIGHT_SIDE_THRESHOLD){
        globalPacket.side = RIGHT;
    } else {
        globalPacket.side = NONE;
    }

}

//-----------------------------------------------------

Packet DecodePacketFromBytes(const uint8_t *source){
    Packet decoded = {0};

    decoded.ppm = source[0];
    decoded.side = (Side) source[1];
    decoded.breakLightLevel = source[2];

    return decoded;
}

void EncodeBytesFromPacket(Packet sourcePacket, uint8_t *destination){

    destination[0] = sourcePacket.ppm;
    destination[1] = (uint8_t) sourcePacket.side;
    destination[2] = sourcePacket.breakLightLevel;

}

//-----------------------------------------------------


void SendPacket(void){

    uint8_t buffer[3] = {0};
    EncodeBytesFromPacket(globalPacket, buffer);
    NRF24_Send(buffer);

}

void ManageReceivedPacket(Packet received){

    VESC_SetPPM(received.ppm);

    SetIndicators(received.side);
    SetBreakLights(received.breakLightLevel);

}



