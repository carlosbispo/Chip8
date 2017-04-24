#include <cstdint>
#include <SFML/Graphics.hpp>
#include "CPU.h"

void updateInput(CPU &cpu);

int main(int argc, char *argv[])
{
   //sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!"); 
   FILE *file;
   char *game;
   char c;
   int index;
   CPU cpu;
   sf::Texture texture;
   sf::Clock clock;
   static float refreshSpeed = 1.f/60.f;

   if (!texture.create(64,32)) {
        printf("error creating texture...");
        return 1;
   }

   sf::Sprite sprite(texture);
   sprite.scale(sf::Vector2f(16.0f, 16.0f));


   // Create pointer of pixels
   sf::Uint8* pixels;
   // Allocate pixels
   pixels = new sf::Uint8[64 * 32 * 4];

   sf::RenderWindow window(sf::VideoMode(1024, 512), "Chip8");

   file = fopen(argv[1], "rb");
   if (file == NULL) {
       printf("File %s not found!!\n", argv[1]);
       return 1;
   } 

   fseek(file, 0, SEEK_END);
   long fileSize = ftell(file);
   rewind(file);
   // Alloc memory
   game = (char*)malloc(sizeof(char) * fileSize);

   fread (game, 1, fileSize, file);
    
    fclose(file);
   
    cpu.init();
    cpu.loadProgram(game, fileSize);
    window.setFramerateLimit(50);

    while (window.isOpen())
    {

        sf::Event event;

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        cpu.emulateCycle();
        if(clock.getElapsedTime().asSeconds() >= refreshSpeed){
            cpu.updateTimers();
            updateInput(cpu);
            clock.restart();
        }

        if (cpu.drawFlag) { // Time to draw stuff...
            window.clear(sf::Color::Black);
            for(int i = 0;i < 64*32 * 4;i+=4) {
                    unsigned char color = cpu.gfx[i / 4];
                    color = color == 0 ? 0 : 0xff;
                    pixels[i] = color;
                    pixels[i + 1] = color;
                    pixels[i + 2] = color;
                    pixels[i + 3] = 0xff;
             }

            texture.update(pixels);
            window.draw(sprite);
            window.display();
            cpu.drawFlag = false;
        }
        //clock.restart();
    }
    return 0;
}

void updateInput(CPU &cpu) {
    // handle input
            cpu.setKeyState(0x1, sf::Keyboard::isKeyPressed(sf::Keyboard::Num1));
            cpu.setKeyState(0x2, sf::Keyboard::isKeyPressed(sf::Keyboard::Num2));
            cpu.setKeyState(0x3, sf::Keyboard::isKeyPressed(sf::Keyboard::Num3));
            cpu.setKeyState(0xC, sf::Keyboard::isKeyPressed(sf::Keyboard::Num4));
            cpu.setKeyState(0x4, sf::Keyboard::isKeyPressed(sf::Keyboard::Q));
            cpu.setKeyState(0x5, sf::Keyboard::isKeyPressed(sf::Keyboard::W));
            cpu.setKeyState(0x6, sf::Keyboard::isKeyPressed(sf::Keyboard::E));
            cpu.setKeyState(0xD, sf::Keyboard::isKeyPressed(sf::Keyboard::R));
            cpu.setKeyState(0x7, sf::Keyboard::isKeyPressed(sf::Keyboard::A));
            cpu.setKeyState(0x8, sf::Keyboard::isKeyPressed(sf::Keyboard::S));
            cpu.setKeyState(0x9, sf::Keyboard::isKeyPressed(sf::Keyboard::D));
            cpu.setKeyState(0xE, sf::Keyboard::isKeyPressed(sf::Keyboard::F));
            cpu.setKeyState(0xA, sf::Keyboard::isKeyPressed(sf::Keyboard::Z));
            cpu.setKeyState(0x0, sf::Keyboard::isKeyPressed(sf::Keyboard::X));
            cpu.setKeyState(0xB, sf::Keyboard::isKeyPressed(sf::Keyboard::C));
            cpu.setKeyState(0xF, sf::Keyboard::isKeyPressed(sf::Keyboard::V));
}