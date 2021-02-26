/*
 ******************************************************************************
 * @file    lis2de12_reg.c
 * @author  Sensors Software Solution Team
 * @brief   LIS2DE12 driver file
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

#include "bh1749nuc_reg.h"

/**
  * @defgroup  LIS2DE12
  * @brief     This file provides a set of functions needed to drive the
  *            lis2de12 enanced inertial module.
  * @{
  *
  */

/**
  * @defgroup  LIS2DE12_Interfaces_Functions
  * @brief     This section provide a set of functions used to read and
  *            write a generic register of the device.
  *            MANDATORY: return 0 -> no Error.
  * @{
  *
  */

/**
  * @brief  Read generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to read
  * @param  data  pointer to buffer that store the data read(ptr)
  * @param  len   number of consecutive register to read
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t bh1749nuc_read_reg(bh1749nuc_ctx_t* ctx, uint8_t reg, uint8_t* data,
                          uint16_t len)
{
  int32_t ret;
  ret = ctx->read_reg(ctx->handle, reg, data, len);
  return ret;
}

/**
  * @brief  Write generic device register
  *
  * @param  ctx   read / write interface definitions(ptr)
  * @param  reg   register to write
  * @param  data  pointer to data to write in register reg(ptr)
  * @param  len   number of consecutive register to write
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t bh1749nuc_write_reg(bh1749nuc_ctx_t* ctx, uint8_t reg, uint8_t* data,
                           uint16_t len)
{
  int32_t ret;
  ret = ctx->write_reg(ctx->handle, reg, data, len);
  return ret;
}


/**
  * @defgroup  LIS2DE12_Common
  * @brief     This section group common usefull functions
  * @{
  *
  */

/**
  * @brief  DeviceWhoamI .[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  buff     buffer that stores data read
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t bh1749nuc_device_id_get(bh1749nuc_ctx_t *ctx, uint8_t *buff)
{
  int32_t ret;
  ret = bh1749nuc_read_reg(ctx, MANUFACTURER_ID_REG_ADD, buff, 1);
  return ret;
}

/**
  * @brief  Reboot memory content. Reload the calibration parameters.[set]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      change the values of boot in reg CTRL_REG5
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t bh1749nuc_boot_set(bh1749nuc_ctx_t *ctx, uint8_t val)
{
  //bh1749nuc_ctrl_reg5_t ctrl_reg5;
  int32_t ret;

  //ret = bh1749nuc_read_reg(ctx, LIS2DE12_CTRL_REG5, (uint8_t*)&ctrl_reg5, 1);
 // if (ret == 0) {
  //  ctrl_reg5.boot = val;
 //   ret = bh1749nuc_write_reg(ctx, LIS2DE12_CTRL_REG5, (uint8_t*)&ctrl_reg5, 1);
 // }
  return ret;
}

/**
  * @brief  Reboot memory content. Reload the calibration parameters.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      change the values of boot in reg CTRL_REG5
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
  */
int32_t bh1749nuc_boot_get(bh1749nuc_ctx_t *ctx, uint8_t *val)
{
 // bh1749nuc_ctrl_reg5_t ctrl_reg5;
  int32_t ret;

 // ret = bh1749nuc_read_reg(ctx, LIS2DE12_CTRL_REG5, (uint8_t*)&ctrl_reg5, 1);
 // *val = (uint8_t)ctrl_reg5.boot;

  return ret;
}

/**
  * @brief  Info about device status.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      register STATUS_REG
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *

int32_t bh1749nuc_status_get(bh1749nuc_ctx_t *ctx, lis2de12_status_reg_t *val)
{
  int32_t ret;
  //ret = bh1749nuc_read_reg(ctx, LIS2DE12_STATUS_REG, (uint8_t*) val, 1);
  return ret;
}
*/
int32_t bh1749nuc_ir_gain_get(bh1749nuc_ctx_t *ctx, uint8_t *val)
{
    bh1749nuc_mode_control1_reg_t mode_control1_reg;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);
    *val=mode_control1_reg.ir_gain;
    return ret;
}

int32_t bh1749nuc_ir_gain_set(bh1749nuc_ctx_t *ctx, uint8_t val)
{
    bh1749nuc_mode_control1_reg_t mode_control1_reg;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);

    if (ret == 0) {
        mode_control1_reg.ir_gain = (uint8_t)val;
        ret = bh1749nuc_write_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);
    }

    return ret;
}

int32_t bh1749nuc_rgb_gain_get(bh1749nuc_ctx_t *ctx, uint8_t *val)
{
    bh1749nuc_mode_control1_reg_t mode_control1_reg;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);
    *val=mode_control1_reg.rgb_gain;
    return ret;
}

int32_t bh1749nuc_rgb_gain_set(bh1749nuc_ctx_t *ctx, uint8_t val)
{
    bh1749nuc_mode_control1_reg_t mode_control1_reg;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);

    if (ret == 0) {
        mode_control1_reg.rgb_gain = (uint8_t)val;
        ret = bh1749nuc_write_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);
    }

    return ret;
}

int32_t bh1749nuc_measurement_mode_get(bh1749nuc_ctx_t *ctx, uint8_t *val)
{
    bh1749nuc_mode_control1_reg_t mode_control1_reg;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);
    *val=mode_control1_reg.measurement_mode;
    return ret;
}

int32_t bh1749nuc_measurement_mode_set(bh1749nuc_ctx_t *ctx, uint8_t val)
{
    bh1749nuc_mode_control1_reg_t mode_control1_reg;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);

    if (ret == 0) {
        mode_control1_reg.measurement_mode = (uint8_t)val;
        ret = bh1749nuc_write_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)&mode_control1_reg, 1);
    }

    return ret;
}
/**
  * @brief  Info about device status.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      register STATUS_REG
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
*/
int32_t bh1749nuc_mode_control1_get(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control1_reg_t *val)
{
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*) val, 1);
    return ret;
}

int32_t bh1749nuc_mode_control1_set(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control1_reg_t *val)
{
    int32_t ret;
    ret = bh1749nuc_write_reg(ctx, BH1749NUC_MODE_CONTROL1_REG_ADDR, (uint8_t*)val, 1);
    return ret;
}
/**
  * @brief  Info about device status.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      register STATUS_REG
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
*/
int32_t bh1749nuc_mode_control2_get(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control2_reg_t *val)
{
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_MODE_CONTROL2_REG_ADDR, (uint8_t*) val, 1);
    return ret;
}

int32_t bh1749nuc_mode_control2_set(bh1749nuc_ctx_t *ctx, bh1749nuc_mode_control2_reg_t *val)
{
    int32_t ret;
        ret = bh1749nuc_write_reg(ctx, BH1749NUC_MODE_CONTROL2_REG_ADDR, (uint8_t*)val, 1);
    return ret;
}
/**
  * @brief  Info about device status.[get]
  *
  * @param  ctx      read / write interface definitions
  * @param  val      register STATUS_REG
  * @retval          interface status (MANDATORY: return 0 -> no Error)
  *
*/
int32_t bh1749nuc_red_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val)
{
    rgb1bit16_t data_raw;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_RED_DATA_L8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[0], 1);
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_RED_DATA_H8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[1], 1);
    *val=data_raw.i16bit;
    return ret;
}
int32_t bh1749nuc_green_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val)
{
    rgb1bit16_t data_raw;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_GREEN_DATA_L8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[0], 1);
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_GREEN_DATA_H8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[1], 1);
    *val=data_raw.i16bit;
    return ret;
}
int32_t bh1749nuc_blue_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val)
{
    rgb1bit16_t data_raw;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_BLUE_DATA_L8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[0], 1);
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_BLUE_DATA_H8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[1], 1);
    *val=data_raw.i16bit;
    return ret;
}

int32_t bh1749nuc_ir_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val)
{
    rgb1bit16_t data_raw;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_IR_DATA_L8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[0], 1);
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_IR_DATA_H8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[1], 1);
    *val=data_raw.i16bit;
    return ret;
}
int32_t bh1749nuc_green2_data_get(bh1749nuc_ctx_t *ctx, uint16_t *val)
{
    rgb1bit16_t data_raw;
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_GREEN2_DATA_L8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[0], 1);
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_GREEN2_DATA_H8BIT_REG_ADDR, (uint8_t*)&data_raw.u8bit[1], 1);
    *val=data_raw.i16bit;
    return ret;
}

int32_t bh1749nuc_system_control_get(bh1749nuc_ctx_t *ctx, bh1749nuc_system_control_reg_t *val)
{
    int32_t ret;
    ret = bh1749nuc_read_reg(ctx, BH1749NUC_SYSTEM_CONTROL_REG_ADDR, (uint8_t*) val, 1);
    return ret;
}

int32_t bh1749nuc_system_control_set(bh1749nuc_ctx_t *ctx, bh1749nuc_system_control_reg_t *val)
{
    int32_t ret;
    ret = bh1749nuc_write_reg(ctx, BH1749NUC_SYSTEM_CONTROL_REG_ADDR, (uint8_t*)val, 1);
    return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
