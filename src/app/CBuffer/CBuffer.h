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

#ifndef C_BUFFER_H_
#define C_BUFFER_H_
#include "Arduino.h"
#include <stdint.h>
#include <string>
#include "rtos/Mutex.h"
/** A templated software ring buffer
 *
 * Example:
 * @code
 *  #include "mbed.h"
 *  #include "MyBuffer.h"
 *
 *  MyBuffer <char> buf;
 *
 *  int main()
 *  {
 *      buf = 'a';
 *      buf.put('b');
 *      char *head = buf.head();
 *      puts(head);
 *
 *      char whats_in_there[2] = {0};
 *      int pos = 0;
 *
 *      while(buf.available())
 *      {   
 *          whats_in_there[pos++] = buf;
 *      }
 *      printf("%c %c\n", whats_in_there[0], whats_in_there[1]);
 *      buf.clear();
 *      error("done\n\n\n");
 *  }
 * @endcode
 */
template <class T>
class CBuffer {
public:
    CBuffer(int length=12);
    
    ~CBuffer();
   
    bool isFull();
    

    bool isEmpty();
    
 
    void queue(T k);
    
  
    void flush();
    
    
    uint32_t available();
    
    
    bool dequeue(T * c);

private:
    rtos::Mutex _mtx;
    volatile uint32_t write;
    volatile uint32_t read;
    uint32_t size;
    T * buf;
    
};

#endif


