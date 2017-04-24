class CPU {
    public:
        unsigned short opcode;
        unsigned char memory[4096];
        unsigned char V[16];
        unsigned short I;
        unsigned short pc;
        unsigned char gfx[64 * 32];
        unsigned char delay_timer;
        unsigned char sound_timer;
        unsigned short stack[16];
        unsigned short sp;
        unsigned char key[16];

        bool drawFlag = true;
       
        void init();
        void loadProgram(char*, int);
        void setKeyState(char key, bool state);
        void emulateCycle();
        void updateTimers();
        void debugVRAM();
        void dump();
    private:
        void clearScreen();
        void log(unsigned short);
        bool debug = true;
};