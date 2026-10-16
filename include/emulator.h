// -------------------
// Copyright @BeB0000
// UNDER MIT LICENSE
// include/emulator.h
// --------------------

// libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <time.h> 

#include <SDL2/SDL.h>

// config stuff tweak these if you fancy (idts)
#define MemorySize 4096
#define StackSize 16
#define NumRegisters 16
#define ScreenWidth 64
#define ScreenHeight 32
#define PixelScale 10   // window ends up 640x320  big enough
#define Fps 60
#define CyclesPerFrame 10 // run 10 opcodes per frame gives us a groovy ~600hz
