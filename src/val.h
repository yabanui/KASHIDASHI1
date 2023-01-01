#ifndef VAL_H
#define VAL_H

#include <Arduino.h>

//----------enum定義----------
enum LCD_REQUEST
{
    LCD_NO,
    LCD_ERROR,
    LCD_OP,
    LCD_HOME,
    LCD_READING,
    LCD_READ,
    LCD_MENU,
    LCD_TIME_SETTING,
    LCD_SET_POWER_OFF,
    LCD_POWER_OFF
};

enum STATE
{
    STATE_ERROR,
    STATE_OP,
    STATE_CHECK,
    STATE_DISPLAY_HOME,
    STATE_HOME,
    STATE_DISPLAY_MENU,
    STATE_MENU,
    STATE_TIME_SETTING,
    STATE_DISPLAY_POWER_OFF,
    STATE_POWER_OFF,
    STATE_NUMPAD
};

enum ERROR
{
    NO_ERROR,
    ERROR_UNKOWN,
    ERROR_NO_SD,
    ERROR_NO_FONT,
    ERROR_NO_FILE_SETTINGS
};

//----------グローバル変数----------
extern String g_serial_data;
extern String g_key_data;

//----------構造体定義----------
struct kashidashi_data
{
    int row;
    int16_t month[100];
    int16_t day[100];
    int16_t hour[100];
    int16_t minute[100];
    int16_t second[100];
    int16_t kashi[100];
    char supply[100][10];
    int32_t name[100];
};
extern struct kashidashi_data g_data;

struct object_data
{
    int o_row;
    char o_supply[100][10];
    int32_t o_name[100];
    bool o_kashi[100];
};
extern struct object_data g_o_data;

#endif