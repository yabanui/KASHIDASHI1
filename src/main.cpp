#include <Arduino.h>
#include <M5Core2.h>

#include <Ticker.h>



#include "val.h"
#include "lcd.h"
#include "sd_card.h"
#include "serial.h"

//----------メモゾーン----------
// LCD解像度:320x240

//----------クラスの作成----------
Ticker timer1;

//----------変数宣言----------
String g_serial_data;
String g_key_data;

int vibrate_count = 0;
int lcd_count = 0;
bool vibrate = false;
int lcd_request = LCD_NO;
int state = STATE_OP;
int error = ERROR_UNKOWN;
bool hasdata;
bool t_rise;
bool press_old;
int n_num;
TouchPoint_t touch_offset;
String n_name;

String serial_data;

struct kashidashi_data g_data;
struct object_data g_o_data;

//----------タイマーハンドラ----------
void timer1_handler(void)
{
  if (vibrate_count < 10)
    vibrate_count++;

  if (lcd_count < 400)
    lcd_count++;
}

void setup()
{
  // put your setup code here, to run once:
  M5.begin();
  Serial.println("-----KASHIDASHI Sterted-----");
  timer1.attach_ms(10, timer1_handler);

  if (!lcd::lcd_init())
  {
    state = STATE_ERROR;
    error = ERROR_NO_FONT;
  }

  int sd = sd_card::sd_init();
  if (sd != 0)
  {
    state = STATE_ERROR;
    error = sd;
  }

  serial::serial_init();

  M5.Axp.SetLDOVoltage(3, 3300);
  M5.begin(true, true, true, true);
  // lcd.init();
  // lcd.setFont(&fonts::lgfxJapanGothic_36);
  // lcd.println("こんにちは　世界！");
}

void loop()
{
  M5.update();
  if (vibrate_count >= 6)
  {
    vibrate = false;
  }
  M5.Axp.SetLDOEnable(3, vibrate);

  lcd::Lcd(lcd_request, error);

  switch (state)
  {
  case STATE_ERROR:
    lcd_request = LCD_ERROR;
    if (M5.BtnA.wasPressed())
    {
      M5.Axp.PowerOff();
    }

    if (M5.BtnB.wasPressed())
    {
    }

    if (M5.BtnC.wasPressed())
    {
      M5.Axp.DeepSleep(1);
    }
    break;
  case STATE_OP: // ステータス：オープニング表示要求
    lcd_request = LCD_OP;
    state = STATE_CHECK;
    sd_card::sd_read_data();
    lcd_count = 0;
    break;

  case STATE_CHECK: // ステータス：エラーチェック
    lcd_request = LCD_NO;
    if (lcd_count >= 200)
    {
      state = STATE_DISPLAY_HOME;
    }
    break;

  case STATE_DISPLAY_HOME: // ステータス：ホーム表示要求
    sd_card::sd_read_data();
    lcd_request = LCD_HOME;
    state = STATE_HOME;
    break;

  case STATE_HOME: // ステータス：ホーム
    lcd_request = LCD_NO;
    if (serial::serial_check())
    {
      serial::serial_read();
      sd_card::sd_read_data();
      if (sd_card::sd_check_state(g_serial_data.substring(0, 5)))
      {
        state = STATE_NUMPAD;
      }
      else
      {
        state = STATE_DISPLAY_HOME;
        char buff[6];
        g_serial_data.toCharArray(buff, 6);
        sd_card::sd_ob_data_update(buff, g_key_data.toInt());
      }
    }

    if (M5.BtnA.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_MENU;
    }

    if (M5.BtnB.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
    }

    if (M5.BtnC.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_POWER_OFF;
    }
    break;

  case STATE_DISPLAY_MENU: // ステータス：メニュー表示要求
    lcd_request = LCD_MENU;
    state = STATE_MENU;
    /* code */
    break;

  case STATE_MENU: // ステータス：メーニュー
    lcd_request = LCD_NO;
    t_rise = (press_old ^ M5.Touch.ispressed()) & M5.Touch.ispressed();
    press_old = M5.Touch.ispressed();
    if (t_rise)
    {
      delay(10);
      TouchPoint_t coordinate;
      coordinate = M5.Touch.getPressPoint();
      Serial.printf("x:%d, y:%d \r\n", coordinate.x, coordinate.y);
      int fig[2][4] = {{1, 2, 3, 0},
                       {STATE_TIME_SETTING, 5, 6, 0}};
      int fig_cul;
      int fig_row;
      if (45 < coordinate.y && coordinate.y < 130)
      {
        fig_row = 0;
      }
      else if (135 < coordinate.y && coordinate.y < 220)
      {
        fig_row = 1;
      }

      if (0 < coordinate.x && coordinate.x < 100)
      {
        fig_cul = 0;
      }
      else if (110 < coordinate.x && coordinate.x < 200)
      {
        fig_cul = 1;
      }
      else if (210 < coordinate.x && coordinate.x < 300)
      {
        fig_cul = 2;
      }
      else
      {
        fig_cul = 3;
      }
      Serial.printf("p:%d\n", fig[fig_row][fig_cul]);
      if (fig[fig_row][fig_cul] != 0)
      {
        state = fig[fig_row][fig_cul];
      }
    }

    if (M5.BtnA.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
    }

    if (M5.BtnB.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_HOME;
    }

    if (M5.BtnC.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_POWER_OFF;
    }
    break;
  case STATE_TIME_SETTING:
    lcd_request = LCD_TIME_SETTING;

    t_rise = (press_old ^ M5.Touch.ispressed()) & M5.Touch.ispressed();
    press_old = M5.Touch.ispressed();
    if (t_rise)
    {
      delay(10);
      TouchPoint_t coordinate;
      coordinate = M5.Touch.getPressPoint();
      touch_offset = M5.Touch.getPressPoint();
      Serial.printf("x:%d, y:%d \r\n", coordinate.x, coordinate.y);
      int fig;
      if (10 < coordinate.x && coordinate.x < 80)
      {
        fig = 1;
      }
      else if (95 < coordinate.x && coordinate.x < 135)
      {
        fig = 2;
      }
      else if (140 < coordinate.x && coordinate.x < 180)
      {
        fig = 3;
      }
      else if (200 < coordinate.x && coordinate.x < 240)
      {
        fig = 4;
      }
      else if (245 < coordinate.x && coordinate.x < 295)
      {
        fig = 5;
      }
      else
      {
        fig = 0;
      }
      Serial.printf("p:%d\n", fig);
      if (fig != 0)
      {
        TouchPoint_t coordinate;
        coordinate = M5.Touch.getPressPoint();
      }
    }
    if (M5.BtnA.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_HOME;
    }

    if (M5.BtnB.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_MENU;
    }

    break;

  case STATE_DISPLAY_POWER_OFF:
    lcd_request = LCD_SET_POWER_OFF;
    state = STATE_POWER_OFF;
    break;

  case STATE_POWER_OFF:
    lcd_request = LCD_POWER_OFF;
    if (M5.BtnA.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_MENU;
    }

    if (M5.BtnB.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_HOME;
    }

    if (M5.BtnC.wasPressed())
    {
      vibrate = true;
      vibrate_count = 0;
    }
    break;

  case STATE_NUMPAD:
    lcd_request = LCD_READING;

    t_rise = (press_old ^ M5.Touch.ispressed()) & M5.Touch.ispressed();
    press_old = M5.Touch.ispressed();
    if (t_rise)
    {
      TouchPoint_t coordinate;
      coordinate = M5.Touch.getPressPoint();
      Serial.printf("x:%d, y:%d \r\n", coordinate.x, coordinate.y);
      int fig[4][4] = {{10, 7, 8, 9},
                       {11, 4, 5, 6},
                       {0, 1, 2, 3},
                       {12, 12, 12, 12}};
      int fig_cul;
      int fig_row;
      if (40 < coordinate.y && coordinate.y < 100)
      {
        fig_cul = 0;
      }
      else if (100 < coordinate.y && coordinate.y < 160)
      {
        fig_cul = 1;
      }
      else if (160 < coordinate.y && coordinate.y < 220)
      {
        fig_cul = 2;
      }
      else
      {
        fig_cul = 3;
      }

      if (0 < coordinate.x && coordinate.x < 80)
      {
        fig_row = 0;
      }
      else if (80 < coordinate.x && coordinate.x < 160)
      {
        fig_row = 1;
      }
      else if (160 < coordinate.x && coordinate.x < 240)
      {
        fig_row = 2;
      }
      else if (240 < coordinate.x && coordinate.x < 320)
      {
        fig_row = 3;
      }
      else
      {
        fig_cul = 3;
        fig_row = 0;
      }
      Serial.printf("p:%d\n", fig[fig_cul][fig_row]);
      if (fig[fig_cul][fig_row] < 10 && g_key_data.length() < 5)
      {
        g_key_data += String(fig[fig_cul][fig_row]);
      }
      else if (fig[fig_cul][fig_row] == 10)
      {
        g_key_data = "\0";
      }
      else if (fig[fig_cul][fig_row] == 11)
      {
        g_key_data = g_key_data.substring(0, g_key_data.length() - 1);
      }
      else
      {
        vibrate = true;
        vibrate_count = 0;
      }
    }

    if (M5.BtnA.wasPressed())
    {
      g_key_data = "\0";
      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_HOME;
    }

    if (M5.BtnB.wasPressed())
    {
      char buff[6];
      g_serial_data.toCharArray(buff, 6);
      sd_card::sd_ob_data_update(buff, g_key_data.toInt());
      g_key_data = "\0";

      vibrate = true;
      vibrate_count = 0;
      state = STATE_DISPLAY_HOME;
    }

    // if (M5.BtnC.wasPressed())
    // {
    // }
    break;

    // default:

    //   break;
  }
}