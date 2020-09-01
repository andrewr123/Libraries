/* cppfix.h
 * 
 * This file fixes a few definition problems in AVR programs written in C++.
 *
 * It was originally posted here:
 * http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=410870
 * It was reported on the Arduino forum:
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1216134904
 */

#ifndef cppfix
#define cppfix

#include "WProgram.h"

__extension__ typedef int __guard __attribute__((mode (__DI__)));

void * operator new(size_t size);
void operator delete(void * ptr); 

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
void __cxa_guard_abort (__guard *) {}; 

void * operator new(size_t size)
{
  return malloc(size);
}

void operator delete(void * ptr)
{
  free(ptr);
} 


#endif 