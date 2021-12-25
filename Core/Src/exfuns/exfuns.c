#include "string.h"
#include "exfuns.h"
#include "malloc.h"

//文件类型列表
const uint8_t *FILE_TYPE_TBL[6][13] =
	{
		{"BIN"},																			  //BIN文件
		{"LRC"},																			  //LRC文件
		{"NES"},																			  //NES文件
		{"TXT", "C", "H"},																	  //文本文件
		{"MP1", "MP2", "MP3", "MP4", "M4A", "3GP", "3G2", "OGG", "ACC", "WMA", "WAV", "MID"}, //音乐文件
		{"BMP", "JPG", "JPEG", "GIF", "PNG"},												  //图片文件
};
///////////////////////////////公共文件区,使用malloc的时候////////////////////////////////////////////
FATFS *fs[2];	 //逻辑磁盘工作区.
FIL *file;		  //文件1
FIL *ftemp;		  //文件2.
UINT br, bw;	  //读写变量
FILINFO fileinfo; //文件信息
DIR dir;		  //目录

uint8_t *fatbuf; //SD卡数据缓存区
///////////////////////////////////////////////////////////////////////////////////////
//为exfuns申请内存
//返回值:0,成功
//1,失败
uint8_t exfuns_init(void)
{
	fs[0] = (FATFS *)malloc(sizeof(FATFS)); //为磁盘0工作区申请内存
	fs[1] = (FATFS *)malloc(sizeof(FATFS)); //为磁盘1工作区申请内存
	file = (FIL *)malloc(sizeof(FIL));	  //为file申请内存
	ftemp = (FIL *)malloc(sizeof(FIL));	 //为ftemp申请内存
	fatbuf = (uint8_t *)malloc(512);			  //为fatbuf申请内存
	if (fs[0] && fs[1] && file && ftemp && fatbuf)
		return 0; //申请有一个失败,即失败.
	else
		return 1;
}

//将小写字母转为大写字母,如果是数字,则保持不变.
uint8_t char_upper(uint8_t c)
{
	if (c < 'A')
		return c; //数字,保持不变.
	if (c >= 'a')
		return c - 0x20; //变为大写.
	else
		return c; //大写,保持不变
}
//报告文件的类型
//fname:文件名
//返回值:0XFF,表示无法识别的文件类型编号.
//		 其他,高四位表示所属大类,低四位表示所属小类.
uint8_t f_typetell(uint8_t *fname)
{
	uint8_t tbuf[5];
	uint8_t *attr = '\0'; //后缀名
	uint8_t i = 0, j;
	while (i < 250)
	{
		i++;
		if (*fname == '\0')
			break; //偏移到了最后了.
		fname++;
	}
	if (i == 250)
		return 0XFF;		//错误的字符串.
	for (i = 0; i < 5; i++) //得到后缀名
	{
		fname--;
		if (*fname == '.')
		{
			fname++;
			attr = fname;
			break;
		}
	}
	strcpy((char *)tbuf, (const char *)attr); //copy
	for (i = 0; i < 4; i++)
		tbuf[i] = char_upper(tbuf[i]); //全部变为大写
	for (i = 0; i < 6; i++)
	{
		for (j = 0; j < 13; j++)
		{
			if (*FILE_TYPE_TBL[i][j] == 0)
				break;																//此组已经没有可对比的成员了.
			if (strcmp((const char *)FILE_TYPE_TBL[i][j], (const char *)tbuf) == 0) //找到了
			{
				return (i << 4) | j;
			}
		}
	}
	return 0XFF; //没找到
}

//得到磁盘剩余容量
//drv:磁盘编号("0:"/"1:")
//total:总容量	 （单位KB）
//free:剩余容量	 （单位KB）
//返回值:0,正常.其他,错误代码
uint8_t exf_getfree(uint8_t *drv, uint32_t *total, uint32_t *free)
{
	FATFS *fs1;
	uint8_t res;
	uint32_t fre_clust = 0, fre_sect = 0, tot_sect = 0;
	//得到磁盘信息及空闲簇数量
	res = (uint32_t)f_getfree((const TCHAR *)drv, (DWORD *)&fre_clust, &fs1);
	if (res == 0)
	{
		tot_sect = (fs1->n_fatent - 2) * fs1->csize; //得到总扇区数
		fre_sect = fre_clust * fs1->csize;			 //得到空闲扇区数
#if _MAX_SS != 512									 //扇区大小不是512字节,则转换为512字节
		tot_sect *= fs1->ssize / 512;
		fre_sect *= fs1->ssize / 512;
#endif
		*total = tot_sect >> 1; //单位为KB
		*free = fre_sect >> 1;  //单位为KB
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//文件复制
//注意文件大小不要超过4GB.
//将psrc文件,copy到pdst.
//fcpymsg,函数指针,用于实现拷贝时的信息显示
//        pname:文件/文件夹名
//        pct:百分比
//        mode:
//			[0]:更新文件名
//			[1]:更新百分比pct
//			[2]:更新文件夹
//			[3~7]:保留
//psrc,pdst:源文件和目标文件
//totsize:总大小(当totsize为0的时候,表示仅仅为单个文件拷贝)
//cpdsize:已复制了的大小.
//fwmode:文件写入模式
//0:不覆盖原有的文件
//1:覆盖原有的文件
//返回值:0,正常
//    其他,错误,0XFF,强制退出
uint8_t exf_copy(uint8_t (*fcpymsg)(uint8_t *pname, uint8_t pct, uint8_t mode), uint8_t *psrc, uint8_t *pdst, uint32_t totsize, uint32_t cpdsize, uint8_t fwmode)
{
	uint8_t res;
	uint16_t br = 0;
	uint16_t bw = 0;
	FIL *fsrc = 0;
	FIL *fdst = 0;
	uint8_t *fbuf = 0;
	uint8_t curpct = 0;
	fsrc = (FIL *)malloc(sizeof(FIL)); //申请内存
	fdst = (FIL *)malloc(sizeof(FIL));
	fbuf = (uint8_t *)malloc(8192);
	if (fsrc == NULL || fdst == NULL || fbuf == NULL)
		res = 100; //前面的值留给fatfs
	else
	{
		if (fwmode == 0)
			fwmode = FA_CREATE_NEW; //不覆盖
		else
			fwmode = FA_CREATE_ALWAYS; //覆盖存在的文件

		res = f_open(fsrc, (const TCHAR *)psrc, FA_READ | FA_OPEN_EXISTING); //打开只读文件
		if (res == 0)
			res = f_open(fdst, (const TCHAR *)pdst, FA_WRITE | fwmode); //第一个打开成功,才开始打开第二个
		if (res == 0)													//两个都打开成功了
		{
			if (totsize == 0) //仅仅是单个文件复制
			{
				totsize = fsrc->fsize;
				cpdsize = 0;
				curpct = 0;
			}
			else
				curpct = (cpdsize * 100) / totsize; //得到新百分比
			fcpymsg(psrc, curpct, 0X02);			//更新百分比
			while (res == 0)						//开始复制
			{
				res = f_read(fsrc, fbuf, 8192, (UINT *)&br); //源头读出512字节
				if (res || br == 0)
					break;
				res = f_write(fdst, fbuf, (UINT)br, (UINT *)&bw); //写入目的文件
				cpdsize += bw;
				if (curpct != (cpdsize * 100) / totsize) //是否需要更新百分比
				{
					curpct = (cpdsize * 100) / totsize;
					if (fcpymsg(psrc, curpct, 0X02)) //更新百分比
					{
						res = 0XFF; //强制退出
						break;
					}
				}
				if (res || bw < br)
					break;
			}
			f_close(fsrc);
			f_close(fdst);
		}
	}
	free(fsrc); //释放内存
	free(fdst);
	free(fbuf);
	return res;
}

//得到路径下的文件夹
//返回值:0,路径就是个卷标号.
//    其他,文件夹名字首地址
uint8_t *exf_get_src_dname(uint8_t *dpfn)
{
	uint16_t temp = 0;
	while (*dpfn != 0)
	{
		dpfn++;
		temp++;
	}
	if (temp < 4)
		return 0;
	while ((*dpfn != 0x5c) && (*dpfn != 0x2f))
		dpfn--; //追述到倒数第一个"\"或者"/"处
	return ++dpfn;
}
//得到文件夹大小
//注意文件夹大小不要超过4GB.
//返回值:0,文件夹大小为0,或者读取过程中发生了错误.
//    其他,文件夹大小.
uint32_t exf_fdsize(uint8_t *fdname)
{
#define MAX_PATHNAME_DEPTH 512 + 1 //最大目标文件路径+文件名深度
	uint8_t res = 0;
	DIR *fddir = 0;		//目录
	FILINFO *finfo = 0; //文件信息
	uint8_t *fn = 0;			//长文件名
	uint8_t *pathname = 0;   //目标文件夹路径+文件名
	uint16_t pathlen = 0;	//目标路径长度
	uint32_t fdsize = 0;

	fddir = (DIR *)malloc(sizeof(DIR)); //申请内存
	finfo = (FILINFO *)malloc(sizeof(FILINFO));
	if (fddir == NULL || finfo == NULL)
		res = 100;
	if (res == 0)
	{
		finfo->lfsize = _MAX_LFN * 2 + 1;
		finfo->lfname = malloc(finfo->lfsize); //申请内存
		pathname = malloc(MAX_PATHNAME_DEPTH);
		if (finfo->lfname == NULL || pathname == NULL)
			res = 101;
		if (res == 0)
		{
			pathname[0] = 0;
			strcat((char *)pathname, (const char *)fdname); //复制路径
			res = f_opendir(fddir, (const TCHAR *)fdname);  //打开源目录
			if (res == 0)									//打开目录成功
			{
				while (res == 0) //开始复制文件夹里面的东东
				{
					res = f_readdir(fddir, finfo); //读取目录下的一个文件
					if (res != FR_OK || finfo->fname[0] == 0)
						break; //错误了/到末尾了,退出
					if (finfo->fname[0] == '.')
						continue;			   //忽略上级目录
					if (finfo->fattrib & 0X10) //是子目录(文件属性,0X20,归档文件;0X10,子目录;)
					{
						fn = (uint8_t *)(*finfo->lfname ? finfo->lfname : finfo->fname); //得到文件名
						pathlen = strlen((const char *)pathname);					//得到当前路径的长度
						strcat((char *)pathname, (const char *)"/");				//加斜杠
						strcat((char *)pathname, (const char *)fn);					//源路径加上子目录名字
																					//printf("\r\nsub folder:%s\r\n",pathname);	//打印子目录名
						fdsize += exf_fdsize(pathname);								//得到子目录大小
						pathname[pathlen] = 0;										//加入结束符
					}
					else
						fdsize += finfo->fsize; //非目录,直接加上文件的大小
				}
			}
			free(pathname);
			free(finfo->lfname);
		}
	}
	free(fddir);
	free(finfo);
	if (res)
		return 0;
	else
		return fdsize;
}
//文件夹复制
//注意文件夹大小不要超过4GB.
//将psrc文件夹,copy到pdst文件夹.
//pdst:必须形如"X:"/"X:XX"/"X:XX/XX"之类的.而且要实现确认上一级文件夹存在
//fcpymsg,函数指针,用于实现拷贝时的信息显示
//        pname:文件/文件夹名
//        pct:百分比
//        mode:
//			[0]:更新文件名
//			[1]:更新百分比pct
//			[2]:更新文件夹
//			[3~7]:保留
//psrc,pdst:源文件夹和目标文件夹
//totsize:总大小(当totsize为0的时候,表示仅仅为单个文件拷贝)
//cpdsize:已复制了的大小.
//fwmode:文件写入模式
//0:不覆盖原有的文件
//1:覆盖原有的文件
//返回值:0,成功
//    其他,错误代码;0XFF,强制退出
uint8_t exf_fdcopy(uint8_t (*fcpymsg)(uint8_t *pname, uint8_t pct, uint8_t mode), uint8_t *psrc, uint8_t *pdst, uint32_t *totsize, uint32_t *cpdsize, uint8_t fwmode)
{
#define MAX_PATHNAME_DEPTH 512 + 1 //最大目标文件路径+文件名深度
	uint8_t res = 0;
	DIR *srcdir = 0;	//源目录
	DIR *dstdir = 0;	//源目录
	FILINFO *finfo = 0; //文件信息
	uint8_t *fn = 0;			//长文件名

	uint8_t *dstpathname = 0; //目标文件夹路径+文件名
	uint8_t *srcpathname = 0; //源文件夹路径+文件名

	uint16_t dstpathlen = 0; //目标路径长度
	uint16_t srcpathlen = 0; //源路径长度

	srcdir = (DIR *)malloc(sizeof(DIR)); //申请内存
	dstdir = (DIR *)malloc(sizeof(DIR));
	finfo = (FILINFO *)malloc(sizeof(FILINFO));

	if (srcdir == NULL || dstdir == NULL || finfo == NULL)
		res = 100;
	if (res == 0)
	{
		finfo->lfsize = _MAX_LFN * 2 + 1;
		finfo->lfname = malloc(finfo->lfsize); //申请内存
		dstpathname = malloc(MAX_PATHNAME_DEPTH);
		srcpathname = malloc(MAX_PATHNAME_DEPTH);
		if (finfo->lfname == NULL || dstpathname == NULL || srcpathname == NULL)
			res = 101;
		if (res == 0)
		{
			dstpathname[0] = 0;
			srcpathname[0] = 0;
			strcat((char *)srcpathname, (const char *)psrc); //复制原始源文件路径
			strcat((char *)dstpathname, (const char *)pdst); //复制原始目标文件路径
			res = f_opendir(srcdir, (const TCHAR *)psrc);	//打开源目录
			if (res == 0)									 //打开目录成功
			{
				strcat((char *)dstpathname, (const char *)"/"); //加入斜杠
				fn = exf_get_src_dname(psrc);
				if (fn == 0) //卷标拷贝
				{
					dstpathlen = strlen((const char *)dstpathname);
					dstpathname[dstpathlen] = psrc[0]; //记录卷标
					dstpathname[dstpathlen + 1] = 0;   //结束符
				}
				else
					strcat((char *)dstpathname, (const char *)fn); //加文件名
				fcpymsg(fn, 0, 0X04);							   //更新文件夹名
				res = f_mkdir((const TCHAR *)dstpathname);		   //如果文件夹已经存在,就不创建.如果不存在就创建新的文件夹.
				if (res == FR_EXIST)
					res = 0;
				while (res == 0) //开始复制文件夹里面的东东
				{
					res = f_readdir(srcdir, finfo); //读取目录下的一个文件
					if (res != FR_OK || finfo->fname[0] == 0)
						break; //错误了/到末尾了,退出
					if (finfo->fname[0] == '.')
						continue;												//忽略上级目录
					fn = (uint8_t *)(*finfo->lfname ? finfo->lfname : finfo->fname); //得到文件名
					dstpathlen = strlen((const char *)dstpathname);				//得到当前目标路径的长度
					srcpathlen = strlen((const char *)srcpathname);				//得到源路径长度

					strcat((char *)srcpathname, (const char *)"/"); //源路径加斜杠
					if (finfo->fattrib & 0X10)						//是子目录(文件属性,0X20,归档文件;0X10,子目录;)
					{
						strcat((char *)srcpathname, (const char *)fn);								   //源路径加上子目录名字
						res = exf_fdcopy(fcpymsg, srcpathname, dstpathname, totsize, cpdsize, fwmode); //拷贝文件夹
					}
					else //非目录
					{
						strcat((char *)dstpathname, (const char *)"/");								   //目标路径加斜杠
						strcat((char *)dstpathname, (const char *)fn);								   //目标路径加文件名
						strcat((char *)srcpathname, (const char *)fn);								   //源路径加文件名
						fcpymsg(fn, 0, 0X01);														   //更新文件名
						res = exf_copy(fcpymsg, srcpathname, dstpathname, *totsize, *cpdsize, fwmode); //复制文件
						*cpdsize += finfo->fsize;													   //增加一个文件大小
					}
					srcpathname[srcpathlen] = 0; //加入结束符
					dstpathname[dstpathlen] = 0; //加入结束符
				}
			}
			free(dstpathname);
			free(srcpathname);
			free(finfo->lfname);
		}
	}
	free(srcdir);
	free(dstdir);
	free(finfo);
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
