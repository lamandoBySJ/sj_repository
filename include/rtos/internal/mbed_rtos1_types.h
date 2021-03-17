/* mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_RTOS_RTX1_TYPES_H
#define MBED_RTOS_RTX1_TYPES_H

#if MBED_CONF_RTOS_PRESENT || defined(UNITTEST)

#include "cmsis_os.h"
typedef int32_t osStatus;
typedef int32_t osPriority;
#else
#include "rtos/cmsis_os2.h"
typedef int32_t osStatus;
typedef int32_t osPriority;
#endif

#endif