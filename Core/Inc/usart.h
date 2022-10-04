//
// Created by alex on 24.03.2021.
//

#ifndef RECEIVER_USART_H
#define RECEIVER_USART_H

#include <stdint.h>

#define DEBUG_MESSAGE(message) {PrintPhrase("DEBUG: "); PrintLn(message);}

void PrintByte(char byte);
void PrintPhrase(const char phrase[]);
void PrintLn(const char line[]);
void PrintArray(const uint8_t *array, uint8_t bytes);

#endif //RECEIVER_USART_H
