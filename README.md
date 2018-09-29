Install Allegro5 en Ubuntu
============================

sudo apt update
sudo apt-get install liballegro5-dev

Compilar
========
A Makefile is attached, but in case you want to compile it manually, you can, by running:
gcc main.cpp -Wall -Wextra \`pkg-config --cflags --libs allegro-5 allegro\_acodec-5 allegro\_audio-5 allegro\_color-5 allegro\_dialog-5 allegro\_font-5 allegro\_image-5 allegro\_main-5 allegro\_memfile-5 allegro\_physfs-5 allegro\_primitives-5 allegro\_ttf-5\` -o main
