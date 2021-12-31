#include "lvgl.h"
#include "main.h"
#include "qspi/w25q64.h"
#include "fatfs.h"
#include "malloc.h"
#include "Tasks/task_start.h"
#include "myfont.h"
#include "Show/show.h"
#include "misc/lv_utils.h"

#define FONT_ADDR 0x1000
#define MAX 0x1000000

#define SAVE_ADDR 0x100
#define TEMP_L 0x8000

#define SAVE_BIT 67

typedef union {
    uint32_t u32;
    uint8_t u8[4];
}font_len_cov;

typedef struct cmap_table_bin {
    uint32_t data_offset;
    uint32_t range_start;
    uint16_t range_length;
    uint16_t glyph_id_start;
    uint16_t data_entries_count;
    uint8_t format_type;
    uint8_t padding;
} cmap_table_bin_t;

typedef struct {
    w25qxx_utils *fp;
    int8_t bit_pos;
    uint8_t byte_value;
} bit_iterator_t;

static int32_t font_read_label(w25qxx_utils *head, uint32_t start, const char *label);

static void font_load(my_font_data *font, uint32_t local);

static int32_t font_load_cmaps(w25qxx_utils *head, lv_font_fmt_txt_dsc_t *font_dsc, uint32_t cmaps_start);

static bool font_load_cmaps_tables(w25qxx_utils *head, lv_font_fmt_txt_dsc_t *font_dsc,
                                   uint32_t cmaps_start, cmap_table_bin_t *cmap_table);

static int32_t font_load_kern(w25qxx_utils *head, lv_font_fmt_txt_dsc_t *font_dsc, uint8_t format, uint32_t start);

static int32_t font_load_glyph(w25qxx_utils *head, uint32_t start);

static bit_iterator_t font_init_bit_iterator(w25qxx_utils *fp);

static unsigned int font_read_bits(bit_iterator_t *it, int n_bits);

static int font_read_bits_signed(bit_iterator_t *it, int n_bits);

static uint32_t font_get_glyph_dsc_id(const lv_font_t *font, uint32_t letter);

static int32_t font_unicode_list_compare(const void *ref, const void *element);

static int32_t font_kern_pair_16_compare(const void *ref, const void *element);

static int32_t font_kern_pair_8_compare(const void *ref, const void *element);

static int8_t font_get_kern_value(const lv_font_t *font, uint32_t gid_left, uint32_t gid_right);

ramfast FIL font_file;

ramfast my_font_data font_16;
ramfast my_font_data font_24;
ramfast my_font_data font_32;

ramfast font_len_cov cov;

uint8_t data_temp[TEMP_L];

void font_get_glyph_dsc(const lv_font_t *font, uint32_t gid, lv_font_fmt_txt_glyph_dsc_t *gdsc) {
    w25qxx_utils *head = malloc(sizeof(w25qxx_utils));
    my_font_data *font_data;

    if (font == &font_16.font) {
        font_data = &font_16;
        head->local = FONT_ADDR;
    } else if (font == &font_24.font) {
        font_data = &font_24;
        head->local = FONT_ADDR + font_16.length;
    } else {
        font_data = &font_32;
        head->local = FONT_ADDR + font_16.length + font_24.length;
    }

    uint32_t now;
    if (font_data->header_bin.index_to_loc_format == 0) {
        head->pos = font_data->loca_start + gid * sizeof(uint16_t);
        W25QXX_Read_Utils(head, &now, sizeof(uint16_t));
    } else {
        head->pos = font_data->loca_start + gid * sizeof(uint32_t);
        W25QXX_Read_Utils(head, &now, sizeof(uint32_t));
    }
    head->pos = font_data->glyph_start + now;

    bit_iterator_t bit_it = font_init_bit_iterator(head);

    if (font_data->header_bin.advance_width_bits == 0) {
        gdsc->adv_w = font_data->header_bin.default_advance_width;
    } else {
        gdsc->adv_w = font_read_bits(&bit_it, font_data->header_bin.advance_width_bits);
    }

    if (font_data->header_bin.advance_width_format == 0) {
        gdsc->adv_w *= 16;
    }

    gdsc->ofs_x = font_read_bits_signed(&bit_it, font_data->header_bin.xy_bits);
    gdsc->ofs_y = font_read_bits_signed(&bit_it, font_data->header_bin.xy_bits);
    gdsc->box_w = font_read_bits(&bit_it, font_data->header_bin.wh_bits);
    gdsc->box_h = font_read_bits(&bit_it, font_data->header_bin.wh_bits);

    free(head);
}

bool my_get_glyph_dsc_cb(const lv_font_t *font, lv_font_glyph_dsc_t *dsc_out, uint32_t unicode_letter,
                         uint32_t unicode_letter_next) {
    bool is_tab = false;
    if (unicode_letter == '\t') {
        unicode_letter = ' ';
        is_tab = true;
    }

    uint32_t gid = font_get_glyph_dsc_id(font, unicode_letter);
    if (!gid) return NULL;

    lv_font_fmt_txt_dsc_t *fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;

    int8_t kvalue = 0;
    if (fdsc->kern_dsc) {
        uint32_t gid_next = font_get_glyph_dsc_id(font, unicode_letter_next);
        if (gid_next) {
            kvalue = font_get_kern_value(font, gid, gid_next);
        }
    }

    lv_font_fmt_txt_glyph_dsc_t *gdsc = malloc(sizeof(lv_font_fmt_txt_glyph_dsc_t));
    font_get_glyph_dsc(font, gid, gdsc);

    int32_t kv = ((int32_t) ((int32_t) kvalue * fdsc->kern_scale) >> 4);

    uint32_t adv_w = gdsc->adv_w;
    if (is_tab) adv_w *= 2;

    adv_w += kv;
    adv_w = (adv_w + (1 << 3)) >> 4;

    dsc_out->adv_w = adv_w;
    dsc_out->box_h = gdsc->box_h;
    dsc_out->box_w = gdsc->box_w;
    dsc_out->ofs_x = gdsc->ofs_x;
    dsc_out->ofs_y = gdsc->ofs_y;
    dsc_out->bpp = (uint8_t) fdsc->bpp;

    if (is_tab) dsc_out->box_w = dsc_out->box_w * 2;

    free(gdsc);

    return true;
}

const uint8_t *my_get_glyph_bitmap_cb(const lv_font_t *font, uint32_t unicode_letter) {
    w25qxx_utils *head = malloc(sizeof(w25qxx_utils));
    uint32_t next;
    uint32_t now;
    uint32_t gid;
    bit_iterator_t bit_it;
    int nbits;
    uint32_t next_offset;
    uint32_t bmp_size;
    lv_font_fmt_txt_dsc_t *fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;
    my_font_data *font_data;

    if (font == &font_16.font) {
        font_data = &font_16;
        head->local = FONT_ADDR;
    } else if (font == &font_24.font) {
        font_data = &font_24;
        head->local = FONT_ADDR + font_16.length;
    } else {
        font_data = &font_32;
        head->local = FONT_ADDR + font_16.length + font_24.length;
    }

    if (unicode_letter == '\t') unicode_letter = ' ';

    gid = font_get_glyph_dsc_id(font, unicode_letter);
    if (!gid) return NULL;

    if (font_data->header_bin.index_to_loc_format == 0) {
        head->pos = font_data->loca_start + gid * sizeof(uint16_t);
        W25QXX_Read_Utils(head, &now, sizeof(uint16_t));
        W25QXX_Read_Utils(head, &next, sizeof(uint16_t));
    } else {
        head->pos = font_data->loca_start + gid * sizeof(uint32_t);
        W25QXX_Read_Utils(head, &now, sizeof(uint32_t));
        W25QXX_Read_Utils(head, &next, sizeof(uint32_t));
    }
    now = font_data->glyph_start + now;
    head->pos = now;

    bit_it = font_init_bit_iterator(head);

    nbits = font_data->header_bin.advance_width_bits + 2 * font_data->header_bin.xy_bits +
            2 * font_data->header_bin.wh_bits;

    font_read_bits(&bit_it, nbits);

    if (gid < font_data->loca_count) {
        next = font_data->glyph_start + next;
        next_offset = next;
    } else {
        next_offset = font_data->glyph_length;
    }

    bmp_size = next_offset - now - nbits / 8;

    if (nbits % 8 == 0) {  /*Fast path*/
        W25QXX_Read_Utils(head, font_data->glyph_bmp, bmp_size);
    } else {
        for (int k = 0; k < bmp_size - 1; ++k) {
            font_data->glyph_bmp[k] = font_read_bits(&bit_it, 8);
        }
        font_data->glyph_bmp[bmp_size - 1] = font_read_bits(&bit_it, 8 - nbits % 8);

        /*The last fragment should be on the MSB but read_bits() will place it to the LSB*/
        font_data->glyph_bmp[bmp_size - 1] = font_data->glyph_bmp[bmp_size - 1] << (nbits % 8);
    }

    if (fdsc->bitmap_format == LV_FONT_FMT_TXT_PLAIN) {
        return font_data->glyph_bmp;
    }

    free(head);

    return NULL;
}

uint32_t write_font(uint32_t pos, uint32_t addr, const char *path) {
    FRESULT res;
    uint16_t len;
    uint32_t now;
    uint8_t write_temp[5];
    res = f_open(&font_file, path, FA_READ);
    if (res != FR_OK) {
        lv_label_set_text_fmt(info, "no file %s", path);
        while (1);
    }
    now = 0;
    for (;;) {
        res = f_read(&font_file, data_temp, TEMP_L, (UINT *) &len);
        if (res != FR_OK) {
            lv_label_set_text_fmt(info, "file %s error res:%d", path, res);
            while (1);
        }
        W25QXX_Write(data_temp, pos + now, len);
        if (len == 0)
            break;
        now += len;
        if (now == font_file.obj.objsize)
            break;

        lv_label_set_text_fmt(info, "init font %s %d/%d", path, font_file.obj.objsize, now);
    }
    if (now != font_file.obj.objsize) {
        lv_label_set_text_fmt(info, "init font %s error:size check fail", path);
        while (1);
    }

    lv_label_set_text_fmt(info, "init font %s done.", path);

    f_close(&font_file);

    cov.u32 = now + 10;
    write_temp[0] = SAVE_BIT;
    write_temp[1] = cov.u8[0];
    write_temp[2] = cov.u8[1];
    write_temp[3] = cov.u8[2];
    write_temp[4] = cov.u8[3];
    osDelay(10);
    W25QXX_Write(write_temp, addr, 5);

    return cov.u32;
}

void load_font() {
    uint8_t temp[20];
    uint32_t pos = FONT_ADDR;

    W25QXX_Read(temp, SAVE_ADDR, 20);
    if (temp[0] != SAVE_BIT) {
        font_16.length = write_font(pos, SAVE_ADDR, "0:/font/16.font");
    } else {
        cov.u8[0] = temp[1];
        cov.u8[1] = temp[2];
        cov.u8[2] = temp[3];
        cov.u8[3] = temp[4];
        font_16.length = cov.u32;
    }

    pos = FONT_ADDR + font_16.length;

    if (temp[5] != SAVE_BIT) {
        font_24.length = write_font(pos, SAVE_ADDR + 5, "0:/font/24.font");
    } else {
        cov.u8[0] = temp[6];
        cov.u8[1] = temp[7];
        cov.u8[2] = temp[8];
        cov.u8[3] = temp[9];
        font_24.length = cov.u32;
    }

    pos = FONT_ADDR + font_16.length + font_24.length;

    if (temp[10] != SAVE_BIT) {
        font_32.length = write_font(pos, SAVE_ADDR + 10, "0:/font/32.font");
    } else {
        cov.u8[0] = temp[11];
        cov.u8[1] = temp[12];
        cov.u8[2] = temp[13];
        cov.u8[3] = temp[14];
        font_32.length = cov.u32;
    }

    font_16.glyph_bmp = malloc(sizeof(uint8_t) * 2 * 16 * 3);
    font_24.glyph_bmp = malloc(sizeof(uint8_t) * 3 * 24 * 3);
    font_32.glyph_bmp = malloc(sizeof(uint8_t) * 4 * 32 * 3);

    font_load(&font_16, FONT_ADDR);
    font_load(&font_24, FONT_ADDR + font_16.length);
    font_load(&font_32, FONT_ADDR + font_16.length + font_24.length);

    lv_label_set_text(info, "font init done");
}

static void font_load(my_font_data *font, uint32_t local) {
    w25qxx_utils *head = malloc(sizeof(w25qxx_utils));
    head->local = local;
    int32_t header_length = font_read_label(head, 0, "head");
    if (header_length < 0) {
        lv_label_set_text(info, "font error:head error");
        while (1);
    }

    W25QXX_Read_Utils(head, &font->header_bin, sizeof(font_header_bin_t));

    font->font.base_line = -font->header_bin.descent;
    font->font.line_height = font->header_bin.ascent - font->header_bin.descent;
    font->font.get_glyph_dsc = my_get_glyph_dsc_cb;
    font->font.get_glyph_bitmap = my_get_glyph_bitmap_cb;
    font->font.subpx = font->header_bin.subpixels_mode;
    font->font.underline_position = font->header_bin.underline_position;
    font->font.underline_thickness = font->header_bin.underline_thickness;

    font->font.dsc = &font->dsc;
    lv_font_fmt_txt_dsc_t *font_dsc = (lv_font_fmt_txt_dsc_t *) font->font.dsc;
    font_dsc->cache = &font->cache;

    font_dsc->bpp = font->header_bin.bits_per_pixel;
    font_dsc->kern_scale = font->header_bin.kerning_scale;
    font_dsc->bitmap_format = font->header_bin.compression_id;

    uint32_t cmaps_start = header_length;
    int32_t cmaps_length = font_load_cmaps(head, font_dsc, cmaps_start);
    if (cmaps_length < 0) {
        lv_label_set_text(info, "font error:cmaps error");
        while (1);
    }

    uint32_t loca_start = header_length + cmaps_length;
    int32_t loca_length = font_read_label(head, loca_start, "loca");
    if (loca_length < 0) {
        lv_label_set_text(info, "font error:loca error");
        while (1);
    }

    uint32_t loca_count;
    W25QXX_Read_Utils(head, &loca_count, sizeof(uint32_t));

    /*glyph*/
    uint32_t glyph_start = loca_start + loca_length;
    int32_t glyph_length = font_load_glyph(head, glyph_start);

    font->glyph_start = glyph_start;
    font->loca_count = loca_count;
    font->glyph_length = glyph_length;
    font->loca_start = loca_start + 12;

    if (font->header_bin.tables_count < 4) {
        font_dsc->kern_dsc = NULL;
        font_dsc->kern_classes = 0;
        font_dsc->kern_scale = 0;
        return;
    }

    uint32_t kern_start = glyph_start + glyph_length;
    int32_t kern_length = font_load_kern(head, font_dsc, font->header_bin.glyph_id_format, kern_start);
    if (kern_length < 0) {
        lv_label_set_text(info, "font 16 error:kern_length is error");
        while (1);
    }
}

static int32_t font_load_glyph(w25qxx_utils *head, uint32_t start) {
    return font_read_label(head, start, "glyf");
}

static int32_t font_read_label(w25qxx_utils *head, uint32_t start, const char *label) {
    uint32_t length;
    uint8_t buf[8];
    uint8_t i;

    head->pos = start;

    W25QXX_Read_Utils(head, buf, 8);
    cov.u8[0] = buf[0];
    cov.u8[1] = buf[1];
    cov.u8[2] = buf[2];
    cov.u8[3] = buf[3];
    length = cov.u32;

    for (i = 0; i < 4; i++) {
        if (buf[i + 4] != label[i])
            return -1;
    }

    return length;
}

static bool font_load_cmaps_tables(w25qxx_utils *head, lv_font_fmt_txt_dsc_t *font_dsc,
                                   uint32_t cmaps_start, cmap_table_bin_t *cmap_table) {
    W25QXX_Read_Utils(head, cmap_table, font_dsc->cmap_num * sizeof(cmap_table_bin_t));
    for (unsigned int i = 0; i < font_dsc->cmap_num; ++i) {
        head->pos = cmaps_start + cmap_table[i].data_offset;
        lv_font_fmt_txt_cmap_t *cmap = (lv_font_fmt_txt_cmap_t *) &(font_dsc->cmaps[i]);

        cmap->range_start = cmap_table[i].range_start;
        cmap->range_length = cmap_table[i].range_length;
        cmap->glyph_id_start = cmap_table[i].glyph_id_start;
        cmap->type = cmap_table[i].format_type;

        switch (cmap_table[i].format_type) {
            case LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL: {
                uint8_t ids_size = sizeof(uint8_t) * cmap_table[i].data_entries_count;
                uint8_t *glyph_id_ofs_list = malloc(ids_size);

                cmap->glyph_id_ofs_list = glyph_id_ofs_list;

                W25QXX_Read_Utils(head, glyph_id_ofs_list, ids_size);

                cmap->list_length = cmap->range_length;
                break;
            }
            case LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY:
                break;
            case LV_FONT_FMT_TXT_CMAP_SPARSE_FULL:
            case LV_FONT_FMT_TXT_CMAP_SPARSE_TINY: {
                uint32_t list_size = sizeof(uint16_t) * cmap_table[i].data_entries_count;
                uint16_t *unicode_list = (uint16_t *) malloc(list_size);

                cmap->unicode_list = unicode_list;
                cmap->list_length = cmap_table[i].data_entries_count;

                W25QXX_Read_Utils(head, unicode_list, list_size);

                if (cmap_table[i].format_type == LV_FONT_FMT_TXT_CMAP_SPARSE_FULL) {
                    uint16_t *buf = malloc(sizeof(uint16_t) * cmap->list_length);

                    cmap->glyph_id_ofs_list = buf;

                    W25QXX_Read_Utils(head, buf, sizeof(uint16_t) * cmap->list_length);
                }
                break;
            }
            default:
                LV_LOG_WARN("Unknown cmaps format type %d.", cmap_table[i].format_type);
                return false;
        }
    }
    return true;
}

static int32_t font_load_cmaps(w25qxx_utils *head, lv_font_fmt_txt_dsc_t *font_dsc, uint32_t cmaps_start) {
    int32_t cmaps_length = font_read_label(head, cmaps_start, "cmap");
    if (cmaps_length < 0) {
        return -1;
    }

    uint32_t cmaps_subtables_count;
    W25QXX_Read_Utils(head, &cmaps_subtables_count, sizeof(uint32_t));

    lv_font_fmt_txt_cmap_t *cmaps =
            malloc(cmaps_subtables_count * sizeof(lv_font_fmt_txt_cmap_t));

    memset(cmaps, 0, cmaps_subtables_count * sizeof(lv_font_fmt_txt_cmap_t));

    font_dsc->cmaps = cmaps;
    font_dsc->cmap_num = cmaps_subtables_count;

    cmap_table_bin_t *cmaps_tables = malloc(sizeof(cmap_table_bin_t) * font_dsc->cmap_num);

    bool success = font_load_cmaps_tables(head, font_dsc, cmaps_start, cmaps_tables);

    free(cmaps_tables);

    return success ? cmaps_length : -1;
}

static int32_t font_load_kern(w25qxx_utils *head, lv_font_fmt_txt_dsc_t *font_dsc, uint8_t format, uint32_t start) {
    int32_t kern_length = font_read_label(head, start, "kern");
    if (kern_length < 0) {
        return -1;
    }

    uint8_t kern_format_type;
    int32_t padding;
    W25QXX_Read_Utils(head, &kern_format_type, sizeof(uint8_t));
    W25QXX_Read_Utils(head, &padding, 3 * sizeof(uint8_t));

    if (0 == kern_format_type) { /*sorted pairs*/
        lv_font_fmt_txt_kern_pair_t *kern_pair = malloc(sizeof(lv_font_fmt_txt_kern_pair_t));

        memset(kern_pair, 0, sizeof(lv_font_fmt_txt_kern_pair_t));

        font_dsc->kern_dsc = kern_pair;
        font_dsc->kern_classes = 0;

        uint32_t glyph_entries;
        W25QXX_Read_Utils(head, &glyph_entries, sizeof(uint32_t));

        int ids_size;
        if (format == 0) {
            ids_size = sizeof(int8_t) * 2 * glyph_entries;
        } else {
            ids_size = sizeof(int16_t) * 2 * glyph_entries;
        }

        uint8_t *glyph_ids = malloc(ids_size);
        int8_t *values = malloc(glyph_entries);

        kern_pair->glyph_ids_size = format;
        kern_pair->pair_cnt = glyph_entries;
        kern_pair->glyph_ids = glyph_ids;
        kern_pair->values = values;

        W25QXX_Read_Utils(head, glyph_ids, ids_size);
        W25QXX_Read_Utils(head, values, glyph_entries);
    } else if (3 == kern_format_type) { /*array M*N of classes*/

        lv_font_fmt_txt_kern_classes_t *kern_classes = malloc(sizeof(lv_font_fmt_txt_kern_classes_t));

        memset(kern_classes, 0, sizeof(lv_font_fmt_txt_kern_classes_t));

        font_dsc->kern_dsc = kern_classes;
        font_dsc->kern_classes = 1;

        uint16_t kern_class_mapping_length;
        uint8_t kern_table_rows;
        uint8_t kern_table_cols;

        W25QXX_Read_Utils(head, &kern_class_mapping_length, sizeof(uint16_t));
        W25QXX_Read_Utils(head, &kern_table_rows, sizeof(uint8_t));
        W25QXX_Read_Utils(head, &kern_table_cols, sizeof(uint8_t));

        int kern_values_length = sizeof(int8_t) * kern_table_rows * kern_table_cols;

        uint8_t *kern_left = malloc(kern_class_mapping_length);
        uint8_t *kern_right = malloc(kern_class_mapping_length);
        int8_t *kern_values = malloc(kern_values_length);

        kern_classes->left_class_mapping = kern_left;
        kern_classes->right_class_mapping = kern_right;
        kern_classes->left_class_cnt = kern_table_rows;
        kern_classes->right_class_cnt = kern_table_cols;
        kern_classes->class_pair_values = kern_values;

        W25QXX_Read_Utils(head, kern_left, kern_class_mapping_length);
        W25QXX_Read_Utils(head, kern_right, kern_class_mapping_length);
        W25QXX_Read_Utils(head, kern_values, kern_values_length);
    } else {
        lv_label_set_text_fmt(info, "Unknown kern_format_type: %d", kern_format_type);
        while (1);
    }

    return kern_length;
}

static bit_iterator_t font_init_bit_iterator(w25qxx_utils *fp) {
    bit_iterator_t it;
    it.fp = fp;
    it.bit_pos = -1;
    it.byte_value = 0;
    return it;
}

static unsigned int font_read_bits(bit_iterator_t *it, int n_bits) {
    unsigned int value = 0;
    while (n_bits--) {
        it->byte_value = it->byte_value << 1;
        it->bit_pos--;

        if (it->bit_pos < 0) {
            it->bit_pos = 7;
            W25QXX_Read_Utils(it->fp, &(it->byte_value), 1);
        }
        int8_t bit = (it->byte_value & 0x80) ? 1 : 0;

        value |= (bit << n_bits);
    }
    return value;
}

static int font_read_bits_signed(bit_iterator_t *it, int n_bits) {
    unsigned int value = font_read_bits(it, n_bits);
    if (value & (1 << (n_bits - 1))) {
        value |= ~0u << n_bits;
    }
    return value;
}

static uint32_t font_get_glyph_dsc_id(const lv_font_t *font, uint32_t letter) {
    if (letter == '\0') return 0;

    lv_font_fmt_txt_dsc_t *fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;

    /*Check the cache first*/
    if (fdsc->cache && letter == fdsc->cache->last_letter) return fdsc->cache->last_glyph_id;

    uint16_t i;
    for (i = 0; i < fdsc->cmap_num; i++) {

        /*Relative code point*/
        uint32_t rcp = letter - fdsc->cmaps[i].range_start;
        if (rcp > fdsc->cmaps[i].range_length) continue;
        uint32_t glyph_id = 0;
        if (fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY) {
            glyph_id = fdsc->cmaps[i].glyph_id_start + rcp;
        } else if (fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_FORMAT0_FULL) {
            const uint8_t *gid_ofs_8 = fdsc->cmaps[i].glyph_id_ofs_list;
            glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_8[rcp];
        } else if (fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_TINY) {
            uint16_t key = rcp;
            uint16_t *p = _lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                            sizeof(fdsc->cmaps[i].unicode_list[0]), font_unicode_list_compare);

            if (p) {
                lv_uintptr_t ofs = p - fdsc->cmaps[i].unicode_list;
                glyph_id = fdsc->cmaps[i].glyph_id_start + ofs;
            }
        } else if (fdsc->cmaps[i].type == LV_FONT_FMT_TXT_CMAP_SPARSE_FULL) {
            uint16_t key = rcp;
            uint16_t *p = _lv_utils_bsearch(&key, fdsc->cmaps[i].unicode_list, fdsc->cmaps[i].list_length,
                                            sizeof(fdsc->cmaps[i].unicode_list[0]), font_unicode_list_compare);

            if (p) {
                lv_uintptr_t ofs = p - fdsc->cmaps[i].unicode_list;
                const uint16_t *gid_ofs_16 = fdsc->cmaps[i].glyph_id_ofs_list;
                glyph_id = fdsc->cmaps[i].glyph_id_start + gid_ofs_16[ofs];
            }
        }

        /*Update the cache*/
        if (fdsc->cache) {
            fdsc->cache->last_letter = letter;
            fdsc->cache->last_glyph_id = glyph_id;
        }
        return glyph_id;
    }

    if (fdsc->cache) {
        fdsc->cache->last_letter = letter;
        fdsc->cache->last_glyph_id = 0;
    }
    return 0;

}

static int32_t font_unicode_list_compare(const void *ref, const void *element) {
    return ((int32_t) (*(uint16_t *) ref)) - ((int32_t) (*(uint16_t *) element));
}

static int8_t font_get_kern_value(const lv_font_t *font, uint32_t gid_left, uint32_t gid_right) {
    lv_font_fmt_txt_dsc_t *fdsc = (lv_font_fmt_txt_dsc_t *) font->dsc;

    int8_t value = 0;

    if (fdsc->kern_classes == 0) {
        /*Kern pairs*/
        const lv_font_fmt_txt_kern_pair_t *kdsc = fdsc->kern_dsc;
        if (kdsc->glyph_ids_size == 0) {
            /*Use binary search to find the kern value.
             *The pairs are ordered left_id first, then right_id secondly.*/
            const uint16_t *g_ids = kdsc->glyph_ids;
            uint16_t g_id_both = (gid_right << 8) + gid_left; /*Create one number from the ids*/
            uint16_t *kid_p = _lv_utils_bsearch(&g_id_both, g_ids, kdsc->pair_cnt, 2, font_kern_pair_8_compare);

            /*If the `g_id_both` were found get its index from the pointer*/
            if (kid_p) {
                lv_uintptr_t ofs = kid_p - g_ids;
                value = kdsc->values[ofs];
            }
        } else if (kdsc->glyph_ids_size == 1) {
            /*Use binary search to find the kern value.
             *The pairs are ordered left_id first, then right_id secondly.*/
            const uint32_t *g_ids = kdsc->glyph_ids;
            uint32_t g_id_both = (gid_right << 16) + gid_left; /*Create one number from the ids*/
            uint32_t *kid_p = _lv_utils_bsearch(&g_id_both, g_ids, kdsc->pair_cnt, 4, font_kern_pair_16_compare);

            /*If the `g_id_both` were found get its index from the pointer*/
            if (kid_p) {
                lv_uintptr_t ofs = kid_p - g_ids;
                value = kdsc->values[ofs];
            }

        } else {
            /*Invalid value*/
        }
    } else {
        /*Kern classes*/
        const lv_font_fmt_txt_kern_classes_t *kdsc = fdsc->kern_dsc;
        uint8_t left_class = kdsc->left_class_mapping[gid_left];
        uint8_t right_class = kdsc->right_class_mapping[gid_right];

        /*If class = 0, kerning not exist for that glyph
         *else got the value form `class_pair_values` 2D array*/
        if (left_class > 0 && right_class > 0) {
            value = kdsc->class_pair_values[(left_class - 1) * kdsc->right_class_cnt + (right_class - 1)];
        }

    }
    return value;
}

static int32_t font_kern_pair_8_compare(const void *ref, const void *element) {
    const uint8_t *ref8_p = ref;
    const uint8_t *element8_p = element;

    /*If the MSB is different it will matter. If not return the diff. of the LSB*/
    if (ref8_p[0] != element8_p[0]) return (int32_t) ref8_p[0] - element8_p[0];
    else return (int32_t) ref8_p[1] - element8_p[1];

}

static int32_t font_kern_pair_16_compare(const void *ref, const void *element) {
    const uint16_t *ref16_p = ref;
    const uint16_t *element16_p = element;

    /*If the MSB is different it will matter. If not return the diff. of the LSB*/
    if (ref16_p[0] != element16_p[0]) return (int32_t) ref16_p[0] - element16_p[0];
    else return (int32_t) ref16_p[1] - element16_p[1];
}