#ifndef __EXFUNS_H
#define __EXFUNS_H
#include "main.h"
#include "fatfs.h"

extern FATFS *fs[2];
extern FIL *file;
extern FIL *ftemp;
extern UINT br, bw;
extern FILINFO fileinfo;
extern DIR dir;
extern uint8_t *fatbuf; //SD卡数据缓存区

//f_typetell返回的类型定义
//根据表FILE_TYPE_TBL获得.在exfuns.c里面定义
#define T_BIN 0X00  //bin文件
#define T_LRC 0X10  //lrc文件
#define T_NES 0X20  //nes文件
#define T_TEXT 0X30 //.txt文件
#define T_C 0X31    //.c文件
#define T_H 0X32    //.h文件
#define T_BMP 0X50  //bmp文件
#define T_JPG 0X51  //jpg文件
#define T_JPEG 0X52 //jpeg文件
#define T_GIF 0X53  //gif文件
#define T_PNG 0X54  //gif文件

uint8_t exfuns_init(void);                           //申请内存
uint8_t f_typetell(uint8_t *fname);                       //识别文件类型
uint8_t exf_getfree(uint8_t *drv, uint32_t *total, uint32_t *free); //得到磁盘总容量和剩余容量
uint32_t exf_fdsize(uint8_t *fdname);                     //得到文件夹大小
uint8_t exf_copy(uint8_t (*fcpymsg)(uint8_t *pname, uint8_t pct, uint8_t mode), uint8_t *psrc, uint8_t *pdst, uint32_t totsize, uint32_t cpdsize, uint8_t fwmode);
uint8_t *exf_get_src_dname(uint8_t *dpfn);
uint32_t exf_fdsize(uint8_t *fdname);
uint8_t exf_fdcopy(uint8_t (*fcpymsg)(uint8_t *pname, uint8_t pct, uint8_t mode), uint8_t *psrc, uint8_t *pdst, uint32_t *totsize, uint32_t *cpdsize, uint8_t fwmode);
#endif
