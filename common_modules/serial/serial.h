#ifndef _SERIAL_H
#define _SERIAL_H

#include "ets_sys.h"

void serial_init(void);

void serial_write(const char* buf);

// returns the number of bytes successfully read
int serial_read(char* buf, int len);


#endif