#include <Arduino.h>
#include "M5Core2.h"
#include "val.h"
namespace sd_card
{
    int sd_init()
    {
        if (!SD.begin())
            return ERROR_NO_SD;
        if (!SD.exists("/SystemFiles/settings.txt"))
        {
            if (!SD.exists("/SystemFiles"))
            {
                SD.mkdir("/SystemFiles");
            }
            File fp;
            fp = SD.open("/SystemFiles/settings.txt", FILE_WRITE);
            fp.println("setting_file");
            fp.close();
            return ERROR_NO_FILE_SETTINGS;
        }
        // objects_state.txt
        return NO_ERROR;
    }

    void sd_read_data()
    {
        if (SD.exists("/DataFiles/objects_state.csv") == false)
        {
            if (SD.exists("/DataFiles") == false)
            {
                SD.mkdir("/DataFiles");
            }
            File fp;
            fp = SD.open("/DataFiles/objects_state.csv", FILE_WRITE);
            fp.println("object,name,state");
            fp.close();
        }

        File fpob;
        fpob = SD.open("/DataFiles/objects_state.csv", FILE_READ);
        int f_size = fpob.size();
        fpob.close();
        delay(100);

        if (f_size == 0)
        {
            File fp;
            fp = SD.open("/DataFiles/objects_state.csv", FILE_WRITE);
            fp.println("object,name,state");
            fp.close();
        }
        else
        {
            File fp_ob;
            fp_ob = SD.open("/DataFiles/objects_state.csv", FILE_READ);

            String line;              // 行を一時的に記憶する変数
            int row_inheader = 0;     // ヘッダーも含めた行数を記憶する変数
            while (fp_ob.available()) // データが有ればループ
            {
                char ch = fp_ob.read(); // データを1byteずつ読み取り
                line += String(ch);     // 1byteずつ繋げていく
                if (ch == '\n')         //'\n'が読み込まれた所で一旦読み取りをやめる
                {
                    line.trim(); // 行に含まれる不要な空白を取り除く

                    int column = 0;    // 行に含まれる要素番号カウントする変数
                    int num_end = 0;   // 要素の終わりの列番号を記憶する変数
                    int num_start = 0; // 要素の始まりの列番号を記憶する変数

                    if (0 < row_inheader) // ヘッダーを読み込まないように除外する
                    {
                        while (num_end != -1) // 範囲内に','がなくなるまで繰り返し
                        {
                            num_end = line.indexOf(',', num_start);           // num_strから','がないか確認。あれば列番号をnum_endに記憶
                            String part = line.substring(num_start, num_end); //','で区切られた要素を行から切り出す
                            if (column == 0)                                  // 行の最初の要素
                            {
                                part.toCharArray(g_o_data.o_supply[row_inheader - 1], num_end + 1 - num_start); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                            }
                            else if (column == 1) // 行の2番目の要素
                            {
                                g_o_data.o_name[row_inheader - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをint型に変換して代入
                            }
                            else if (column == 2) // 行の3番目の要素
                            {
                                g_o_data.o_kashi[row_inheader - 1] = boolean(part.toInt()); // 配列の"ヘッダーを除いた行数"番目にデータをbool型に変換して代入
                            }
                            num_start = num_end + 1; //','分で1を足して、次の要素を区切る','を探し始めるスタート地点を代入
                            column++;                // 要素数に1を加算
                        }
                    }
                    line = "\0";    // 変数を初期化
                    row_inheader++; // 行数に1を加算
                }
                g_o_data.o_row = row_inheader - 1; // ヘッダーを除いた行数をrowに代入
            }
        }

        // month,day,hour,minute,second,state,supply,name
        RTC_DateTypeDef RTCDate;
        M5.Rtc.GetDate(&RTCDate);
        String data_dir = "/DataFiles/" + String(RTCDate.Year);
        String data_file = "/DataFiles/" + String(RTCDate.Year) + "/" + String(RTCDate.Year) + "_" + String(RTCDate.Month) + ".csv";

        if (SD.exists(data_file) == false)
        {
            if (SD.exists(data_dir) == false)
            {
                SD.mkdir(data_dir);
            }
            File fp;
            fp = SD.open(data_file, FILE_WRITE);
            fp.println("month,day,hour,minute,second,state,supply,name");
            fp.close();
        }

        File fp_data;
        fp_data = SD.open(data_file, FILE_READ);

        String line2;               // 行を一時的に記憶する変数
        int row_inheader2 = 0;      // ヘッダーも含めた行数を記憶する変数
        while (fp_data.available()) // データが有ればループ
        {
            char ch = fp_data.read(); // データを1byteずつ読み取り
            line2 += String(ch);      // 1byteずつ繋げていく
            if (ch == '\n')           //'\n'が読み込まれた所で一旦読み取りをやめる
            {
                line2.trim(); // 行に含まれる不要な空白を取り除く

                int column2 = 0;    // 行に含まれる要素番号カウントする変数
                int num_end2 = 0;   // 要素の終わりの列番号を記憶する変数
                int num_start2 = 0; // 要素の始まりの列番号を記憶する変数

                if (0 < row_inheader2) // ヘッダーを読み込まないように除外する
                {
                    while (num_end2 != -1) // 範囲内に','がなくなるまで繰り返し
                    {
                        num_end2 = line2.indexOf(',', num_start2);           // num_strから','がないか確認。あれば列番号をnum_endに記憶
                        String part = line2.substring(num_start2, num_end2); //','で区切られた要素を行から切り出す

                        if (column2 == 0) // 行の最初の要素
                        {
                            g_data.month[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        }
                        else if (column2 == 1) // 行の2番目の要素
                        {
                            g_data.day[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        }
                        else if (column2 == 2) // 行の2番目の要素
                        {
                            g_data.hour[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        }
                        else if (column2 == 3) // 行の2番目の要素
                        {
                            g_data.minute[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        }
                        else if (column2 == 4) // 行の2番目の要素
                        {
                            g_data.second[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        }
                        else if (column2 == 5) // 行の2番目の要素
                        {
                            g_data.kashi[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        }
                        else if (column2 == 6) // 行の2番目の要素
                        {
                            part.toCharArray(g_data.supply[row_inheader2 - 1], num_end2 + 1 - num_start2);
                        }
                        else if (column2 == 7) // 行の2番目の要素
                        {
                            g_data.name[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        }
                        // if (column2 == 0)                                  // 行の最初の要素
                        // {
                        //     part.toCharArray(g_data.o_supply[row_inheader2 - 1], num_end2 + 1 - num_start2); // 配列の"ヘッダーを除いた行数"番目にデータをchar型に変換して代入
                        // }
                        // else if (column2 == 1) // 行の2番目の要素
                        // {
                        //     g_data.o_name[row_inheader2 - 1] = part.toInt(); // 配列の"ヘッダーを除いた行数"番目にデータをint型に変換して代入
                        // }
                        // else if (column2 == 2) // 行の3番目の要素
                        // {
                        //     g_data.o_kashi[row_inheader2 - 1] = boolean(part.toInt()); // 配列の"ヘッダーを除いた行数"番目にデータをbool型に変換して代入
                        // }
                        num_start2 = num_end2 + 1; //','分で1を足して、次の要素を区切る','を探し始めるスタート地点を代入
                        column2++;                 // 要素数に1を加算
                    }
                }
                line2 = "\0";    // 変数を初期化
                row_inheader2++; // 行数に1を加算
            }
            g_data.row = row_inheader2 - 1; // ヘッダーを除いた行数をrowに代入
        }
    }

    void sd_ob_data_update(char *supply, int name)
    {
        sd_read_data();
        bool newdata = true;
        bool data_state = true;
        if (g_o_data.o_row != 0)
        {
            for (int i = 0; i < g_o_data.o_row; i++)
            {
                if (String(supply).equals(String(g_o_data.o_supply[i])))
                {
                    g_o_data.o_name[i] = name;
                    g_o_data.o_kashi[i] = !g_o_data.o_kashi[i];
                    data_state = g_o_data.o_kashi[i];
                    newdata = false;
                    break;
                }
            }
        }
        RTC_DateTypeDef RTCDate;
        RTC_TimeTypeDef RTCtime;
        M5.Rtc.GetDate(&RTCDate);
        M5.Rtc.GetTime(&RTCtime);
        File fp_d;
        fp_d = SD.open("/DataFiles/" + String(RTCDate.Year) + "/" + String(RTCDate.Year) + "_" + String(RTCDate.Month) + ".csv", FILE_APPEND);
        fp_d.println(String(RTCDate.Month) + "," + String(RTCDate.Date) + "," + String(RTCtime.Hours) + "," + String(RTCtime.Minutes) + "," + String(RTCtime.Seconds) + "," + String(data_state) + "," + String(supply) + "," + String(name));
        fp_d.close();

        File fp_od;
        if (newdata)
        {
            fp_od = SD.open("/DataFiles/objects_state.csv", FILE_APPEND);
            fp_od.println(String(supply) + "," + String(name) + "," + "1");
        }
        else
        {
            fp_od = SD.open("/DataFiles/objects_state.csv", FILE_WRITE);
            fp_od.println("object,name,state");
            // Serial.println(g_o_data.o_row);
            for (int i = 0; i < g_o_data.o_row; i++)
            {
                // Serial.println(String(g_o_data.o_supply[i]) + "," + String(g_o_data.o_name[i]) + "," + String(g_o_data.o_kashi[i]));
                fp_od.println(String(g_o_data.o_supply[i]) + "," + String(g_o_data.o_name[i]) + "," + String(g_o_data.o_kashi[i]));
            }
        }
        fp_od.close();
    }

    bool sd_check_state(String supply)
    {
        for (int i = 0; i < g_o_data.o_row; i++)
        {
            if (String(supply).equals(String(g_o_data.o_supply[i])))
            {
                if (g_o_data.o_kashi[i] == false)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        return true;
    }
}