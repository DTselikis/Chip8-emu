#include <exception>
#include <string>
#include <cstdint>

class OpCodeException : public std::exception {
private:
	std::string msg;
	uint16_t opCode;
	uint8_t offset;

public:
	OpCodeException(
		uint16_t opCode,
		uint8_t memOffset
	);

	OpCodeException(
		std::string msg,
		uint16_t opCode,
		uint8_t memOffset
	);

	std::string getMessage(void);
	uint16_t getOpCode(void);
	uint8_t getOffset(void);
};