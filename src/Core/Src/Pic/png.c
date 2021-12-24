#include "png.h"
#include "upng.h"
#include "main.h"
#include "lvgl.h"
#include "Show/show.h"
#include "fatfs.h"
#include "malloc.h"

void dec_png(FIL *file) {
    upng_t *upng;
    unsigned width, height, depth;
    upng_error res;

    upng = upng_new_from_file(file);
    if (upng != NULL) {
        res = upng_header(upng);
        width = upng_get_width(upng);
        height = upng_get_height(upng);
        uint16_t *out = malloc(sizeof(uint16_t) * 4096);

        upng_decode(upng, out);
        if (upng_get_error(upng) != UPNG_EOK) {
            info_show();
            lv_label_set_text_fmt(info, "error: %d %d", upng_get_error(upng), upng_get_error_line(upng));
            while (1);
        }
    } else {
        return;
    }


    depth = upng_get_bpp(upng) / 8;

//    if (upng_get_format(upng) == UPNG_RGB8) {
//        uint8_t *header = bmp_buffer;
//
//    }

    upng_free(upng);
}