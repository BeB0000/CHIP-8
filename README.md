# CHIP-8 Emulator

A simple CHIP-8 emulator written in C using SDL2.  
The name **TSHIP-8** is just a joke; this is a CHIP-8 emulator.

## Requirements

- GCC or a compatible C compiler
- Make
- SDL2 development libraries

Install SDL2:

### Debian / Ubuntu
```sh
sudo apt install build-essential libsdl2-dev
```

### Arch Linux
```sh
sudo pacman -S base-devel sdl2
```

### Fedora
```sh
sudo dnf install gcc make SDL2-devel
```

### macOS
```sh
brew install sdl2
```

### Windows
Use MSYS2 / MinGW and install the SDL2 development package.

## Build

To build the emulator binary:

```sh
make all
```

This creates:

```sh
build/emu
```

## Clean

To remove the build directory and compiled files:

```sh
make clean
```

## Run

```sh
./build/emu path/to/rom.ch8
```

Example:

```sh
./build/emu roms/Pong.ch8
```

## Controls

The CHIP-8 hex keypad is mapped to QWERTY like this:

```text
CHIP-8 keypad      Keyboard
1 2 3 C            1 2 3 4
4 5 6 D            Q W E R
7 8 9 E            A S D F
A 0 B F            Z X C V
```

## ROMs

You can find CHIP-8 ROMs here:

- https://github.com/kripod/chip8-roms
- https://github.com/JohnEarnest/chip8Archive
- https://www.zophar.net/chip8
- https://chip-8.github.io/links/

Please only use ROMs you have the right to use, such as homebrew, public domain, or freely redistributable ROMs.

## Notes

- Screen size: 64x32
- Window size: 640x320
- Pixel scale: 10x
- Target speed: 60 FPS
- Cycles per frame: 10
- Timers tick at 60 Hz

## LICENSE

- This project is under MIT license.
