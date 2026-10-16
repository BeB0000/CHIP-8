// --------------------
// Copyright @BeB0000
// UNDER MIT LICENSE
// emulator.c
// --------------------

#include "../include/emulator.h"

// the brain
uint8_t Memory[MemorySize];
uint8_t VRegisters[NumRegisters];   // v0..vf and vf doubles as carry flag – multitasking
uint16_t IndexRegister;             // points to memory
uint16_t ProgramCounter;            // where we are in the code the cpu's gps
uint16_t Stack[StackSize];
uint8_t StackPointer;               // next free slot on the stack
uint8_t DelayTimer;
uint8_t SoundTimer;
uint8_t Screen[ScreenWidth * ScreenHeight]; // 1 = pixel lit up
uint8_t Keypad[16];                 // current key states 1
uint16_t Opcode;

// font sprites for 0..f each 5 bytes tall stored at the very beginning of memory
uint8_t FontSprites[80] =
{
	// lets joke bad :evel:
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0  looks like a zero whod have thought
	0x20, 0x60, 0x20, 0x20, 0x70, // 1  a skinny little fella
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2  its a swan no wait its a two
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3  like a pair of glasses
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4  someone left the door open
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5  looks like a squirrel but its a five
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6  a six-pack but only one can
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7  a slide wheee
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8  two circles having a hug
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9  a balloon with a string
	0xF0, 0x90, 0xF0, 0x90, 0x90, // a  like a fancy arch
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // b  a chunky dude
	0xF0, 0x80, 0x80, 0x80, 0xF0, // c  a half moon
	0xE0, 0x90, 0x90, 0x90, 0xE0, // d  a rounded square
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // e  a comb or a fence
	0xF0, 0x80, 0xF0, 0x80, 0x80  // f  a flag waving
	// no need to thank me
};

// sdl window and renderer
SDL_Window *Window = NULL;
SDL_Renderer *Renderer = NULL;

// set everything back to factory defaults
void InitChip8()
{
	// wipe all the memory and registers (clean slate bby) ill tell you something too "I USE ARCH BRT"
	memset(Memory, 0, MemorySize);
	memset(VRegisters, 0, NumRegisters);
	memset(Stack, 0, StackSize);
	memset(Screen, 0, ScreenWidth * ScreenHeight);
	memset(Keypad, 0, 16);
	IndexRegister = 0;
	ProgramCounter = 0x200;      // chip-8 programs start right here
	StackPointer = 0;
	DelayTimer = 0;
	SoundTimer = 0;
	Opcode = 0;

	// drop the font sprites into the first 80 bytes they live at 0x000 (= 0 G'HOOK)
	memcpy(&Memory[0], FontSprites, 80);
}

// load a rom file into memory if it fails ill cry a little not like how i cried about who broke my heart :egg:
int LoadRom(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if (!file)
	{
		printf("error: could not open rom '%s' (vro)\n", filename);
		return 0;
	}
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	rewind(file);

	if (size > MemorySize - 0x200)
	{
		printf("rom too large (max 3584 bytes) (BIG AHHH NOOOOOOOOOOOO)\n");
		fclose(file);
		return 0;
	}

	fread(&Memory[0x200], 1, size, file);
	fclose(file);
	return 1; // all good in the hood
}

// draw the screen using sdl turn those bits into glowing squares
void DrawScreen()
{
	SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
	SDL_RenderClear(Renderer); // paint it black

	SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255); // white as snow
	for (int y = 0; y < ScreenHeight; y++)
	{
		for (int x = 0; x < ScreenWidth; x++)
		{
			if (Screen[y * ScreenWidth + x])
			{
				SDL_Rect rect = { x * PixelScale, y * PixelScale, PixelScale, PixelScale };
				SDL_RenderFillRect(Renderer, &rect); // light up that pixel
			}
		}
	}
	SDL_RenderPresent(Renderer); // show the world what weve got
}

// check for key presses we map qwerty (the smothest word ive ever wrote) to the chip-8 hex keypad
void HandleKeys()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT) exit(0); // time to go home (just get out)

		if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
		{
			uint8_t pressed = (e.type == SDL_KEYDOWN);
			SDL_Keycode key = e.key.keysym.sym;

			// heres the secret decoder ring
      // 1 2 3 c  ->  1 2 3 4
      //  4 5 6 d  ->  q w e r
      //  7 8 9 e  ->  a s d f
      //  a 0 b f  ->  z x c v
			switch(key)
			{
				case SDLK_1: Keypad[0x1] = pressed; break;
				case SDLK_2: Keypad[0x2] = pressed; break;
				case SDLK_3: Keypad[0x3] = pressed; break;
				case SDLK_4: Keypad[0xC] = pressed; break;
				case SDLK_q: Keypad[0x4] = pressed; break;
				case SDLK_w: Keypad[0x5] = pressed; break;
				case SDLK_e: Keypad[0x6] = pressed; break;
				case SDLK_r: Keypad[0xD] = pressed; break;
				case SDLK_a: Keypad[0x7] = pressed; break;
				case SDLK_s: Keypad[0x8] = pressed; break;
				case SDLK_d: Keypad[0x9] = pressed; break;
				case SDLK_f: Keypad[0xE] = pressed; break;
				case SDLK_z: Keypad[0xA] = pressed; break;
				case SDLK_x: Keypad[0x0] = pressed; break;
				case SDLK_c: Keypad[0xB] = pressed; break;
				case SDLK_v: Keypad[0xF] = pressed; break;
				default: break; // ignore other keys theyre not invited
			}
		}
	}
}

// execute one instruction the heart of the emulator (i made a heart because mine broke :rizz: is it even rizzful can i just shut up)
void EmuCycle()
{
	// 1 fetch: grab two bytes from memory (big-endian (wdym big vro) because the universe likes it that way)
	Opcode = (Memory[ProgramCounter] << 8) | Memory[ProgramCounter + 1];
	ProgramCounter += 2; // move forward weve (no ' because i hate it) already read this one

	// 2 decode & execute
	uint16_t nnn = Opcode & 0x0FFF;
	uint8_t x = (Opcode & 0x0F00) >> 8;
	uint8_t y = (Opcode & 0x00F0) >> 4;
	uint8_t n = Opcode & 0x000F;
	uint8_t kk = Opcode & 0x00FF;

	switch (Opcode & 0xF000)
	{
		case 0x0000:
			switch (Opcode & 0x00FF)
			{
		    case 0x00E0: // (cls) clear the screen start fresh (ew windows shell GOD LORD)
					memset(Screen, 0, ScreenWidth * ScreenHeight);
					break;
				case 0x00EE: // (ret) pop the stack and go back to where we were called from
					StackPointer--;
					ProgramCounter = Stack[StackPointer];
					break;
				default:
					printf("unknown opcode: 0x%04X (aliens ahhhhh)\n", Opcode);
					break;
			}
			break;

		case 0x1000: // (jp) jump to a new address no return ticket
			ProgramCounter = nnn;
			break;

		case 0x2000: // (call) jump to subroutine remember where we came from
			Stack[StackPointer] = ProgramCounter;
			StackPointer++;
			ProgramCounter = nnn;
			break;

		case 0x3000: // (se) skip next instruction if vx equals kk
			if (VRegisters[x] == kk) ProgramCounter += 2;
			break;

		case 0x4000: // (sne) skip if vx is not equal to kk
			if (VRegisters[x] != kk) ProgramCounter += 2;
			break;

		case 0x5000: // (se) (double kill) skip if vx equals vy (register compare)
			if (VRegisters[x] == VRegisters[y]) ProgramCounter += 2;
			break;

		case 0x6000: // (ld) load a byte into vx
			VRegisters[x] = kk;
			break;

		case 0x7000: // (add) add a byte to vx (no carry its just addition)
			VRegisters[x] += kk;
			break;

		case 0x8000:
			switch (Opcode & 0x000F) // F stands for nothing cuz 0 * something is 0 and 0 is nothing :cool:
			{
				case 0x0: // (ld) copy vy into vx
					VRegisters[x] = VRegisters[y];
					break;
				case 0x1: // (or) bitwise or
					VRegisters[x] |= VRegisters[y];
					break;
				case 0x2: // (and) bitwise and
					VRegisters[x] &= VRegisters[y];
					break;
				case 0x3: // (xor) bitwise exclusive or
					VRegisters[x] ^= VRegisters[y];
					break;
				case 0x4: // (add) with carry – vf gets 1 if overflow
					if (VRegisters[x] + VRegisters[y] > 255) VRegisters[0xF] = 1;
					else VRegisters[0xF] = 0;
					VRegisters[x] += VRegisters[y];
					break;
				case 0x5: // (sub) vx = vx - vy and vf = 1 if no borrow
					if (VRegisters[x] > VRegisters[y]) VRegisters[0xF] = 1;
					else VRegisters[0xF] = 0;
					VRegisters[x] -= VRegisters[y];
					break;
				case 0x6: // (shr) shift vx right by one vf gets the bit that fell off
					VRegisters[0xF] = VRegisters[x] & 0x1;
					VRegisters[x] >>= 1;
					break;
				case 0x7: // (subn) vx = vy - vx and vf = 1 if no borrow
					if (VRegisters[y] > VRegisters[x]) VRegisters[0xF] = 1;
					else VRegisters[0xF] = 0;
					VRegisters[x] = VRegisters[y] - VRegisters[x];
					break;
				case 0xE: // (shl) shift vx left vf gets the most significant bit
					VRegisters[0xF] = (VRegisters[x] & 0x80) >> 7;
					VRegisters[x] <<= 1;
					break;
				default:
					printf("unknown opcode: 0x%04X (mysterious)\n", Opcode);
					break;
			}
			break;

		case 0x9000: // (sne) skip if vx != vy
			if (VRegisters[x] != VRegisters[y]) ProgramCounter += 2;
			break;

		case 0xA000: // (ld i) set index register to a 12-bit address
			IndexRegister = nnn;
			break;

		case 0xB000: // (jp v0) jump to nnn + v0 (indirect jump)
			ProgramCounter = nnn + VRegisters[0];
			break;

		case 0xC000: // (rnd) generate random number and mask it with kk
			VRegisters[x] = (rand() % 256) & kk;
			break;

		case 0xD000: // (drw) draw a sprite at (vx, vy) with height n
		{
			uint8_t x_pos = VRegisters[x] % ScreenWidth;
			uint8_t y_pos = VRegisters[y] % ScreenHeight;
			VRegisters[0xF] = 0; // clear collision flag before we start

			for (int row = 0; row < n; row++)
			{
				uint8_t sprite_row = Memory[IndexRegister + row];
				int screen_y = (y_pos + row) % ScreenHeight;
				for (int col = 0; col < 8; col++)
				{
					uint8_t pixel = (sprite_row >> (7 - col)) & 1;
					int screen_x = (x_pos + col) % ScreenWidth;
					int index = screen_y * ScreenWidth + screen_x;
					if (pixel)
					{
						if (Screen[index] == 1) VRegisters[0xF] = 1; // collision bump into something
						Screen[index] ^= 1; // xor drawing toggles the pixel
					}
				}
			}
			break;
		}

		case 0xE000:
			switch (Opcode & 0x00FF)
			{
				case 0x009E: // (skp) skip next instruction if the key in vx is pressed
					if (Keypad[VRegisters[x]]) ProgramCounter += 2;
					break;
				case 0x00A1: // (sknp) skip if the key is NOT pressed
					if (!Keypad[VRegisters[x]]) ProgramCounter += 2;
					break;
				default:
					printf("unknown opcode: 0x%04X\n", Opcode);
					break;
			}
			break;

		case 0xF000:
			switch (Opcode & 0x00FF)
			{
				case 0x0007: // ld vx dt get the delay timer value
					VRegisters[x] = DelayTimer;
					break;
				case 0x000A: // ld vx k wait for a key press then store it
				{
					uint8_t key_pressed = 0xFF;
					for (int i = 0; i < 16; i++)
					{
						if (Keypad[i])
						{
							key_pressed = i;
							break;
						}
					}
					if (key_pressed != 0xFF)
					{
						VRegisters[x] = key_pressed; // got one
					}
					else
					{
						ProgramCounter -= 2; // no key yet we'll try again next cycle
					}
					break;
				}
				case 0x0015: // ld dt vx set the delay timer from vx
					DelayTimer = VRegisters[x];
					break;
				case 0x0018: // ld st vx set the sound timer from vx
					SoundTimer = VRegisters[x];
					break;
				case 0x001E: // add i vx – add vx to index register (no carry it's just addition)
					IndexRegister += VRegisters[x];
					break;
				case 0x0029: // ld f vx set index to the font sprite for digit vx
					IndexRegister = VRegisters[x] * 5; // each font is 5 bytes starting at 0
					break;
				case 0x0033: // ld b vx store binary-coded decimal of vx at [i] [i+1] [i+2]
					Memory[IndexRegister] = VRegisters[x] / 100;
					Memory[IndexRegister + 1] = (VRegisters[x] / 10) % 10;
					Memory[IndexRegister + 2] = VRegisters[x] % 10;
					break;
				case 0x0055: // ld [i] vx store registers v0..vx into memory starting at i
					for (int i = 0; i <= x; i++)
					{
						Memory[IndexRegister + i] = VRegisters[i];
					}
					break;
				case 0x0065: // ld vx [i] load registers v0..vx from memory starting at i
					for (int i = 0; i <= x; i++)
					{
						VRegisters[i] = Memory[IndexRegister + i];
					}
					break;
				default:
					printf("unknown opcode: 0x%04X\n", Opcode);
					break;
			}
			break;

		default:
			printf("unknown opcode: 0x%04X\n", Opcode);
			break;
	}




	// timers are decremented elsewhere (at 60hz)
}

// the main event let's get this party started bby
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("usage: %s <rom.ch8>\nbro where the hell is the rom vro\n", argv[0]);
		return 1;
	}

	// fire up sdl if it fails we are in trouble
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("sdl init failed: %s\nmaybe you need to install sdl2\n", SDL_GetError());
		return 1;
	}

	Window = SDL_CreateWindow("TSHIP-8 :D", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								ScreenWidth * PixelScale, ScreenHeight * PixelScale, SDL_WINDOW_SHOWN);
	if (!Window)
	{
		printf("window creation failed: %s\nwindows are hard okay\n", SDL_GetError());
		return 1;
	}
	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED);
	if (!Renderer)
	{
		printf("renderer creation failed: %s\nrendering is overrated anyway\n", SDL_GetError());
		return 1;
	}

	srand(time(NULL)); // seed the random number generator for those rnd opcodes
	InitChip8();

	if (!LoadRom(argv[1]))
	{
		return 1; // rom loading failed we're done
	}

	// main loop – infinite like the universe (or until the user quits)
	while (67) // :> Skibidi Ohio Rizz Gyatt
	{
		HandleKeys(); // check for key presses

		// run a few cycles per frame to keep the speed reasonable
		for (int i = 0; i < CyclesPerFrame; i++)
		{
			EmuCycle();
		}

		// tick the timers down at 60hz
		if (DelayTimer > 0) DelayTimer--;
		if (SoundTimer > 0) SoundTimer--;

		DrawScreen(); // show the updated screen
		SDL_Delay(1000 / Fps); // wait about 16ms per frame
	}

	SDL_DestroyRenderer(Renderer);
	SDL_DestroyWindow(Window);
	SDL_Quit();
	return 0;
}



// HOLY MOLY BOLY i finished but wth is boly
// i am SIGMA anyway
// sorry i cringed you alot (my bad jokes :D)
