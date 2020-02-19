#include "chip8.hpp"
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include "OpcodeException.hpp"

uint8_t chip8_fontset[80] =
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8() {
    this->pc = 0x200;
    this->opcode = 0;
    this->I = 0;
    this->sp = -1;
    this->playSound = NULL;
    this->sound = NULL;

    std::fill_n(memory, 4096, 0); // Clear memory
    std::fill_n(V, 16, 0); // Clear registers
    std::fill_n(stack, 16, 0); // Clear stack
    std::fill_n(pixels, 64 * 32, 0); // Clear display
    std::fill_n(keys, 16, false); // Clear keys array

    // Load fontset
    //*memory = *(uint64_t*)chip8_fontset;
    //*(memory + 64) = *(uint16_t*)(chip8_fontset + 64);
    for (int i = 0; i < 80; i++) {
       memory[i] = chip8_fontset[i];
    }

    isRunning = true;
    waitForKey = false;

}

void Chip8::emulateCycle(void) {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    if (!decodeOpCode(opcode)) {
        throw OpCodeException(opcode, (uint8_t) memory + pc);
    }
    else {
        if (delayTimer > 0) {
            delayTimer--;
        }
        if (soundTimer > 0) {
            if (soundTimer == 1) {
                if (playSound) {
                    // If a sound implementation was provided
                    playSound(sound);
                }
            }
            soundTimer--;
        }
        // If the execution is not halted
        if (isRunning) {
            // Increase program counter
            pc += 2;
        }
    }
}

void Chip8::keyPress(const uint8_t key) {
    if (waitForKey) {
        waitForKey = false;
        isRunning = true;

        // Stores pressed key is VX cause of opcodes 0xEX9E and 0xEXA1
        V[opcode >> 8 & 0x0F] = key;
    }
    keys[key] = true;
}

void Chip8::keyRelease(const uint8_t key) {
    keys[key] = false;
}

uint8_t (&Chip8::getPixels(void))[RES]
{
    // Return a reference of the array
    return pixels;
}

void Chip8::loadROM(const char* fName) {
    std::ifstream rom;
    rom.open(fName, std::ios::binary);

    if (rom.fail()) {
        throw std::runtime_error("Error: file");
    }
    
    rom.seekg(0, rom.end);
    uint8_t length = rom.tellg();
    rom.seekg(0, rom.beg);

    rom.read((char*)(memory + 0x200), length);

    rom.close();
}

void Chip8::setSound(void (*func)(void *sound), void *sound) {
    // Make sound a void pointer so the emulator implementation
    // is not library dependend
    this->playSound = func;
    this->sound = sound;
}

bool Chip8::decodeOpCode(const uint16_t opcode) {
    switch (opcode & 0xF000) {
        case 0x0: {
            switch (opcode & 0x0FF) {
                case 0x0E0: { // Clear screan
                    std::fill_n(pixels, 64 * 32, 0);
                    pc += 2;
                    break;
                }
                case 0x0EE: { // Return from subroutine
                    pc = stack[sp--];
                    break;
                }
                default: {
                    return false;
                }
            }
            break;

        }
        case 0x1000: { // Jump to address NNN
            pc = opcode & 0x0FFF;
            break;
        }
        case 0x2000: { // Call subroutine
            stack[++sp] = pc;
            pc = opcode & 0x0FFF;
            break;
        }
        case 0x3000: {
            // Skips the next instruction if VX equals NN
            if (V[opcode >> 8 & 0x0F] == opcode & 0x0FF) {
                pc += 2;
            }
            pc += 2;
            break;
        }
        case 0x4000: {
            // Skips the next instruction if VX doesn't equal NN
            if (V[opcode >> 8 & 0x0F] != opcode & 0x0FF) {
                pc += 2;
            }
            pc += 2;
            break;
        }
        case 0x5000: {
            // Skips the next instruction if VX equals VY
            if (V[opcode >> 8 & 0x0F] == V[opcode >> 4 & 0x0F]) {
                pc += 2;
            }
            pc += 2;
            break;
        }
        case 0x6000: {
            // Sets VX to NN
            V[opcode >> 8 & 0x0F] = opcode & 0x0FF;
            pc += 2;
            break;
        }
        case 0x7000: {
            // Adds NN to VX
            V[opcode >> 8 & 0x0F] += opcode & 0x0FF;
            pc += 2;
            break;
        }
        case 0x8000: {
            switch (opcode & 0x0F) {
                case 0x00: {
                    // Sets VX to the value of VY
                    V[opcode >> 8 & 0xF] = V[opcode >> 4 & 0xF];
                    pc += 2;
                    break;
                }
                case 0x01: {
                    // Sets VX to VX or VY
                    V[opcode >> 8 & 0xF] |= V[opcode >> 4 & 0xF];
                    pc += 2;
                    break;
                }
                case 0x02: {
                    // Sets VX to VX and VY
                    V[opcode >> 8 & 0xF] &= V[opcode >> 4 & 0xF];
                    pc += 2;
                    break;
                }
                case 0x03: {
                    // Sets VX to VX xor VY
                    V[opcode >> 8 & 0xF] ^= V[opcode >> 4 & 0xF];
                    pc += 2;
                    break;
                }
                case 0x04: { // TODO Check
                    // Adds VY to VX
                    // VF is set to 1 when there's a carry, and to 0 when there isn't
                    V[0x0F] = (V[opcode >> 8 & 0xF] + V[opcode >> 4 & 0xF] > 0xFFF) ? 1 : 0;
                    V[opcode >> 8 & 0xF] += V[opcode >> 4 & 0xF];
                    pc += 2;
                    break;
                }
                case 0x05: { //TODO Check
                    // VY is subtracted from VX
                    // VF is set to 0 when there's a borrow, and 1 when there isn't
                    V[0x0F] = (V[opcode >> 8 & 0xF] > V[opcode >> 4 & 0xF]) ? 1 : 0;
                    V[opcode >> 8 & 0xF] -= V[opcode >> 4 & 0xF];
                    pc += 2;
                    break;
                }
                case 0x06: {
                    // Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
                    V[0xF] = V[opcode >> 8 & 0x0F] & 0x01;
                    V[opcode >> 8 & 0x0F] >>= 1;
                    pc += 2;
                    break;
                }
                case 0x07: { // TODO Check
                    // Sets VX to VY minus VX
                    // VF is set to 0 when there's a borrow, and 1 when there isn't
                    V[0x0F] = (V[opcode >> 4 & 0x0F] > V[opcode >> 8 & 0x0F] ? 0 : 1);
                    V[opcode >> 8 & 0x0F] = V[opcode >> 4 & 0xF] - V[opcode >> 8 & 0x0F];
                    pc += 2;
                    break;

                }
                case 0x0E: {
                    // 	Stores the most significant bit of VX in VF
                    // and then shifts VX to the left by 1
                    V[0x0F] = V[opcode >> 8 & 0x0F] >> 15 & 0x01;
                    V[opcode >> 8 & 0x0F] <<= 1;
                    pc += 2;
                    break;
                }
                default: {
                    return false;
                }      
            }
            break;
        }
        case 0x9000: {
            // Skips the next instruction if VX doesn't equal VY
            if (V[opcode >> 8 & 0x0F] != V[opcode >> 4 & 0x0F]) {
                pc += 2;
            }
            pc += 2;
            break;
        }
        case 0xA000: {
            // Sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        }
        case 0xB000: {
            // Jumps to the address NNN plus V0
            pc = (opcode & 0x0FFF) + V[0x0];
            break;
        }
        case 0xC000: {
            // Sets VX to the result of a bitwise and operation on a random number
            // (Typically: 0 to 255) and NN
            V[opcode >> 8 & 0x0F] = (std::rand() % 0xFF) & (opcode & 0x0FF);
            pc += 2;
        }
        case 0xD000: {
            // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a
            // height of N pixels. Each row of 8 pixels is read as bit-coded starting from memory
            // location I; I value doesn’t change after the execution of this instruction. As described
            // above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is
            // drawn, and to 0 if that doesn’t happen
            uint8_t vx = V[opcode >> 8 & 0x0F];
            uint8_t vy = V[opcode >> 4 & 0x0F];
            uint16_t height = opcode & 0x0F;
            uint16_t pixel;

            V[0x0F] = 0;
            for (int yline = 0; yline < height; yline++) {
                pixel = memory[I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if ((pixel & (0x80 >> xline)) != 0) {
                        if (pixels[vx + xline + ((vy + yline) * 64)] == 1) {
                            V[0x0F] = 1;
                        }
                        pixels[vx + xline + ((vy + yline) * 64)] ^= 1;
                    }
                }
            }
            break;
        }
        case 0xE000: {
            switch (opcode & 0x0FF) {
                case 0x09E: { //TODO Check
                    // Skips the next instruction if the keys stored in VX is pressed
                    if (keys[V[opcode >> 8 & 0x0F]]) {
                        pc += 2;
                    }
                    pc += 2;
                    break;
                }
                case 0xA1: { // TODO check
                    // Skips the next instruction if the keys stored in VX isn't pressed
                    if (!keys[V[opcode >> 8 & 0x0F]]) {
                        pc += 2;
                    }
                    pc += 2;
                    break;
                }
                default: {
                    return false;
                }
            }
            break;
        }
        case 0xF000: {
            switch (opcode & 0xFF) {
                case 0x07: {
                    // Sets VX to the value of the delay timer.
                    V[opcode >> 8 & 0x0F] = delayTimer;
                    pc += 2;
                    break;
                }
                case 0x0A: { // TODO check
                    // A keys press is awaited, and then stored in VX
                    waitForKey = true;
                    isRunning = false;
                    break;
                }
                case 0x015: {
                    // Sets the delay timer to VX
                    delayTimer = V[opcode >> 8 & 0x0F];
                    pc += 2;
                    break;
                }
                case 0x018: {
                    // Sets the sound timer to VX
                    soundTimer = V[opcode >> 8 & 0x0F];
                    pc += 2;
                    break;
                }
                case 0x01E: {
                    // Adds VX to I
                    // VF is set to 1 when there is a range overflow
                    // and to 0 when there isn't
                    V[0xF] = (I + (opcode >> 8 & 0x0F) > 0xFFF ? 1 : 0);
                    I = opcode >> 8 & 0x0F;
                    pc += 2;
                    break;
                }
                case 0x029: {
                    // Sets I to the location of the sprite for the character in VX.
                    // Characters 0-F (in hexadecimal) are represented by a 4x5 font
                    I = V[opcode >> 8 & 0x0F] * 5; // Each row has 5 elements
                    break;
                }
                case 0x033: {
                    // Stores the binary-coded decimal representation of VX, with the most significant
                    // of three digits at the address in I, the middle digit at I plus 1, and the least
                    // significant digit at I plus 2. (In other words, take the decimal representation of
                    // VX, place the hundreds digit in memory at location in I, the tens digit at location
                    // I+1, and the ones digit at location I+2.)
                    memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
                    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                    pc += 2;
                    break;
                }
                case 0x055: {
                    // Stores V0 to VX (including VX) in memory starting at address I.
                    // The offset from I is increased by 1 for each value written, but I itself is left unmodified
                    for (int i = 0; i <= (opcode >> 8 & 0x0F); i++) {
                        memory[I + i] = V[i];
                    }
                    pc += 2;
                    break;
                }
                case 0x65: {
                    for (int i = 0; i <= (opcode >> 8 & 0x0F); i++) {
                        // Fills V0 to VX (including VX) with values from memory starting at address I.
                        // The offset from I is increased by 1 for each value written, but I itself is left unmodified.
                        V[i] = memory[I + i];
                    }
                    pc += 2;
                    break;
                }
            }
            break;
        }
        default: {
            return false;
        }

    }

    return true;
}