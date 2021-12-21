#include "fattester.h"
#include "exfuns.h"
#include "ff.h"

//为磁盘注册工作区
//path:磁盘路径，比如"0:"、"1:"
//mt:0，不立即注册（稍后注册）；1，立即注册
//返回值:执行结果
uint8_t mf_mount(uint8_t *path, uint8_t mt) {
    return f_mount(fs[1], (const TCHAR *) path, mt);
}

//打开路径下的文件
//path:路径+文件名
//mode:打开模式
//返回值:执行结果
uint8_t mf_open(uint8_t *path, uint8_t mode) {
    uint8_t res;
    res = f_open(file, (const TCHAR *) path, mode);//打开文件夹
    return res;
}

//关闭文件
//返回值:执行结果
uint8_t mf_close(void) {
    f_close(file);
    return 0;
}

//读出数据
//len:读出的长度
//返回值:执行结果
uint8_t mf_read(uint16_t len) {
    uint16_t i, t;
    uint8_t res = 0;
    uint16_t tlen = 0;
    for (i = 0; i < len / 512; i++) {
        res = f_read(file, fatbuf, 512, &br);
        if (res) {
            break;
        } else {
            tlen += br;
        }
    }
    if (len % 512) {
        res = f_read(file, fatbuf, len % 512, &br);
    }
    return res;
}

//写入数据
//dat:数据缓存区
//len:写入长度
//返回值:执行结果
uint8_t mf_write(uint8_t *dat, uint16_t len) {
    return f_write(file, dat, len, &bw);
}

//打开目录
//path:路径
//返回值:执行结果
uint8_t mf_opendir(uint8_t *path) {
    return f_opendir(&dir, (const TCHAR *) path);
}

//关闭目录
//返回值:执行结果
uint8_t mf_closedir(void) {
    return f_closedir(&dir);
}

//打读取文件夹
//返回值:执行结果
uint8_t mf_readdir(void) {
    uint8_t res;
    res = f_readdir(&dir, &fileinfo);    //读取一个文件的信息
    if (res != FR_OK)return res;        //出错了
    return 0;
}

//遍历文件
//path:路径
//返回值:执行结果
uint8_t mf_scan_files(uint8_t *path) {
    FRESULT res;
    res = f_opendir(&dir, (const TCHAR *) path); //打开一个目录
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fileinfo);                   //读取目录下的一个文件
            if (res != FR_OK || fileinfo.fname[0] == 0) break;  //错误了/到末尾了,退出
            //if (fileinfo.fname[0] == '.') continue;             //忽略上级目录
        }
    }
    return res;
}

//显示剩余容量
//drv:盘符
//返回值:剩余容量(字节)
uint32_t mf_showfree(uint8_t *drv) {
    FATFS *fs1;
    uint8_t res;
    uint32_t fre_clust = 0, fre_sect = 0, tot_sect = 0;
    //得到磁盘信息及空闲簇数量
    res = f_getfree((const TCHAR *) drv, (DWORD *) &fre_clust, &fs1);
    if (res == 0) {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;//得到总扇区数
        fre_sect = fre_clust * fs1->csize;            //得到空闲扇区数
#if _MAX_SS != 512
        tot_sect*=fs1->ssize/512;
        fre_sect*=fs1->ssize/512;
#endif
    }
    return fre_sect;
}

//文件读写指针偏移
//offset:相对首地址的偏移量
//返回值:执行结果.
uint8_t mf_lseek(uint32_t offset) {
    return f_lseek(file, offset);
}

//读取文件当前读写指针的位置.
//返回值:位置
uint32_t mf_tell(void) {
    return f_tell(file);
}

//读取文件大小
//返回值:文件大小
uint32_t mf_size(void) {
    return f_size(file);
}

//创建目录
//pname:目录路径+名字
//返回值:执行结果
uint8_t mf_mkdir(uint8_t *pname) {
    return f_mkdir((const TCHAR *) pname);
}

//格式化
//path:磁盘路径，比如"0:"、"1:"
//opt:模式,FM_FAT,FM_FAT32,FM_EXFAT,FM_ANY等...
//au:簇大小
//返回值:执行结果
uint8_t mf_fmkfs(uint8_t *path, uint8_t opt, uint16_t au) {
    return f_mkfs((const TCHAR *) path, opt, au, fatbuf, _MAX_SS);//格式化,drv:盘符;opt:模式;au:簇大小,workbuf,最少_MAX_SS大小
}

//删除文件/目录
//pname:文件/目录路径+名字
//返回值:执行结果
uint8_t mf_unlink(uint8_t *pname) {
    return f_unlink((const TCHAR *) pname);
}

//修改文件/目录名字(如果目录不同,还可以移动文件哦!)
//oldname:之前的名字
//newname:新名字
//返回值:执行结果
uint8_t mf_rename(uint8_t *oldname, uint8_t *newname) {
    return f_rename((const TCHAR *) oldname, (const TCHAR *) newname);
}

//获取盘符（磁盘名字）
//path:磁盘路径，比如"0:"、"1:"
void mf_getlabel(uint8_t *path) {
    uint8_t buf[20];
    uint32_t sn = 0;
    uint8_t res;
    res = f_getlabel((const TCHAR *) path, (TCHAR *) buf, (DWORD *) &sn);
}

//设置盘符（磁盘名字），最长11个字符！！，支持数字和大写字母组合以及汉字等
//path:磁盘号+名字，比如"0:ALIENTEK"、"1:OPENEDV"
void mf_setlabel(uint8_t *path) {
    uint8_t res;
    res = f_setlabel((const TCHAR *) path);
}

//从文件里面读取一段字符串
//size:要读取的长度
void mf_gets(uint16_t size) {
    TCHAR *rbuf;
    rbuf = f_gets((TCHAR *) fatbuf, size, file);
    if (*rbuf == 0)return;//没有数据读到
}

//需要_USE_STRFUNC>=1
//写一个字符到文件
//c:要写入的字符
//返回值:执行结果
uint8_t mf_putc(uint8_t c) {
    return f_putc((TCHAR) c, file);
}

//写字符串到文件
//c:要写入的字符串
//返回值:写入的字符串长度
uint8_t mf_puts(uint8_t *c) {
    return f_puts((TCHAR *) c, file);
}
