#include <Arduino.h>
#include "M5Core2.h"
#include "OpenFontRender.h"
#define LGFX_M5STACK_CORE2
#include <LovyanGFX.h>

#include "val.h"

//----------メモゾーン----------
// LCD解像度:320x240
// LCDリクエスト
// 0:要求なし、1:OPメッセージ表示要求、2:ホーム画面表示要求

//----------インスタンスの作成----------

static LGFX Llcd;
static LGFX_Sprite sp(&Llcd);

OpenFontRender render;
TFT_eSprite sprite(&M5.Lcd);

RTC_TimeTypeDef RTCtime;
RTC_DateTypeDef RTCDate;
TouchPoint_t coordinate;

namespace lcd
{

    //----------変数宣言----------
    String weekDay[7] = {"月", "火", "水", "木", "金", "土", "日"};
    String error_message[5] = {"No error", "Un known error", "No SDcard", "No Font file", "No settings.txt"};
    int power_x = 60;
    int power_prosess = 0;

    bool lcd_init()
    {
        M5.Lcd.invertDisplay(1);
        M5.Axp.SetLcdVoltage(3000);
        sprite.createSprite(M5.Lcd.width(), M5.Lcd.height());
        // render.showFreeTypeVersion(); // print FreeType version
        // render.showCredit();          // print FTL credit
        if (render.loadFont("/SystemFiles/03SmartFontUI.ttf"))
        {
            return false;
        }
        render.setDrawer(sprite); // Set drawer object
        return true;
    }

    void lcd_print_set(size_t font_size, uint16_t color, int32_t x, int32_t y)
    {
        render.setFontSize(font_size);
        render.setFontColor(color);
        render.setCursor(x, y);
    }

    void lcd_print_seek(size_t font_size, uint16_t color, int32_t x, int32_t y)
    {
        render.setFontSize(font_size);
        render.setFontColor(color);
        render.seekCursor(x, y);
    }

    void lcd_button_menu()
    {
        render.setFontSize(15);
        render.setFontColor(WHITE);
        render.setCursor(35, 220);
        render.printf("MENU");
        render.setCursor(140, 220);
        render.printf("HOME");
        render.setCursor(240, 220);
        render.printf("POWER");
    }

    void lcd_error(int error)
    {
        sprite.fillScreen(BLACK);
        sprite.setTextSize(3);
        sprite.setCursor(0, 0);
        sprite.printf("ERROR No.%d\n", error);
        sprite.print(error_message[error]);

        sprite.setTextSize(2);
        sprite.setCursor(220, 220);
        sprite.print("ReBoot");

        sprite.setCursor(0, 220);
        sprite.print("PowerOFF");
        sprite.pushSprite(10, 0);
    }

    void lcd_op()
    {
        sprite.fillScreen(BLACK);

        lcd_print_set(50, WHITE, 0, 0);
        render.printf("KASHIDASHI\n");

        lcd_print_seek(30, WHITE, 0, 10);
        render.printf("物品貸出管理システム");
        render.seekCursor(0, 10);

        lcd_print_set(10, WHITE, 0, 185);
        render.printf("Copyright(c) Information-technology Promotion Agency\n");
        render.seekCursor(0, 1);
        render.printf("Japan (IPA), 2003-2015.\n");

        render.setCursor(0, 215);
        render.setFontSize(10);
        render.setFontColor(WHITE);
        render.printf("Portions of this software are copyright © < 2.4.12 >\n");
        render.seekCursor(0, 1);
        render.printf("The FreeTypeProject (www.freetype.org).  All rights reserved.\n");

        sprite.pushSprite(0, 0);
    }

    void lcd_home(int home_mode)
    {
        M5.Rtc.GetTime(&RTCtime);
        M5.Rtc.GetDate(&RTCDate);
        float batVoltage = M5.Axp.GetBatVoltage();
        float batPercentage = (batVoltage < 3.2) ? 0 : (batVoltage - 3.2) * 100;

        sprite.fillScreen(0);
        sprite.drawLine(10, 45, 310, 45, WHITE);   // 横線、上
        sprite.drawLine(10, 220, 310, 220, WHITE); // 横線、下

        render.setFontSize(30);
        render.setFontColor(WHITE);
        render.setCursor(10, 0);
        render.printf("%d/%-2d(%s) %d:%02d",
                      RTCDate.Month, RTCDate.Date, weekDay[RTCDate.WeekDay],
                      RTCtime.Hours, RTCtime.Minutes);
        render.setCursor(240, 0);
        render.printf("%3.0f%%", batPercentage);

        lcd_button_menu();

        if (home_mode == 1)
        {
            render.setFontSize(40);
            render.setCursor(0, 100);
            render.printf("読み取り中...\n");
        }
        else if (home_mode == 2)
        {
            render.setFontSize(30);
            render.setCursor(0, 80);
            render.printf("読み取り成功\n");
            if (g_serial_data.length() > 1)
            {
                for (int i = 0; i < g_serial_data.length(); i++)
                {
                    render.printf("%c", g_serial_data.charAt(i));
                }
            }
        }
        else
        {
            sprite.drawLine(210, 130, 310, 130, WHITE); // 横線、中
            sprite.drawLine(200, 55, 200, 210, WHITE);  // 縦線、中

            lcd_print_set(20, WHITE, 10, 50);
            render.printf("最近の貸出一覧\n");
            // render.printf("バーコードQRコード\n");
            for (int i = g_data.row - 1; i >= 0; i--)
            {
                if (i >= g_data.row - 8)
                {
                    lcd_print_seek(15, WHITE, 10, 0);
                    if (g_data.kashi[i] == 1)
                    {
                        render.setFontColor(ORANGE);
                        render.printf("貸出 ");
                    }
                    else
                    {
                        render.setFontColor(CYAN);
                        render.printf("返却 ");
                    }

                    render.setFontColor(WHITE);
                    render.printf("%2d/%2d %2d:%02d %s\n", g_data.month[i], g_data.day[i], g_data.hour[i], g_data.minute[i], g_data.supply[i]);
                }
            }

            int num = 0;
            for (int i = g_o_data.o_row - 1; i >= 0; i--)
            {
                if (g_o_data.o_kashi[i] == 1)
                {
                    num++;
                }
            }

            render.setFontSize(25);
            render.setCursor(210, 45);
            render.printf("未返却数\n");

            render.seekCursor(218, 0);
            render.setFontSize(35);
            render.printf("%3d", num);
            render.setFontSize(20);
            render.seekCursor(0, 18);
            render.printf(" 件\n");

            render.setFontSize(13);
            render.setCursor(220, 130);
            render.printf("物品名 責任者\n");

            num = 0;
            for (int i = g_o_data.o_row - 1; i >= 0; i--)
            {
                if (g_o_data.o_kashi[i] == 1)
                {
                    num++;
                    if (num > 4)
                    {
                        render.seekCursor(256, -5);
                        render.printf("---");
                        break;
                    }
                    render.seekCursor(215, 0);
                    render.printf("%s %d\n", g_o_data.o_supply[i], g_o_data.o_name[i]);
                }
            }
        }
        sprite.pushSprite(0, 0); // 画面更新
    }

    void lcd_read()
    {
        sprite.fillScreen(0);
        sprite.drawLine(10, 45, 310, 45, WHITE);   // 横線、上
        sprite.drawLine(10, 220, 310, 220, WHITE); // 横線、下

        render.setFontSize(15);
        render.setFontColor(WHITE);
        render.setCursor(10, 0);
        render.printf("物品名:");
        char buff[6];
        g_serial_data.toCharArray(buff, 6);
        render.printf(buff);
        render.setFontSize(20);
        render.setCursor(10, 15);
        render.printf("責任者番号:");
        render.setFontSize(30);
        render.setCursor(120, 5);
        char buffs[6];
        g_key_data.toCharArray(buffs, 6);
        render.printf(buffs);

        sprite.drawLine(80, 55, 80, 210, WHITE);   // 縦線、中
        sprite.drawLine(160, 55, 160, 210, WHITE); // 縦線、中
        sprite.drawLine(240, 55, 240, 210, WHITE); // 縦線、中

        sprite.drawLine(10, 103, 310, 103, WHITE); // 横線、下
        sprite.drawLine(10, 162, 310, 162, WHITE); // 横線、下

        render.setFontSize(40);

        render.setFontColor(RED);
        render.setCursor(15, 42);
        render.printf("AC");
        render.setCursor(28, 100);
        render.printf("C");

        render.setFontColor(WHITE);
        render.setCursor(28, 158);
        render.printf("0");

        render.setCursor(108, 42);
        render.printf("7");
        render.setCursor(108, 100);
        render.printf("4");
        render.setCursor(108, 158);
        render.printf("1");

        render.setCursor(188, 42);
        render.printf("8");
        render.setCursor(188, 100);
        render.printf("5");
        render.setCursor(188, 158);
        render.printf("2");

        render.setCursor(268, 42);
        render.printf("9");
        render.setCursor(268, 100);
        render.printf("6");
        render.setCursor(268, 158);
        render.printf("3");

        render.setFontSize(15);
        render.setFontColor(WHITE);
        render.setCursor(15, 220);
        render.printf("キャンセル");
        render.setCursor(145, 220);
        render.printf("決定");
        sprite.pushSprite(0, 0); // 画面更新
    }

    void lcd_menu(int menu_mode)
    {
        sprite.fillScreen(0);
        sprite.drawLine(10, 45, 310, 45, WHITE);   // 横線、上
        sprite.drawLine(10, 132, 310, 132, WHITE); // 横線、中
        sprite.drawLine(10, 220, 310, 220, WHITE); // 横線、下
        sprite.drawLine(110, 55, 110, 210, WHITE); // 縦線、中
        sprite.drawLine(210, 55, 210, 210, WHITE); // 縦線、中

        lcd_print_set(30, WHITE, 10, 0);
        render.printf("MENU");

        lcd_print_set(20, WHITE, 15, 100);
        render.printf("貸出一覧");
        lcd_print_set(20, WHITE, 120, 100);
        render.printf("貸出変更");
        lcd_print_set(20, WHITE, 225, 100);
        render.printf("名称変更");

        lcd_print_set(20, WHITE, 15, 185);
        render.printf("時間設定");
        M5.Rtc.GetTime(&RTCtime);
        M5.Rtc.GetDate(&RTCDate);
        lcd_print_set(20, WHITE, 120, 185);
        render.printf("画面設定");
        lcd_print_set(20, WHITE, 225, 185);
        render.printf("詳細表示");

        render.setFontSize(15);
        render.setFontColor(WHITE);
        render.setCursor(35, 220);
        render.printf("MENU");
        render.setCursor(140, 220);
        render.printf("HOME");
        render.setCursor(240, 220);
        render.printf("POWER");
        sprite.pushSprite(0, 0); // 画面更新
    }

    void lcd_time_setting()
    {
        sprite.fillScreen(0);
        sprite.drawLine(10, 45, 310, 45, WHITE);   // 横線、上
        sprite.drawLine(10, 220, 310, 220, WHITE); // 横線、下

        lcd_print_set(30, WHITE, 10, 0);
        render.printf("Time Setting");
        render.setFontSize(30);
        render.setFontColor(WHITE);
        render.setCursor(10, 100);
        render.printf("%d/%02d/%02d %02d:%02d",
                      RTCDate.Year, RTCDate.Month, RTCDate.Date,
                      RTCtime.Hours, RTCtime.Minutes);

        render.setFontSize(15);
        render.setFontColor(WHITE);
        render.setCursor(15, 220);
        render.printf("キャンセル");
        render.setCursor(145, 220);
        render.printf("決定");
        sprite.pushSprite(0, 0); // 画面更新
    }

    void lcd_set_power_off()
    {
        sprite.fillScreen(0);

        sprite.drawLine(10, 220, 310, 220, WHITE); // 横線、下
        sprite.drawRoundRect(25, 85, 270, 70, 35, WHITE);

        lcd_print_set(30, WHITE, 75, 20);
        render.printf("右にスライド");

        render.setFontSize(15);
        render.setFontColor(WHITE);
        render.setCursor(35, 220);
        render.printf("MENU");
        render.setCursor(140, 220);
        render.printf("HOME");
        render.setCursor(240, 220);
        render.printf("POWER");

        sprite.pushSprite(0, 0); // 画面更新
    }

    void lcd_power_off()
    {
        coordinate = M5.Touch.getPressPoint();
        sprite.fillRect(20, 90, 280, 70, BLACK);
        sprite.drawRoundRect(25, 85, 270, 70, 35, WHITE);
        if (power_prosess == 0)
        {
            if (30 < coordinate.x && coordinate.x < 90 && coordinate.y < 160 && coordinate.y > 90)
                power_prosess = 1;
        }
        else if (power_prosess == 1)
        {
            if (60 <= coordinate.x && coordinate.y < 160 && coordinate.y > 90)
            {
                power_x = coordinate.x;
                if (coordinate.x >= 260)
                {
                    M5.Axp.PowerOff();
                }
            }
            else
            {
                power_x = 60;
                power_prosess = 0;
            }
        }
        sprite.fillCircle(power_x, 120, 30, WHITE);
        sprite.pushSprite(0, 0); // 画面更新
    }

    void lcd(int lcd_request, int error)
    {
        switch (lcd_request)
        {
        case LCD_ERROR:
            lcd_error(error);
            break;
        case LCD_OP:
            lcd_op();
            break;

        case LCD_HOME:
            lcd_home(0);
            break;

        case LCD_READING:
            lcd_read();
            break;

        case LCD_READ:
            lcd_home(2);
            break;

        case LCD_MENU:
            lcd_menu(0);
            break;
        case LCD_TIME_SETTING:
            lcd_time_setting();
            break;

        case LCD_SET_POWER_OFF:
            lcd_set_power_off();
            break;

        case LCD_POWER_OFF:
            lcd_power_off();
            break;

        default:
            break;
        }
    }
}