set gbdk=C:\Misc\GBDK2020

%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -o build/main.o main.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo9 -o build/bgtiles.o bgtiles.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo9 -o build/wintiles.o wintiles.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo9 -o build/winmap.o winmap.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo9 -o build/bgmap.o bgmap.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo9 -o build/menu.o menu.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo9 -o build/sprites.o sprites.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -o build/is_word.o is_word.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo1 -o build/dict_0.o dict/dict_0.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo2 -o build/dict_1.o dict/dict_1.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo3 -o build/dict_2.o dict/dict_2.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo4 -o build/dict_3.o dict/dict_3.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo5 -o build/dict_4.o dict/dict_4.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo6 -o build/dict_5.o dict/dict_5.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo7 -o build/dict_6.o dict/dict_6.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo8 -o build/dict_7.o dict/dict_7.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo10 -o build/dict_8.o dict/dict_8.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo11 -o build/dict_9.o dict/dict_9.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo12 -o build/dict_10.o dict/dict_10.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -c -Wf-bo13 -o build/dict_11.o dict/dict_11.c
%gbdk%/bin/lcc -Wa-l -Wl-m -Wl-j -Wm-yc -Wl-yt0x2 -Wl-yoA -Wl-ya4 -Wm-yn"WORDBLOCKS" -o build/test.gb build/*.o

DEL build\*.o
DEL build\*.lst
DEL build\*.sym
DEL build\*.ihx
DEL build\*.map
DEL build\*.noi
DEL build\*.asm
DEL build\*.cdb
DEL build\*.s19