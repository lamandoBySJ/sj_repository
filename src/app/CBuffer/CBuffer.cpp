/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "CBuffer.h"
#include <mutex>

template <class T>
CBuffer<T>::CBuffer(int length):_mtx()
{
        write = 0;
        read = 0;
        size = length + 1;
        buf = new T [size];
    };

template <class T>
CBuffer<T>::~CBuffer()
{
    delete [] buf;
    
    return;
}

template <class T>
bool CBuffer<T>::isFull() {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    return (((write + 1) % size) == read);
};

template <class T>
bool CBuffer<T>::isEmpty() {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    return (read == write);
};

template <class T>
void CBuffer<T>::queue(T k) {
    
        if (isFull()) {
            _mtx.lock();
            read++;
            read %= size;
            _mtx.unlock();
        }
        std::lock_guard<rtos::Mutex> lck(_mtx);
        buf[write++] = k;
        write %= size;
}

template <class T> 
void CBuffer<T>::flush() {
    std::lock_guard<rtos::Mutex> lck(_mtx);
    read = 0;
    write = 0;
}
    
template <class T>
uint32_t CBuffer<T>::available() {
    std::lock_guard<rtos::Mutex> lck(_mtx);
        return (write >= read) ? write - read : size - read + write;
};

template <class T>
bool CBuffer<T>::dequeue(T * c) {
    std::lock_guard<rtos::Mutex> lck(_mtx);
        bool empty = isEmpty();
        if (!empty) {
            *c = buf[read++];
            read %= size;
        }
        return(!empty);
};

// make the linker aware of some possible types
template class CBuffer<uint8_t>;
template class CBuffer<int8_t>;
template class CBuffer<uint16_t>;
template class CBuffer<int16_t>;
template class CBuffer<uint32_t>;
template class CBuffer<int32_t>;
template class CBuffer<uint64_t>;
template class CBuffer<int64_t>;
template class CBuffer<char>;
template class CBuffer<wchar_t>;
template class CBuffer<char *>;
template class CBuffer<double>;
template class CBuffer<float>;

