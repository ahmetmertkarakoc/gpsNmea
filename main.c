#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "nmea.h"
#include "info.h"
#include "buffer.h"

#define BUFFER_SIZE 1024

static char ringBufferData[BUFFER_SIZE];
static RingBuffer uart1Buffer;
nmeaPARSER parser;
nmeaINFO info;

void RingBuffer_Init(void) {
    ring_buffer_init(&uart1Buffer, ringBufferData, BUFFER_SIZE);
}

void UART0_Send(const char *str) {
    while (*str) {
        MAP_UARTCharPut(UART0_BASE, *str++);
    }
}

void UART0IntHandler(void) {
    uint32_t status = MAP_UARTIntStatus(UART0_BASE, true);
    MAP_UARTIntClear(UART0_BASE, status);

    while (MAP_UARTCharsAvail(UART0_BASE)) {
        char receivedChar = MAP_UARTCharGetNonBlocking(UART0_BASE);
        MAP_UARTCharPut(UART0_BASE, receivedChar); // Gelen veriyi geri gönder (echo)
    }
}

void UART1IntHandler(void) {
    uint32_t status = MAP_UARTIntStatus(UART1_BASE, true);
    MAP_UARTIntClear(UART1_BASE, status);

    while (MAP_UARTCharsAvail(UART1_BASE)) {
        char receivedChar = MAP_UARTCharGetNonBlocking(UART1_BASE);
        ring_buffer_writeByte(&uart1Buffer, &receivedChar);
    }
}



void ProcessNMEAData(void) {
    unsigned char buffer[BUFFER_SIZE];
    int bytesRead = ring_buffer_read(&uart1Buffer, buffer, sizeof(buffer));

    if (bytesRead > 0) {
        int parsedBytes = nmea_parser_push(&parser, (char *)buffer, bytesRead);

        if (parsedBytes > 0) {
            int parseResult = nmea_parse(&parser, (char *)buffer, parsedBytes, &info);

            if (parseResult > 0) {
                char debugMsg[128];
                snprintf(debugMsg, sizeof(debugMsg), "Lat: %.6f, Lon: %.6f, Alt: %.2f\n",
                         info.lat, info.lon, info.elv);
                UART0_Send(debugMsg);
            } else {
                UART0_Send("NMEA Parse Failed\n");
            }
        } else {
            UART0_Send("NMEA Parser Push Failed\n");
        }
    }
}

void InitializeUART1(void) {
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    MAP_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    MAP_UARTConfigSetExpClk(UART1_BASE, MAP_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    MAP_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
    MAP_IntEnable(INT_UART1);
}

void InitializeUART0(void) {
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    MAP_UARTConfigSetExpClk(UART0_BASE, MAP_SysCtlClockGet(), 115200,
                            (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    MAP_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);
    MAP_IntEnable(INT_UART0);
}

int main(void) {
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    InitializeUART0();
    InitializeUART1();
    RingBuffer_Init();

    MAP_IntMasterEnable();

    // Parse buffer size ayarýný artýrýyoruz
    nmea_property()->parse_buff_size = 2048;

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    UART0_Send("System Initialized\n");

    while (1) {
        ProcessNMEAData();
    }

    nmea_parser_destroy(&parser);
    return 0;
}
