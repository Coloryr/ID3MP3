#include "lvgl.h"
#include "main.h"
#include "Flash/w25q64.h"
#include "fatfs.h"
#include "malloc.h"
#include "Tasks/task_start.h"

lv_font_t * sy16;
lv_font_t * sy24;
lv_font_t * sy32;

#define FONT_ADDR 0x200
#define MAX 0x1000000

#define SAVE_ADDR 0x100
#define TEMP_L 0x8000
ramfast FIL font_file;
ramfast uint32_t font_16_local;
ramfast uint32_t font_24_local;
ramfast uint32_t font_32_local;
ramfast uint8_t data_temp[TEMP_L];

union font_len_cov {
    uint32_t u32;
    uint8_t u8[4];
};

void load_font() {
    uint8_t temp[20];
    uint8_t write_temp[5];
    uint16_t len;
    uint32_t pos = 0, now = 0;
    FRESULT res;
    union font_len_cov cov;
    W25QXX_Read(temp, SAVE_ADDR, 20);
    if (temp[0] != 16) {
        res = f_open(&font_file, "0:/font/sy16.font", FA_READ);
        if (res != FR_OK) {
            lv_label_set_text(info, "no file sy16.font");
            while (1);
        }
        now = 0;
        for (;;) {
            res = f_read(&font_file, data_temp, TEMP_L, (UINT *) &len);
            if (res != FR_OK) {
                lv_label_set_text_fmt(info, "file sy16.font error res:%d", res);
                while (1);
            }
            W25QXX_Write(data_temp, FONT_ADDR + pos, len);
            if (len == 0)
                break;
            pos += len;
            now += len;

            lv_label_set_text_fmt(info, "init font 16.. %d/%d", font_file.obj.objsize, now);
        }

        pos += 10;

        font_16_local = cov.u32 = pos;
        write_temp[0] = 16;
        write_temp[1] = cov.u8[0];
        write_temp[2] = cov.u8[1];
        write_temp[3] = cov.u8[2];
        write_temp[4] = cov.u8[3];
        W25QXX_Write(write_temp, SAVE_ADDR, 5);
    } else {
        cov.u8[0] = temp[1];
        cov.u8[1] = temp[2];
        cov.u8[2] = temp[3];
        cov.u8[3] = temp[4];
        font_16_local = pos = cov.u32;
    }

    if (temp[5] != 24) {
        res = f_open(&font_file, "0:/font/sy24.font", FA_READ);
        if (res != FR_OK) {
            lv_label_set_text(info, "no file sy24.font");
            while (1);
        }
        now = 0;
        for (;;) {
            res = f_read(&font_file, data_temp, TEMP_L, (UINT *) &len);
            if (res != FR_OK) {
                lv_label_set_text_fmt(info, "file sy24.font error res:%d", res);
                while (1);
            }
            W25QXX_Write(data_temp, FONT_ADDR + pos, len);
            if (len == 0)
                break;
            pos += len;
            now += len;

            lv_label_set_text_fmt(info, "init font 24.. %d/%d", font_file.obj.objsize, now);
        }
        pos += 10;

        font_24_local = cov.u32 = pos;
        write_temp[0] = 24;
        write_temp[1] = cov.u8[0];
        write_temp[2] = cov.u8[1];
        write_temp[3] = cov.u8[2];
        write_temp[4] = cov.u8[3];
        W25QXX_Write(write_temp, SAVE_ADDR + 5, 5);
    } else {
        cov.u8[0] = temp[6];
        cov.u8[1] = temp[7];
        cov.u8[2] = temp[8];
        cov.u8[3] = temp[9];
        font_24_local = pos = cov.u32;
    }

    if (temp[10] != 32) {
        res = f_open(&font_file, "0:/font/sy32.font", FA_READ);
        if (res != FR_OK) {
            lv_label_set_text(info, "no file sy32.font");
            while (1);
        }
        now = 0;
        for (;;) {
            res = f_read(&font_file, data_temp, TEMP_L, (UINT *) &len);
            if (res != FR_OK) {
                lv_label_set_text_fmt(info, "file sy32.font error res:%d", res);
                while (1);
            }
            W25QXX_Write(data_temp, FONT_ADDR + pos, len);
            if (len == 0)
                break;
            pos += len;
            now += len;

            lv_label_set_text_fmt(info, "init font 32.. %d/%d", font_file.obj.objsize, now);
        }
        pos += 10;

        font_32_local = cov.u32 = pos;
        write_temp[0] = 24;
        write_temp[1] = cov.u8[0];
        write_temp[2] = cov.u8[1];
        write_temp[3] = cov.u8[2];
        write_temp[4] = cov.u8[3];
        W25QXX_Write(temp, SAVE_ADDR + 10, 5);
    } else {
        cov.u8[0] = temp[11];
        cov.u8[1] = temp[12];
        cov.u8[2] = temp[13];
        cov.u8[3] = temp[14];
        font_32_local = pos = cov.u32;
    }
}