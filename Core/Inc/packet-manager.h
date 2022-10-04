//
// Created by Alex on 30.05.2021.
//

#ifndef RECEIVER_PACKET_MANAGER_H
#define RECEIVER_PACKET_MANAGER_H

#include "drivers.h"
#include "turn-indicators.h"


typedef struct{
    uint8_t ppm;
    Side side;
    uint8_t breakLightLevel; // in 0-5 levels notation

} Packet;

void SetPPM(uint8_t newPPM);
void SetIndicatorsInPacket(Side newSide);
void SetupPacket(uint16_t yAxisValue, uint16_t xAxisValue);

Packet DecodePacketFromBytes(const uint8_t *source);
void EncodeBytesFromPacket(Packet sourcePacket, uint8_t *destination);

void SendPacket(void);

void ManageReceivedPacket(Packet received);

#endif //RECEIVER_PACKET_MANAGER_H
