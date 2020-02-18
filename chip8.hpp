#pragma once
#include <cstdint>
class Chip8 {
private:
	uint16_t opcode;
	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t I; // Index register
	uint16_t pc;
	bool waitForKey;
	bool isRunning;
	void *sound;
	void (*playSound)(void *sound);

	uint8_t pixels[64 * 32];
	uint8_t keys[16];

public:
	Chip8();

	uint16_t stack[16];
	int8_t sp;
	uint8_t delayTimer;
	uint8_t soundTimer;

	bool decodeOpCode(const uint16_t opcode);
	void keyPress(const uint8_t key);
	void keyRelease(const uint8_t key);
	void loadROM(const char* fName);
	uint8_t* getPixels(void);
	void emulateCycle(void);
	void setSound(void (*func)(void *sound), void *sound);

};