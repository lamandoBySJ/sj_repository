/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
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
#include "platform/CriticalSectionLock.h"
#include "platform/mbed_critical.h"
#include "platform/mbed_debug.h"
#include "rtos/Mutex.h"
namespace mbed {

static rtos::Mutex mtx;

CriticalSectionLock::CriticalSectionLock()
{
    //core_util_critical_section_enter();
    mtx.lock();
     debug("-- 1-- CriticalSectionLock\n");
}

CriticalSectionLock::~CriticalSectionLock()
{
   // core_util_critical_section_exit();
    debug("-- 2-- ~CriticalSectionLock\n");
     mtx.unlock();
}

void CriticalSectionLock::enable()
{
    mtx.lock();
    debug("-- 3 -- enable()\n");
   // core_util_critical_section_enter();
}

void CriticalSectionLock::disable()
{
   
    debug("-- 4 -- disable()\n");
    //core_util_critical_section_exit();
    mtx.unlock();
}

} // namespace mbed
