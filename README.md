# Space Shooter
The asteroid belt that surrounds our solar system has partially collapsed and a bunch of asteroids are heading Earth's way! You know what to do (in case you don't, destroy them all), you're our only hope!

## Preview
<img src="https://github.com/lucianoinso/spaceshooter/blob/master/Preview/spaceshooter-preview.gif" width="400" height="200"/>

## Installation (Windows)
Just run `setup.exe`

## Setup and compilation (Ubuntu)
Compiling it yourself has some benefits, you can increase/decrease the difficulty by changing the `NUM_BULLETS`, `NUM_COMETS` and `SCORE_NEEDED_TO_WON` constants at main.cpp, also you can tweak the resolution (at your own risk).

First, install the Allegro5 library:

```console
sudo apt update
sudo apt install liballegro5-dev
```

### Compile

Run `make` inside the `Source` directory

In case you want to compile it manually, you can by running:
```console
gcc main.cpp -Wall -Wextra `pkg-config --cflags --libs allegro-5 allegro_acodec-5 allegro_audio-5 allegro_color-5 allegro_dialog-5 allegro_font-5 allegro_image-5 allegro_main-5 allegro_memfile-5 allegro_physfs-5 allegro_primitives-5 allegro_ttf-5` -o main
```

### Run
run `./main` inside the `Source` directory, after compiling it.
