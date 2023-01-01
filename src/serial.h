#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

namespace serial
{
    int serial_init();
    bool serial_check(void);
    int serial_read(void);
}

#endif