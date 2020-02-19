#include <ctime>
#include <iostream>
#include "chip8.hpp"
#include "OpcodeException.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#define RES_MULT 20
#define WIDTH 64
#define HEIGHT 32
#define PIXELS WIDTH * HEIGHT

void displayError(sf::RenderWindow &window, const std::string errorText, const uint8_t errorCode);
void playSound(void *sound);


int main(int argc, char* argv[]) {
	// Randomize random number generator for use at opcode 0xC000
	srand(static_cast<unsigned int>(time(nullptr)));

	sf::RenderWindow window(sf::VideoMode(64 * RES_MULT, 32 * RES_MULT), "Chip8 Emulator", sf::Style::Titlebar | sf::Style::Close);
	
	// Using pointer so it can later be ckecked if sound loaded successfully
	// withoud the need of additional code
	sf::SoundBuffer *soundBuffer = new sf::SoundBuffer();
	sf::Sound *sound = new sf::Sound();
	if (!(argc > 2 && soundBuffer->loadFromFile(argv[2]))) {
		delete soundBuffer;
		delete sound;
		sound = NULL;
	}
	else {
		sound->setBuffer(*soundBuffer);
	}

	Chip8 chip8;
	try {
		chip8.loadROM(argv[1]);
		chip8.setSound(playSound, (void *) sound);
	}
	catch (const std::runtime_error & error) {
		displayError(window, "File error", 3);
	}

	sf::Color pixelOff(sf::Color::Black);
	sf::Color pixelOn(sf::Color::White);
	sf::Event event;
	// As long the window is not closed
	while (window.isOpen()) {
		// and there is an event in the event queue
		/// if there's no pending event then it will return false and leave event unmodified
		while (window.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed: window.close(); break;
				case sf::Event::KeyPressed: {
					switch (event.key.code) {
					case sf::Keyboard::Num1: chip8.keyPress(0x1); break;
					case sf::Keyboard::Num2: chip8.keyPress(0x2); break;
					case sf::Keyboard::Num3: chip8.keyPress(0x3); break;
					case sf::Keyboard::Num4: chip8.keyPress(0xc); break;
					case sf::Keyboard::Q: chip8.keyPress(0x4); break;
					case sf::Keyboard::W: chip8.keyPress(0x5); break;
					case sf::Keyboard::E: chip8.keyPress(0x6); break;
					case sf::Keyboard::R: chip8.keyPress(0xd); break;
					case sf::Keyboard::A: chip8.keyPress(0x7); break;
					case sf::Keyboard::S: chip8.keyPress(0x8); break;
					case sf::Keyboard::D: chip8.keyPress(0x9); break;
					case sf::Keyboard::F: chip8.keyPress(0xe); break;
					case sf::Keyboard::Z: chip8.keyPress(0xa); break;
					case sf::Keyboard::X: chip8.keyPress(0x0); break;
					case sf::Keyboard::C: chip8.keyPress(0xb); break;
					case sf::Keyboard::V: chip8.keyPress(0xf); break;
					}
					break;
				}
				case sf::Event::KeyReleased: {
					switch (event.key.code) {
					case sf::Keyboard::Num1: chip8.keyRelease(0x1); break;
					case sf::Keyboard::Num2: chip8.keyRelease(0x2); break;
					case sf::Keyboard::Num3: chip8.keyRelease(0x3); break;
					case sf::Keyboard::Num4: chip8.keyRelease(0xc); break;
					case sf::Keyboard::Q: chip8.keyRelease(0x4); break;
					case sf::Keyboard::W: chip8.keyRelease(0x5); break;
					case sf::Keyboard::E: chip8.keyRelease(0x6); break;
					case sf::Keyboard::R: chip8.keyRelease(0xd); break;
					case sf::Keyboard::A: chip8.keyRelease(0x7); break;
					case sf::Keyboard::S: chip8.keyRelease(0x8); break;
					case sf::Keyboard::D: chip8.keyRelease(0x9); break;
					case sf::Keyboard::F: chip8.keyRelease(0xe); break;
					case sf::Keyboard::Z: chip8.keyRelease(0xa); break;
					case sf::Keyboard::X: chip8.keyRelease(0x0); break;
					case sf::Keyboard::C: chip8.keyRelease(0xb); break;
					case sf::Keyboard::V: chip8.keyRelease(0xf); break;
					}
					break;
				}
			}
		}
		// If there is no other event in queue

		try {
			chip8.emulateCycle();
		}
		catch (OpCodeException e) {
			std::cerr << e.getMessage() << std::endl << "opcode: " << e.getOpCode() << std::endl << "Memory offset: " << e.getOffset() << std::endl;

			window.clear();
			displayError(window, "Exception", 5);
		}

		sf::RectangleShape pixel;
		uint8_t i = 0;
		for (auto memPixel : chip8.getPixels()) {
			pixel.setFillColor(memPixel ? pixelOn : pixelOff);
			pixel.setPosition(i % WIDTH * RES_MULT, i / WIDTH * RES_MULT);
			pixel.setSize(sf::Vector2f(RES_MULT, RES_MULT));
			// Draw primitives defined by a vertex buffer
			window.draw(pixel);
		}

		// Display on screen what has been rendered to the window so far
		window.display();
	}

	delete sound;
	
	return 0;
}

void displayError(sf::RenderWindow &window, const std::string errorText, const uint8_t errorCode) {
	sf::Text errorMsg;
	errorMsg.setCharacterSize(256);
	errorMsg.setFillColor(sf::Color::Red);
	errorMsg.setStyle(sf::Text::Style::Bold);
	errorMsg.setString(errorText);

	window.draw(errorMsg);

	// Block execution until any event is occured
	sf::Event event;
	window.waitEvent(event);

	exit(errorCode);
}

void playSound(void *sound) {
	// Emulator class will not be dependent from a certain library
	// thus we use a void pointer and typecast it to our library
	sf::Sound *playSound;
	if (sound) {
		playSound = (sf::Sound*) sound;
		playSound->play();
	}
	else {
		// Play system sound
		fprintf(stdout, "%c", 7);
	}
}