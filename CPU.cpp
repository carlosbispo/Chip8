#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>  
#include "CPU.h"

    unsigned char chip8_fontset[80] =
        { 
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

    void CPU::init() {
        pc     = 0x200;  // Program counter starts at 0x200
        opcode = 0;      // Reset current opcode	
        I = 0;      // Reset index register
        sp = 0;      // Reset stack pointer
 
        // Clear display	
        // Clear stack
        for(int i = 0;i < 16;i++) {
            stack[i] = 0;
        }

        // Clear registers V0-VF
        for(int i = 0;i < 16;i++) {
            V[i] = 0;
        }
        // Clear memory
        for(int i = 0;i < 4096;i++) {
            memory[i] = 0;
        }

        // Clear input
        for(int i = 0;i < 16;i++) {
            key[i] = 0;
        }
 
        // Load fontset
        for(int i = 0; i < 80; ++i) {
            memory[i] = chip8_fontset[i];	
        }

        delay_timer = 0;
        sound_timer = 0;

        clearScreen();

        /* initialize random seed: */
        srand (time(NULL));
    }

    void CPU::loadProgram(char* program, int size) {
        for(int i = 0;i < size;i++) {
            memory[i + 512] = program[i];
        }
    }

    void CPU::emulateCycle()  {
        opcode = memory[pc] << 8 | memory[pc + 1];
        // Decode opcode
        switch(opcode & 0xF000)
        {    
             case 0x0000:
                 switch(opcode & 0x000F) {
                    case 0x0000: // 00E0 clear screen
                        printf("Clear screen\n");
                        clearScreen();
                        pc += 2;
                        drawFlag = true;
                    break;
                    case 0x000E: //00EE Returns from a subroutine.
                        printf("Return from subroutine\n");
                        --sp;
                    	pc = stack[sp];
                    	pc += 2;
                    break;
                    default :  {printf ("Unknown opcode: 0x%X\n", opcode); return;}
                 }
             break;
             case 0x1000: // 1NNN Jumps to address NNN.
                printf("Jump to address  0x%X\n", opcode & 0x0FFF);
                pc = opcode & 0x0FFF;
             break;
             case 0x2000: // 2NNN Calls subroutine at NNN.
                printf("Calls subroutine at 0x%X\n", opcode & 0x0FFF);
                stack[sp] = pc;
                ++sp;
                pc = (opcode & 0x0FFF);
                printf("pc: 0x%X\n",pc);
             break;
             case 0x3000 : // 3XNN Skips the next instruction if VX equals NN
                printf("Skips the next instruction if V[0x%X] equals 0x%X\n", (opcode & 0x0F00) >> 8, opcode & 0x00FF);
             if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                 pc += 4;
             else 
                 pc += 2;
             break;
             case 0x4000: // 4XNN Skips the next instruction if VX doesn't equal NN
             if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                 pc += 4;
             else 
                 pc += 2;
             break;
             case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
             			if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
             				pc += 4;
             			else
             				pc += 2;
             break;
             case 0x6000: //6XNN Sets VX to NN.
                printf("Sets V[0x%X] to 0x%X\n", (opcode & 0x0F00) >> 8, opcode & 0x00FF);
                V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
                pc += 2;
             break;
             case 0x7000: //7XNN Adds NN to VX.
                printf("Adds 0x%X to V[0x%X]\n", opcode & 0x00FF, (opcode & 0x0F00) >> 8);
                V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
                pc += 2;
             break;
             case 0xA000 : //ANNN Sets I to the address NNN.  
                printf("Sets I to the address 0x%X\n", opcode & 0x0FFF);
                I = opcode & 0x0FFF;
                pc += 2;    
             break; 
             case 0x8000:
                 switch(opcode & 0x000F) {
                     case 0x0000: //8XY0 Sets VX to the value of VY.
                        V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                        pc += 2;
                     break;
                     case 0x0001 : //Sets VX to VX or VY. (Bitwise OR operation)
                        V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                        V[0xF] = 0;
                        pc += 2;
                     break;
                     case 0x0002 : //Sets VX to VX and VY. (Bitwise AND operation)
                        V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                        V[0xF] = 0;
                        pc += 2;
                     break;
                     case 0x0003 : //Sets VX to VX xor VY. VF is reset to 0.
                        V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                        V[0xF] = 0;
                        pc += 2;
                     break;
                     case 0x0004 : // 8XY4 Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                     if (V[(opcode & 0x0F00) >> 8] > (0xFF - V[(opcode & 0x00F0) >> 4])) {
                        V[0xF] = 1;
                     } else {
                        V[0xF] = 0;
                     }
                     V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                     pc += 2;
                     break;
                     case 0x0005 : // 8XY5 VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                        if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                            V[0xF] = 0;
                        else
                            V[0xF] = 1;
                        V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                        pc += 2;
                     break;
                     case 0x0006: // 8XY6 Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift.[2]
                        V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                        V[(opcode & 0x0F00) >> 8] >>= 1;
                        pc += 2;
                     break;
                     case 0x0007 : // 8XY7 Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                     if (V[(opcode & 0x00F0) >> 4] < V[(opcode & 0x0F00) >> 8]) { // Borrow
                        V[0xF] = 0;
                     } else {
                        V[0XF] = 1;
                     }
                     V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                     pc += 2;
                     break;
                     case 0x000E : // Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift.[2]
                        V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                        V[(opcode & 0x0F00) >> 8] <<= 1;
                        pc += 2;
                        break;
                     default : {printf ("Unknown opcode: 0x%X\n", opcode); return; }
                 }
             break;
             case 0x9000 : // 9XY0 Skips the next instruction if VX doesn't equal VY.
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
             break;
             case  0xC000: // CXNN Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
                V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
                pc += 2;
             break;
             case 0xD000: //DXYN Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
                {
                    unsigned short x = V[(opcode & 0x0F00) >> 8];
                    unsigned short y = V[(opcode & 0x00F0) >> 4];
                    unsigned short height = opcode & 0x000F;
                    unsigned short pixel;
                    printf("Draws a sprite at coordinate (%d, %d) that has a width of 8 pixels and a height of %d pixels\n", x, y, height);
                    if ((x >= 64) || (y >= 32)) { // Some games actually try to draw at invalid coordinates... That's lame...
                        V[0xF] = 0;
                        pc += 2;
                        return;
                    }

                    V[0xF] = 0;
                    for(int yline = 0; yline < height;yline++) {
                        pixel = memory[I + yline];
                       
                        for(int xline = 0; xline < 8; xline++)
				            {
					            if((pixel & (0x80 >> xline)) != 0)
					            {
						            if(gfx[(x + xline + ((y + yline) * 64))] == 1)
						            {
							            V[0xF] = 1;                                    
						            }
						            gfx[x + xline + ((y + yline) * 64)] ^= 1;
					            }
				            }
                    }
                    //printf ("opcode: 0x%X\n", opcode); 
                    drawFlag = true;
                    pc += 2;   
                    //debugVRAM();
                }
                break;
            case 0xE000 :
                switch(opcode & 0x00FF) {
                    case 0x00A1 : //Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
                        if (key[V[(opcode & 0x0F00) >> 8]] == 0) {
                            pc += 4;
                        } else pc += 2;
                    break;
                    case 0x009E : // Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
                        if (key[V[(opcode & 0x0F00) >> 8]] != 0) {
                            pc += 4;
                        } else pc += 2;
                    break;
                }
            break;    
            case 0xF000:
                switch(opcode & 0x00FF) {
                    case 0x0007 : //FX07 Sets VX to the value of the delay timer.
                        V[(opcode & 0x0F00) >> 8] = delay_timer;
                        pc += 2;
                    break;
                    case 0x0018 : //Sets the sound timer to VX.
                        sound_timer = V[(opcode & 0x0F00) >> 8];
                        pc += 2;
                    break;
                    case 0x000A: // A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                        for(int i = 0; i < 16 ;i++) {
                            if (key[i] == 1) { // key press!
                                V[(opcode & 0x0F00) >> 8] = i;
                                pc += 2;
                            }
                        }
                    break;
                    case 0x001E: //FX1E Adds VX to I
                        if (I + V[(opcode & 0x0F00) >> 8] > 0x0FFF) 
                            V[0xF] = 1;
                        else
                            V[0xF] = 0;
                            I += V[(opcode & 0x0F00) >> 8];
                            pc += 2;
                    break;
                    case 0x0015: // FX15 Sets the delay timer to VX.
                            delay_timer = V[(opcode & 0x0F00) >> 8];
                            pc += 2;
                    break;
                    case 0x0029: // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                        I = V[(opcode & 0x0F00) >> 8] * 0x5 ;
                        pc += 2;
                    break;
                    case 0x0055: // FX55 Stores V0 to VX (including VX) in memory starting at address I
                        for(char i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                            memory[I + i] = V[i];
                        }
                        pc += 2;
                    break;
                    case 0x0033: // FX33 Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2
                        memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
					    memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					    memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;	
                        pc += 2;
                    break;
                    case 0x0065:// FX65 Fills V0 to VX (including VX) with values from memory starting at address I
                        for(char i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                            V[i] = memory[I + i];
                        }
                        pc += 2;
                    break;
                    default : { printf ("Unknown opcode: 0x%X\n", opcode); return; }
                }
                 //default : printf ("Unknown opcode: 0x%X\n", opcode);
            break;
            default:
            printf ("Unknown opcode: 0x%X\n", opcode);
            return;
        }  
 
        // Update timers

        log(opcode);
        //dump();
    }

    void CPU::updateTimers() {
        if(delay_timer > 0)
                    --delay_timer;
                if(sound_timer > 0)
                {
                    if(sound_timer == 1)
                    printf("BEEP!\n");
                    --sound_timer;
                }
    }

    void CPU::clearScreen() {
        for(int i = 0; i < 2048;i++) {
            gfx[i] = 0;
        }
    }

    /*void CPU::debugVRAM() {
        for(int y = 0;y < 32;y++) { 
            for(int x = 0;x < 64;x++)
            {
                if(gfx[(y*64) + x] == 0) 
				    printf("O");
			    else 
				    printf(" ");
            }
                printf("\n");
        }
        printf("\n");
        drawFlag = false;
        //dump();
    }*/

    void CPU::log(unsigned short opcode) {
        printf("Opcode = 0x%X\n", opcode);
    }

    void CPU::setKeyState(char index, bool state) {
        key[index] = state ? 1 : 0;
    }
    
    void CPU::dump() {
        printf("\n");
        printf("pc = 0x%X\n", pc);
        printf("sp = 0x%X\n", sp);
        printf("I = 0x%X\n", I);
        printf("delay_timer = 0x%X\n", delay_timer);

        for(char i= 0;i < 16;i++) {
            printf("V[0x%X] = 0x%X\n", i, V[i]);
        }
    }