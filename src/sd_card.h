#ifndef SD_CARD_H
#define SD_CARD_H

#include <Arduino.h>

namespace sd_card
{
    int sd_init();
    void sd_read_data();
    void sd_ob_data_update(char* supply,int name);
    bool sd_check_state(String supply);
}

#endif