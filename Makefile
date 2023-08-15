CC	= ${GBDKDIR}bin/lcc

all: nspc_song_data.h
	$(CC)  -Wm-ys -o sgb_mus.gb main.c sgb_sfx_names.c sgb_music.c

# workaround/hack: use sdasgb to compile music data (GB and SPC are both little endian)
# then use a python script to extract the relevant data into a uint8_t C array.
nspc_song_data.h: nspc_music.s extract_nspc.py
	$(CC) -o tmp.gb nspc_music.s
	python -m extract_nspc tmp.gb && rm tmp.gb


compile.bat: Makefile
	@echo "REM Automatically generated from Makefile" > compile.bat
	@make -sn | sed y/\\//\\\\/ | sed s/mkdir\ -p\/mkdir\/ | grep -v make >> compile.bat

clean:
	rm -f *.o *.lst *.gb *.ihx *.sym *.cdb *.adb *.asm
