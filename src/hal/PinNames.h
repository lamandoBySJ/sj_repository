/*
 * Copyright (c) 2019-2020 SparkFun Electronics
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef MBED_PINNAMES_H
#define MBED_PINNAMES_H

//#include "am_bsp.h"
//#include "objects_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define NC_VAL (int)0xFFFFFFFF

typedef enum {
    PIN_INPUT = 0,
    PIN_OUTPUT
} PinDirection;

typedef enum {
    Input           = 0x01,
    Output          = 0x02,
    Pullup          = 0x04,
    InputPullup     = 0x05,
    Pulldown        = 0x08,
    InputPulldown   = 0x09,
    OpenDrain       = 0x10,
    OutputOPenDrain = 0x12,
    Special         = 0xF0,
    Funcation1      = 0x00,
    Funcation2      = 0x20,
    Funcation3      = 0x40,
    Funcation4      = 0x60,
    Funcation5      = 0x80,
    Funcation6      = 0xA0,
    Analog          = 0xC0
} PinMode;

typedef enum
{
    // Digital naming
    IO0 = 0,
    IO1 = 1,
    IO2 = 2,
    IO3 = 3,
    IO4 = 4,
    IO5 = 5,
    IO6 = 6,
    IO7 = 7,
    IO8 = 8,
    IO9 = 9,
    IO10 = 10,
    IO11 = 11,
    IO12 = 12,
    IO13 = 13,
    IO14 = 14,
    IO15 = 15,
    IO16 = 16,
    IO17 = 17,
    IO18 = 18,
    IO19 = 19,
    IO20 = 20,
    IO21 = 21,
    IO22 = 22,
    IO23 = 23,
    IO24 = 24,
    IO25 = 25,
    IO26 = 26,
    IO27 = 27,
    IO28 = 28,
    IO29 = 29,
    IO30 = 30,
    IO31 = 31,
    IO32 = 32,
    IO33 = 33,
    IO34 = 34,
    IO35 = 35,
    IO36 = 36,
    IO37 = 37,
    IO38 = 38,
    IO39 = 39,

    IN34 = IO34,
    IN35 = IO35,
    IN36 = IO36,
    IN37 = IO37,
    IN38 = IO38,
    IN39 = IO39,

     // I2C
    I2C_SCL_DBG = IO22,
    I2C_SDA_DBG = IO23,
    ID_SD = IO32,
    ID_SC = IO33,

    VSPI_SS   = IO18,
    VSPI_MOSI = IO27,
    VSPI_MISO = IO19,
    VSPI_SCK  = IO5,

    //UART
    U0TXD = IO1,
    U0RXD = IO3,
    U1TXD = IO12,
    U1RXD = IO13,
    // Not connected
    NC = NC_VAL
} PinName;

#define STDIO_UART_TX USBTX
#define STDIO_UART_RX USBRX

#ifdef __cplusplus
}
#endif

#endif
