#include "OpcodeException.hpp"

OpCodeException::OpCodeException(uint16_t opCode, uint8_t offset) : OpCodeException("Invalid opcode", opCode, offset)
{ }

OpCodeException::OpCodeException(std::string msg, uint16_t opCode, uint8_t offset) : msg(msg), opCode(opCode), offset(offset)
{ }

std::string OpCodeException::getMessage(void) {
	return msg;
}

uint16_t OpCodeException::getOpCode(void) {
	return opCode;
}

uint8_t OpCodeException::getOffset(void) {
	return offset;
}