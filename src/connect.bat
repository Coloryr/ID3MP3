D:\openocd\bin\openocd.exe -c "tcl_port disabled" -c "gdb_port 3333" -c "telnet_port 4444" -s D:\openocd\share\openocd\scripts -f G:\code\ID3MP3\src\debug.cfg -c "program D:/code/ID3MP3/src/cmake-build-debug/ID3MP3.elf" -c "init;reset init;" -c "echo (((READY)))"