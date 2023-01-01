#ifndef LCD_H
#define LCD_H

#include <Arduino.h>

namespace lcd
{
    bool lcd_init();
    void lcd_print_set(size_t font_size, uint16_t color, int32_t x, int32_t y);
    void lcd_print_seek(size_t font_size, uint16_t color, int32_t x, int32_t y);
    void lcd_button_menu();

    void lcd_error(int error);
    void lcd_op();
    void lcd_home(int home_mode);
    void lcd_menu(int menu_mode);
    void lcd_time_setting();
    void lcd_set_power_off();
    void lcd_power_off();
    void lcd(int lcd_request,int error);
}

#endif