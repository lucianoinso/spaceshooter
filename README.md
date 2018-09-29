Install Allegro5 on Ubuntu
============================

sudo apt update

sudo apt install liballegro5-dev

Compile
========
A Makefile is attached, but in case you want to compile it manually, you can by running:
```console
gcc main.cpp -Wall -Wextra `pkg-config --cflags --libs allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5` -o main
```
