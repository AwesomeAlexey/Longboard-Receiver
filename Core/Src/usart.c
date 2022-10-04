//
// Created by alex on 24.03.2021.
//

#include "usart.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f1xx_ll_usart.h"

/**
 * Basic UART sending function
 * Just send one byte with waiting for a flag to be set
 *
 * @param byte - byte to be sent
 */
void PrintByte(char byte){

    while(!LL_USART_IsActiveFlag_TXE(USART1));
    LL_USART_TransmitData8(USART1, byte);

}

/**
 * Print entire phrase calling PrintByte function for each symbol
 *
 * ATTENTION: THIS FUNCTION IS ONLY FOR A PHRASES WITH \0 IN THE END
 *
 * To print array of the bytes use PrintArray function
 *
 * @param phrase - pointer on the phrase. Phrase must ending with a \0 symbol
 */
void PrintPhrase(const char phrase[]){

    uint32_t size = strlen(phrase);

    for (uint32_t i = 0; i < size; ++i){
        PrintByte(phrase[i]);
    }

}

/**
 * Print phrase and special character after it to go to a new line in the terminal
 * More useful in the debug or communication with a PC
 *
 * @param line - pointer on the phrase. As in PrintPhrase, phrase mus ending with a \0 symbol
 */
void PrintLn(const char line[]){
    PrintPhrase(line);
    PrintByte(0x0A);
    PrintByte(0x0D);
}

/**
 * You can use this function to print array of bytes.
 * In contrast of the PrintPhrase, it's not necessary for array to ending with a \0 symbol
 * But you have to give a size of your array to the function
 *
 * @param array - pointer on the array must be printed
 * @param bytes - number of bytes must be printed
 */
void PrintArray(const uint8_t *array, uint8_t bytes){

    for (uint8_t i = 0; i < bytes; i++){
        PrintByte(array[i]);
        PrintByte(' ');
    }
    PrintByte(0x0A);
    PrintByte(0x0D);

}

int _write(int fd, char* ptr, int len)
{
    (void)fd;
    int i = 0;
    while (ptr[i] && (i < len)) {
        PrintByte((int)ptr[i]);
        if (ptr[i] == '\n') {
            PrintByte((int)'\r');
        }
        i++;
    }
    return len;
}

int fputc(int c, FILE *f){

    PrintByte(c);

}

int fputs(const char *__restrict str, FILE *__restrict f){
    PrintPhrase(str);
}