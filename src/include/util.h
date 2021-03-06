#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdint.h> 

void *memset(void *mem, int c, size_t n);
void *memcpy(void *dest, void *src, size_t n);
size_t strlen(const char *str);
size_t strcmp(char *str1, char *str2);
void strcpy(char *str1, char *str2);
int atoi(char *p);
uint32_t hatoi(char *p);
void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

#endif
