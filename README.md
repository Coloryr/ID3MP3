# 让Lvgl支持从Flash（W25Qxx）读取字库

使用说明，工程是Clion工程，其他编译器需要自己转换。

## 硬件连接
- W25Q64：  
  挂载在`Qspi`  
  CS管脚`PB6`
- SD卡：  
  挂载在`SDIO`
- TFT屏幕：  
  挂载在`FMC`  
  CS管脚`A19`

## 字库制作
[字库制作](https://docs.lvgl.io/master/overview/font.html#add-a-new-font)

下载那个`Offline font converter.`

制作的指令
```
$ npx lv_font_conv --font .\fangzheng.ttf -r 0x00-0xFFFF --size 16 --format bin --no-compress --bpp 4 -o fz16_no.font
```

- `--font`：字体
- `--size`：字体大小
- `--format`：输出格式
- `--bpp`：bits per pixel (antialiasing) ~~不懂有啥用~~
- `--format`：输出格式，必须是`bin`
- `--no-compress`：不使用压缩，必须加\
- `-o`：输出文件名

运行结束后会生成一个文件

## 字库烧写
在SD卡上新建一个`font`文件夹

复制16,24,32大小的bin文件，并命名为`16.font`，`24.font`，`32.font`

装上SD卡就能自动烧写了