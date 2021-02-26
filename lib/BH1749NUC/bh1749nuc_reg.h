/*
 ******************************************************************************
 * @file    lis2de12_reg.h
 * @author  Sensors Software Solution Team
 * @brief   This file contains all the functions prototypes for the
 *          lis2de12_reg.c driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BH1749NUC_REGS_H
#define BH1749NUC_REGS_H

#ifdef __cplusplus
  extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>

/** @addtogroup LIS2DE12
  * @{
  *
  */

/** @defgroup LIS2DE12_sensors_common_types
  * @{
  *
  */

#ifndef MEMS_SHARED_TYPES
#define MEMS_SHARED_TYPES

/**
  * @defgroup axisXbitXX_t
  * @brief    These unions are useful to represent different sensors data type.
  *           These unions are not need by the driver.
  *
  *           REMOVING the unions you are compliant with:
  *           MISRA-C 2012 [Rule 19.2] -> " Union are not allowed "
  *
  * @{
  *
  */

typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

typedef union{
  int32_t i32bit[3];
  uint8_t u8bit[12];
} axis3bit32_t;

typedef union{
  int32_t i32bit;
  uint8_t u8bit[4];
} axis1bit32_t;


typedef union{
  uint8_t u8bit[2];
  uint16_t i16bit;
  
}rgb1bit16_t;
/**
  * @}
  *
  */

typedef struct{
  uint8_t bit0       : 1;
  uint8_t bit1       : 1;
  uint8_t bit2       : 1;
  uint8_t bit3       : 1;
  uint8_t bit4       : 1;
  uint8_t bit5       : 1;
  uint8_t bit6       : 1;
  uint8_t bit7       : 1;
} bitwise_t;

#define PROPERTY_DISABLE                (0U)
#define PROPERTY_ENABLE                 (1U)

#endif /* MEMS_SHARED_TYPES */

/**
  * @}
  *
  */

/** @addtogroup LIS3MDL_Interfaces_Functions
  * @brief       This section provide a set of functions used to read and
  *              write a generic register of the device.
  *              MANDATORY: return 0 -> no Error.
  * @{
  *
  */

typedef int32_t (*bh1749nuc_write_ptr)(void *, uint8_t, uint8_t*, uint16_t);
typedef int32_t (*bh1749nuc_read_ptr) (void *, uint8_t, uint8_t*, uint16_t);

typedef struct {
  /** Component mandatory fields **/
  bh1749nuc_write_ptr  write_reg;
  bh1749nuc_read_ptr   read_reg;
  /** Customizable optional pointer **/
  void *handle;
} bh1749nuc_ctx_t;

/**
  * @}
  *
  */

/** @defgroup LIS2DE12_Infos
  * @{
  *
  */

/** I2C Device Address 8 bit format if SA0=0 -> 31 if SA0=1 -> 33 **/
#define BH1749NUC_I2C_ADDR_L   0x70U
#define BH1749NUC_I2C_ADDR_H   0x72U

/** Device Identification (Who am I) **/
#define BH1749NUC_ID          0xE0U

/**
  * @}
  *
  */

#define BH1749NUC_SYSTEM_CONTROL_REG_ADDR       0x40U
typedef struct {
  uint8_t part_id       : 6;
  uint8_t int_reset     : 1;
  uint8_t sw_reset      : 1;
} bh1749nuc_system_control_reg_t;



#define BH1749NUC_MODE_CONTROL1_REG_ADDR    0x41U
typedef struct {
  uint8_t measurement_mode      : 3;
  uint8_t rgb_gain              : 2;
  uint8_t ir_gain               : 2;
  uint8_t not_used_01           : 1;
} bh1749nuc_mode_control1_reg_t;

#define BH1749NUC_MODE_CONTROL2_REG_ADDR     0x42U
typedef struct {
  uint8_t not_used_01       : 4;
  uint8_t rgb_en            : 1;
  uint8_t not_used_02       : 2;
  uint8_t valid             : 1;
} bh1749nuc_mode_control2_reg_t;

#define BH1749NUC_RED_DATA_L8BIT_REG_ADDR         0x50U
typedef struct {
  uint8_t red_data_l           : 8;
} bh1749nuc_red_data_bit8low_reg_t;

#define BH1749NUC_RED_DATA_H8BIT_REG_ADDR         0x51U
typedef struct {
  uint8_t red_data_h           : 8;
} bh1749nuc_red_data_bit8high_reg_t;

#define BH1749NUC_GREEN_DATA_L8BIT_REG_ADDR       0x52U
typedef struct {
  uint8_t green_data_l           : 8;
} bh1749nuc_green_data_bit8low_reg_t;

#define BH1749NUC_GREEN_DATA_H8BIT_REG_ADDR       0x53U
typedef struct {
  uint8_t green_data_h           : 8;
} bh1749nuc_green_data_bit8high_reg_t;

#define BH1749NUC_BLUE_DATA_L8BIT_REG_ADDR        0x54U
typedef struct {
  uint8_t blue_data_l           : 8;
} bh1749nuc_blue_data_bit8low_reg_t;

#define BH1749NUC_BLUE_DATA_H8BIT_REG_ADDR        0x55U
typedef struct {
  uint8_t blue_data_h           : 8;
} bh1749nuc_blue_data_bit8high_reg_t;

#define BH1749NUC_RESERVED_0X56_REG_ADDR        0x56U
#define BH1749NUC_RESERVED_0X57_REG_ADDR        0x57U

#define BH1749NUC_IR_DATA_L8BIT_REG_ADDR            0x58U
typedef struct {
  uint8_t ir_data_l          : 8;
} bh1749nuc_ir_data_bit8low_reg_t;
#define BH1749NUC_IR_DATA_H8BIT_REG_ADDR            0x59U
typedef struct {
  uint8_t ir_data_h          : 8;
} bh1749nuc_ir_data_bit8high_reg_t;


#define BH1749NUC_GREEN2_DATA_L8BIT_REG_ADDR         0x5AU
typedef struct {
  uint8_t green2_data_l         : 8;
} bh1749nuc_green2_data_bit8low_reg_t;
#define BH1749NUC_GREEN2_DATA_H8BIT_REG_ADDR          0x5BU
typedef struct {
  uint8_t green2_data_h          : 8;
} bh1749nuc_green2_data_bit8high_reg_t;


#define BH1749NUC_INTERRUPT_REG_ADDR        0x60U
typedef struct {
  uint8_t int_enable        : 1;
  uint8_t not_used_01       : 1;
  uint8_t int_source        : 2;
  uint8_t not_used_02      : 3;
  uint8_t int_status      : 1;
} bh1749nuc_interrupt_reg_t;

#define BH1749NUC_TH_HIGH_L8BIT_REG_ADDR        0x62U
typedef struct {
  uint8_t th_high_l : 8;
} bh1749nuc_th_high_bit8low_reg_t;
#define BH1749NUC_TH_HIGH_H8BIT_REG_ADDR        0x63U
typedef struct {
  uint8_t th_high_h : 8;
} bh1749nuc_th_high_bit8high_reg_t;

#define BH1749NUC_TH_LOW_L8BIT_REG_ADDR         0x64U
typedef struct {
  uint8_t th_low_l : 8;
} bh1749nuc_th_low_bit8low_reg_t;
#define BH1749NUC_TH_LOW_H8BIT_REG_ADDR         0x65U
typedef struct {
  uint8_t th_low_h : 8;
} bh1749nuc_th_low_bit8high_reg_t;


#define MANUFACTURER_ID_REG_ADD           0x92U
/**
  * @defgroup LIS2DE12_Register_Union
  * @brief    This union group all the registers that has a bitfield
  *           description.
  *           This union is usefull but not need by the driver.
  *
  *           REMOVING this union you are complient with:
  *           MISRA-C 2012 [Rule 19.2] -> " Union are not allowed "
  *
  * @{
  *
  */
typedef union{
    bh1749nuc_system_control_reg_t system_control_reg;
    bh1749nuc_mode_control1_reg_t mode_control1_reg;
    bh1749nuc_mode_control2_reg_t mode_control2_reg;
    bh1749nuc_red_data_bit8low_reg_t red_data_bit8low_reg;
    bh1749nuc_red_data_bit8high_reg_t red_data_bit8high_reg;
    bh1749nuc_green_data_bit8low_reg_t green_data_bit8low_reg;
    bh1749nuc_green_data_bit8high_reg_t green_data_bit8high_reg;
    bh1749nuc_blue_data_bit8low_reg_t blue_data_bit8low_reg;
    bh1749nuc_blue_data_bit8high_reg_t blue_data_bit8high_reg;
    bh1749nuc_ir_data_bit8low_reg_t ir_data_bit8low_reg;
    bh1749nuc_ir_data_bit8high_reg_t ir_data_bit8high_reg;
    bh1749nuc_green2_data_bit8low_reg_t green2_data_bit8low_reg;
    bh1749nuc_green2_data_bit8high_reg_t green2_data_bit8high_reg;
    bh1749nuc_interrupt_reg_t interrupt_reg;
    bh1749nuc_th_high_bit8low_reg_t th_high_bit8low_reg;
    bh1749nuc_th_high_bit8high_reg_t th_high_bit8high_reg;
    bh1749nuc_th_low_bit8low_reg_t th_low_bit8low;
    bh1749nuc_th_low_bit8high_reg_t th_low_bit8high_reg;

  bitwise_t                 bitwise;
  uint8_t                   byte;
} bh1749nuc_reg_t;


int32_t bh1749nuc_read_reg(bh1749nuc_ctx_t *ctx, uint8_t reg, uint8_t* data,
                          uint16_t len);
int32_t bh1749nuc_write_reg(bh1749nuc_ctx_t *ctx, uint8_t reg, uint8_t* data,
                           uint16_t len);


int32_t bh1749nuc_device_id_get(bh1749nuc_ctx_t *ctx, uint8_t *buff);




int32_t bh1749nuc_ir_gain_get(bh1749nuc_ctx_t *ctx, uint8_t *val);
int32_t bh1749nuc_ir_gain_set(bh1749nuc_ctx_t *ctx, uint8_t val);

int32_t bh1749nuc_rgb_gain_get(bh1749nuc_ctx_t *ctx, uint8_t *val);
int32_t bh1749nuc_rgb_gain_set(bh1749nuc_ctx_t *ctx, uint8_t val);

int32_t bh1749nuc_measurement_mode_get(bh1749nuc_ctx_t *ctx, uint8_t *val);
int32_t bh1749nuc_measurement_mode_set(bh1749nuc_ctx_t *ctx, uint8_t val);

int32_t bh1749nuc_mode_control1_get(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control1_reg_t *val);
int32_t bh1749nuc_mode_control1_set(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control1_reg_t *val);

int32_t bh1749nuc_mode_control2_get(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control2_reg_t *val);
int32_t bh1749nuc_mode_control2_set(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control2_reg_t *val);

int32_t bh1749nuc_red_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val);
int32_t bh1749nuc_green_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val);
int32_t bh1749nuc_blue_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val);
int32_t bh1749nuc_ir_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val);
int32_t bh1749nuc_green2_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val);

int32_t bh1749nuc_interrupt_get(bh1749nuc_ctx_t *ctx, bh1749nuc_interrupt_reg_t *val);
int32_t bh1749nuc_interrupt_set(bh1749nuc_ctx_t *ctx, bh1749nuc_interrupt_reg_t *val);

int32_t bh1749nuc_interrupt_get(bh1749nuc_ctx_t *ctx, bh1749nuc_interrupt_reg_t *val);
int32_t bh1749nuc_interrupt_set(bh1749nuc_ctx_t *ctx, bh1749nuc_interrupt_reg_t *val);
//int32_t bh1749nuc_boot_set(bh1749nuc_ctx_t *ctx, uint8_t val);
//int32_t bh1749nuc_boot_get(bh1749nuc_ctx_t *ctx, uint8_t *val);
//int32_t bh1749nuc_status_get(bh1749nuc_ctx_t *ctx, bh1749nuc_status_reg_t *val);
int32_t bh1749nuc_system_control_get(bh1749nuc_ctx_t *ctx, bh1749nuc_system_control_reg_t *val);

int32_t bh1749nuc_system_control_set(bh1749nuc_ctx_t *ctx, bh1749nuc_system_control_reg_t *val);

#ifdef __cplusplus
}
#endif

#endif /* LIS2DE12_REGS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
