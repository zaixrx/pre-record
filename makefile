CC = gcc
FLAGS = -Wall -g -I/usr/include -L/usr/lib -Wl,-rpath,/usr/lib -lavcodec -lavutil -lswscale
FILES = main.c webcam.c rle.c types.c encoder.c
EXE = ./main

TESTS = ./test/compression.c
RESOLUTION = 640x480

c:
	$(CC) $(FILES) $(FLAGS) -o $(EXE) && $(EXE)

all:
	$(CC) $(FILES) $(FLAGS) -o $(EXE) && $(EXE) && ffmpeg -r 10 -i video.h264 -c:v copy video.mp4 && mpv video.mp4
