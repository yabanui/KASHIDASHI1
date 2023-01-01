#include <Arduino.h>
#include "M5Core2.h"
#include "SoftwareSerial.h"

#include "val.h"

SoftwareSerial GroveSerial(33, 32);

namespace serial
{

    int serial_init(void)
    {
        GroveSerial.begin(19200);
        GroveSerial.setTimeout(10);
        return NO_ERROR;
    }

    bool serial_check(void)
    {
        if (GroveSerial.available() > 0)
        {
            return true;
        }
        return false;
    }

    int serial_read(void)
    {
        g_serial_data = GroveSerial.readString();
        return 0;
    }
}