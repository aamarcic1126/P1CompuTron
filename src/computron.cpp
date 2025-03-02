#include "computron.h"

#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <cstdlib>
#include <sstream>

Command opCodeToCommand(size_t opCode) {
	switch (opCode) {
	case 10: return Command::read;
		// ToDo: Complete
	case 11: return Command::write;
	case 20: return Command::load;
	case 21: return Command::store;
	case 30: return Command::add;
	case 31: return Command::subtract;
	case 32: return Command::divide;
	case 33: return Command::multiply;
	case 40: return Command::branch;
	case 41: return Command::branchNeg;
	case 42: return Command::branchZero;
	case 43: return Command::halt;
	default:
		throw std::runtime_error("Invalid weifubwegiuwebg");
	}
}

void load_from_file(std::array<int, memorySize>& memory, const std::string& filename) {
	constexpr int sentinel{ -99999 }; // terminates reading after -99999
	size_t i{ 0 };
	std::string line;
	int instruction;

	std::ifstream inputFile(filename);
	if (!inputFile)
		// throw runtime_error exception with string "invalid_input"
		throw std::runtime_error("invalid_input");


	while (std::getline(inputFile, line)) {
		instruction = std::stoi(line);
		if (instruction == sentinel) break;

		// Check if the instruction is valid using the validWord function
		// If the instruction is valid, store it in memory at position 'i' and increment 'i'
		// If the instruction is invalid, throw a runtime error with message "invalid_input"
		if (!validWord(instruction)) throw std::runtime_error("invalid_input");
		if (i >= memorySize) throw std::runtime_error("invalid_input");
		memory[i++] = instruction;
	}
	inputFile.close();
}

void execute(std::array<int, memorySize>& memory, int* const acPtr,
			size_t* const icPtr, int* const irPtr,
			size_t* const opCodePtr, size_t* const opPtr,
			const std::vector<int>& inputs) {

	size_t inputIndex{ 0 }; // Tracks input

	do {
		// instruciton counter to register
		// instructionRegister = memory [instructionCounter];
		// operationCode = instructionRegister / 100; // divide
		// operand = instructionRegister % 100; // remainder

		// Fetch instruction from memory and store in the instructionRegister
		if (*icPtr >= memorySize) throw std::runtime_error("Instruction counter out of range");
		*irPtr = memory[*icPtr];

		// Decode the instruction
		*opCodePtr = static_cast<size_t>((*irPtr) / 100);
		*opPtr = static_cast<size_t>((*irPtr) % 100);

		// Check if operand is in the valid range
		if (*opPtr >= memorySize) throw std::runtime_error("Operand out of range");

		// Execute
		switch (int word{}; opCodeToCommand(*opCodePtr)) {

		case Command::read:
			if (inputIndex >= inputs.size()) throw std::runtime_error("Not enough input values");

			word = inputs[inputIndex];

			if (!validWord(word)) throw std::runtime_error("Invalid word");

			// Assign the value of 'word' to the memory location pointed to by 'opPtr'
			memory[*opPtr] = word;
			// Increment the instruction counter (icPtr) to point to the next instruction
			++(*icPtr);
			inputIndex++;
			break;

		case Command::write:
			// Dereference 'icPtr' to access the instruction counter and increment its value by 1
			// use the below cout if needed but comment before submission
			// std::cout << "Contents of " << std::setfill('0') << std::setw(2)
			//           << *opPtr << " : " << memory[*opPtr] << "\n";
			++(*icPtr);
			break;

		case Command::load:
			// Load the value from memory location pointed to by 'opPtr'  into the accumulator (acPtr)
			// Increment the instruction counter (icPtr) to point to the next instruction
			*acPtr = memory[*opPtr];
			++(*icPtr);
			break;

		case Command::store:
			// Store the value in the accumulator (acPtr) into the memory location pointed to by 'opPtr'
			// Increment the instruction counter (icPtr) to move to the next instruction
			memory[*opPtr] = *acPtr;
			++(*icPtr);
			break;

		case Command::add:
			// Add the value in the accumulator (acPtr) to the value in memory at the location pointd to 
			// by 'opPtr' and store the result in 'word'
			// If the result is valid, store it in the accumulator and increment the instruction counter
			// / If the result is invalid, throw a runtime error
			word = *acPtr + memory[*opPtr];
			if (!validWord(word)) throw std::runtime_error("Addition out of range");
			*acPtr = word;
			++(*icPtr);
			break;

		case Command::subtract:
			// Subtract the value in memory at the location pointed to by 'opPtr' from the value in the
			// accumulator (acPtr) and store the result in 'word'
			// If the result is valid, store it in the accumulator and increment the instruction counter
			// / If the result is invalid, throw a runtime error
			word = *acPtr - memory[*opPtr];
			if (!validWord(word)) throw std::runtime_error("Subtraction out of range");
			*acPtr = word;
			++(*icPtr);
			break;

		case Command::multiply:
			// as above do it for multiplication
			word = *acPtr * memory[*opPtr];
			if (!validWord(word)) throw std::runtime_error("Multiplication out of range");
			*acPtr = word;
			++(*icPtr);
			break;

		case Command::divide:
			// as above do it for division
			if (memory[*opPtr] == 0) throw std::runtime_error("Division by 0");
			word = *acPtr / memory[*opPtr];
			if (!validWord(word)) throw std::runtime_error("Division out of range");
			*acPtr = word;
			++(*icPtr);
			break;

		case Command::branch:
			*icPtr = *opPtr;
			break;

		case Command::branchNeg:
			*acPtr < 0 ? *icPtr = *opPtr : ++(*icPtr);
			break;

		case Command::branchZero:
			*acPtr == 0 ? *icPtr = *opPtr : ++(*icPtr);
			break;

		case Command::halt:
			break;

		default:
			// any instruction required
			throw std::runtime_error("Unknown instruction");
			break;
		}
		// You may modify the below while condition if required
	} while (opCodeToCommand(*opCodePtr) != Command::halt);
}

bool validWord(int word) {
	return (word >= minWord && word <= maxWord);
}

void dump(std::array <int, memorySize>& memory, int accumulator,
		size_t instructionCounter, size_t instructionRegister,
		size_t operationCode, size_t operand) {

	// Print registers
	std::cout << "Registers" << std::endl;

	output("accumulator", 14, (accumulator), true);
	output("instructionCounter", 7, static_cast<int>(instructionCounter), false);
	output("instructionRegister", 6, instructionRegister, true);
	output("operationCode", 10, static_cast<int>(operationCode), false);
	output("operand", 14, static_cast<int>(operand), false);

	// Print memory

	for (size_t row = 0; row < memorySize; row += 10) {
		// Left column
		std::cout << std::setw(2) << std::setfill('0') << row << " ";

		// Print 10 cells per row
		for (size_t col = 0; col < 10; ++col) {
			size_t index = row + col;
			if (index >= memorySize) break;

			std::ostringstream oss;
			oss << std::internal << std::showpos << std::setw(5) << std::setfill('0') << memory[index];
			std::cout << oss.str();
		}
		std::cout << std::endl;
	}
}

void output(std::string label, int width, int value, bool sign) {

	std::cout << std::setw(24) << std::left << label;

	if (sign) {
		// Print the sign and then the value
		char signChar = (value >= 0) ? '+' : '-';
		int absValue = (value >= 0) ? value : -value;

		std::cout << signChar << std::setw(width - 1) << std::setfill('0') << absValue << std::endl;
	}
	else {
		std::cout << std::setw(width) << std::setfill(' ') << value << std::endl;
	}
}
