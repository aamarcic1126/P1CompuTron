#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#include "computron.h"

TEST_CASE("validWord function tests", "[validWord]") {
    // Check the min boundary
    REQUIRE(validWord(-9999) == true);
    // Check just below the min boundary
    REQUIRE(validWord(-10000) == false);

    // Check the max boundary
    REQUIRE(validWord(9999) == true);
    // Check just above the max boundary
    REQUIRE(validWord(10000) == false);

    // Check zero
    REQUIRE(validWord(0) == true);

    // Check something in the middle
    REQUIRE(validWord(1234) == true);
    REQUIRE(validWord(-55555) == false);
}

TEST_CASE("load_from_file success", "[load_from_file]") {
    // Create a temporary file with valid instructions
    {
        std::ofstream ofs("temp_valid.txt");
        ofs << "20" << "\n";
        ofs << "330" << "\n";
        ofs << "9999" << "\n";   // 4-digit data
        ofs << "1007" << "\n";   // 4-digit instruction
        ofs << "-99999" << "\n"; // sentinel
    }

    // Prepare memory array
    std::array<int, memorySize> memory{};
    for (auto& m : memory) {
        m = 0;
    }

    // Call load_from_file
    REQUIRE_NOTHROW(load_from_file(memory, "temp_valid.txt"));

    // Check that the loaded values are correct
    CHECK(memory[0] == 20);
    CHECK(memory[1] == 330);
    CHECK(memory[2] == 9999);  // valid word
    CHECK(memory[3] == 1007);  // opcode=10 operand=07
    // memory[4] should remain 0 or never be assigned after sentinel
    CHECK(memory[4] == 0);
}

TEST_CASE("load_from_file fails on invalid word", "[load_from_file]") {
    // Create a file with an out-of-range word
    {
        std::ofstream ofs("temp_invalid.txt");
        ofs << "20000" << "\n";   // out of range ( > 9999 )
        ofs << "-99999" << "\n";
    }

    std::array<int, memorySize> memory{};
    // Should throw std::runtime_error("invalid_input")
    REQUIRE_THROWS_AS(load_from_file(memory, "temp_invalid.txt"), std::runtime_error);
}

TEST_CASE("execute - read command", "[execute][read]") {
    // Arrange memory: read from inputs into memory[5], then halt
    // read = opcode 10, operand = 05 => 1005
    // halt = opcode 43, operand = 00 => 4300
    std::array<int, memorySize> memory{};
    memory[0] = 1005;  // read into mem[5]
    memory[1] = 4300;  // halt

    // CPU registers:
    int accumulator = 0;
    size_t instructionCounter = 0;
    int instructionRegister = 0;
    size_t operationCode = 0;
    size_t operand = 0;

    // Supply inputs = {42}
    std::vector<int> inputs{ 42 };

    // Execute
    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &instructionCounter,
        &instructionRegister,
        &operationCode,
        &operand,
        inputs));

    // After read, memory[5] should be 42, instructionCounter should be 1
    CHECK(memory[5] == 42);
    CHECK(instructionCounter == 1);

    // Once halt is encountered, we exit the loop:
    CHECK(operationCode == 43); // confirm last opcode was 43
}

TEST_CASE("execute - write command", "[execute][write]") {
    // memory[0] = 1105 => opcode=11 (write), operand=05
    // memory[1] = 4300 => halt
    // memory[5] = 99   => sample data to be written
    std::array<int, memorySize> memory{};
    memory[0] = 1105;
    memory[1] = 4300;
    memory[5] = 99;

    int accumulator = 0;
    size_t instructionCounter = 0;
    int instructionRegister = 0;
    size_t operationCode = 0;
    size_t operand = 0;

    std::vector<int> inputs;

    // Ensure it doesn't throw an "invalid opcode" or crash.
    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &instructionCounter,
        &instructionRegister,
        &operationCode,
        &operand,
        inputs));

    CHECK(instructionCounter == 1);   // after write, it increments
    CHECK(operationCode == 43);       // last opcode is the halt
}

TEST_CASE("execute - load command", "[execute][load]") {
    // memory[0] = 2005 => opcode=20 (load), operand=05
    // memory[1] = 4300 => halt
    // memory[5] = 777  => data to be loaded
    std::array<int, memorySize> memory{};
    memory[0] = 2005;
    memory[1] = 4300;
    memory[5] = 777;

    int accumulator = 0;
    size_t instructionCounter = 0;
    int instructionRegister = 0;
    size_t operationCode = 0;
    size_t operand = 0;

    std::vector<int> inputs{};

    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &instructionCounter,
        &instructionRegister,
        &operationCode,
        &operand,
        inputs));

    // After load instruction, accumulator should be 777
    CHECK(accumulator == 777);
    // instructionCounter should have advanced to 1
    CHECK(instructionCounter == 1);
    // halt
    CHECK(operationCode == 43);
}

TEST_CASE("execute - store command", "[execute][store]") {
    // memory[0] = 2107 => store into mem[07]
    // memory[1] = 4300 => halt
    // Pre-set the accumulator to 555.
    std::array<int, memorySize> memory{};
    memory[0] = 2107;
    memory[1] = 4300;

    int accumulator = 555;
    size_t instructionCounter = 0;
    int instructionRegister = 0;
    size_t operationCode = 0;
    size_t operand = 0;

    std::vector<int> inputs{};

    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &instructionCounter,
        &instructionRegister,
        &operationCode,
        &operand,
        inputs));

    // The value from the accumulator should now be in memory[7]
    CHECK(memory[7] == 555);
    CHECK(instructionCounter == 1);
    CHECK(operationCode == 43);
}

TEST_CASE("execute - add command", "[execute][add]") {
    // Load from mem[10], add mem[11], then halt.
    // memory[0] = 2010 => load from mem[10]
    // memory[1] = 3011 => add mem[11]
    // memory[2] = 4300 => halt
    // memory[10] = 6, memory[11] = 7
    std::array<int, memorySize> memory{};
    memory[0] = 2010;  // load
    memory[1] = 3011;  // add
    memory[2] = 4300;  // halt
    memory[10] = 6;
    memory[11] = 7;

    int accumulator = 0;
    size_t instructionCounter = 0;
    int instructionRegister = 0;
    size_t operationCode = 0;
    size_t operand = 0;

    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &instructionCounter,
        &instructionRegister,
        &operationCode,
        &operand,
        {}));

    // After load => accumulator=6
    // After add => accumulator=13
    CHECK(accumulator == 13);
    CHECK(instructionCounter == 2);  // halted at memory[2]
    CHECK(operationCode == 43);
}

TEST_CASE("execute - subtract command", "[execute][subtract]") {
    // memory[0] = 2010 => load mem[10]
    // memory[1] = 3111 => subtract mem[11]
    // memory[2] = 4300 => halt
    // memory[10] = 20, memory[11] = 5 => final accum=15
    std::array<int, memorySize> memory{};
    memory[0] = 2010;
    memory[1] = 3111;
    memory[2] = 4300;
    memory[10] = 20;
    memory[11] = 5;

    int accumulator = 0;
    size_t ic = 0;
    int ir = 0;
    size_t opCode = 0;
    size_t operand = 0;

    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &ic,
        &ir,
        &opCode,
        &operand,
        {}));

    CHECK(accumulator == 15);
    CHECK(ic == 2);
    CHECK(opCode == 43);
}

TEST_CASE("execute - multiply command (normal)", "[execute][multiply]") {
    // Program flow:
    //   memory[0] = 2010 -> load mem[10] into accumulator
    //   memory[1] = 3311 -> multiply with mem[11]
    //   memory[2] = 4300 -> halt
    //
    // Place the values in memory[10] and [11], run, then check the result.

    std::array<int, memorySize> memory{};
    memory[0] = 2010;  // load from location 10
    memory[1] = 3311;  // multiply with location 11
    memory[2] = 4300;  // halt

    // Put test values
    memory[10] = 12;
    memory[11] = 3;

    // CPU registers
    int accumulator = 0;
    size_t instructionCounter = 0;
    int instructionRegister = 0;
    size_t operationCode = 0;
    size_t operand = 0;

    // No input needed
    std::vector<int> inputs;

    // Act: run execute
    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &instructionCounter,
        &instructionRegister,
        &operationCode,
        &operand,
        inputs));

    // After "load mem[10]", accumulator = 12
    // After "multiply mem[11]", accumulator = 36
    REQUIRE(accumulator == 36);
    REQUIRE(instructionCounter == 2);  // halted on memory[2]
    REQUIRE(operationCode == 43);
}

TEST_CASE("execute - multiply command (negative)", "[execute][multiply]") {
    //   memory[0] = 2010 -> load mem[10]
    //   memory[1] = 3311 -> multiply mem[11]
    //   memory[2] = 4300 -> halt

    std::array<int, memorySize> memory{};
    memory[0] = 2010;
    memory[1] = 3311;
    memory[2] = 4300;

    memory[10] = -5;
    memory[11] = 10;

    int accumulator = 0;
    size_t ic = 0;
    int ir = 0;
    size_t opCode = 0;
    size_t operand = 0;

    REQUIRE_NOTHROW(execute(memory, &accumulator, &ic, &ir, &opCode, &operand, {}));

    // -5 loaded, multiply by 10 => -50
    REQUIRE(accumulator == -50);
    REQUIRE(ic == 2);
    REQUIRE(opCode == 43);
}

TEST_CASE("execute - multiply command (out of range)", "[execute][multiply]") {
    // memory[0] = 2010 => load mem[10]
    // memory[1] = 3311 => multiply mem[11]
    // memory[2] = 4300 => halt

    std::array<int, memorySize> memory{};
    memory[0] = 2010;
    memory[1] = 3311;
    memory[2] = 4300;

    // Data that exceeds range, e.g. 500 * 30 = 15000
    memory[10] = 500;
    memory[11] = 30;

    int accumulator = 0;
    size_t ic = 0;
    int ir = 0;
    size_t opCode = 0;
    size_t operand = 0;

    // Expect a runtime_error
    REQUIRE_THROWS_AS(
        execute(memory, &accumulator, &ic, &ir, &opCode, &operand, {}),
        std::runtime_error
    );
}

TEST_CASE("execute - divide command (normal)", "[execute][divide]") {
    //   memory[0] = 2010 -> load mem[10]
    //   memory[1] = 3211 -> divide by mem[11]
    //   memory[2] = 4300 -> halt
    std::array<int, memorySize> memory{};
    memory[0] = 2010;  // load
    memory[1] = 3211;  // divide
    memory[2] = 4300;  // halt

    memory[10] = 100;
    memory[11] = 25;

    int accumulator = 0;
    size_t instructionCounter = 0;
    int instructionRegister = 0;
    size_t operationCode = 0;
    size_t operand = 0;

    REQUIRE_NOTHROW(execute(memory,
        &accumulator,
        &instructionCounter,
        &instructionRegister,
        &operationCode,
        &operand,
        {}));

    // 100 / 25 = 4
    REQUIRE(accumulator == 4);
    REQUIRE(instructionCounter == 2);
    REQUIRE(operationCode == 43);
}

TEST_CASE("execute - divide command (divide by zero)", "[execute][divide]") {
    //   memory[0] = 2010 -> load mem[10]  (which will be 123)
    //   memory[1] = 3211 -> divide by mem[11] (which is 0 => error)
    //   memory[2] = 4300 -> halt
    std::array<int, memorySize> memory{};
    memory[0] = 2010;
    memory[1] = 3211;
    memory[2] = 4300;

    // Accumulator will hold 123, then attempt to divide by 0 => throws
    memory[10] = 123;
    memory[11] = 0;

    int accumulator = 0;
    size_t ic = 0;
    int ir = 0;
    size_t opCode = 0;
    size_t operand = 0;

    // Expect runtime_error for dividing by zero
    REQUIRE_THROWS_AS(
        execute(memory, &accumulator, &ic, &ir, &opCode, &operand, {}),
        std::runtime_error
    );
}

TEST_CASE("execute - divide command (out of range)", "[execute][divide]") {

    std::array<int, memorySize> memory{};
    memory[0] = 2010;  // load mem[10]
    memory[1] = 3211;  // divide by mem[11]
    memory[2] = 4300;  // halt

    memory[10] = 12000; // not a valid word by spec
    memory[11] = 1;

    int accumulator = 0;
    size_t ic = 0;
    int ir = 0;
    size_t opCode = 0;
    size_t operand = 0;


    REQUIRE_THROWS_AS(
        execute(memory, &accumulator, &ic, &ir, &opCode, &operand, {}),
        std::runtime_error
    );
}


TEST_CASE("execute - branch command", "[execute][branch]") {
    // memory[0] = 4005 => branch to 05
    // memory[1] = 4300 => halt  (we skip it)
    // memory[5] = 4300 => halt
    std::array<int, memorySize> memory{};
    memory[0] = 4005;
    memory[1] = 4300;
    memory[5] = 4300;

    int ac = 0;
    size_t ic = 0;
    int ir = 0;
    size_t opCode = 0;
    size_t operand = 0;

    REQUIRE_NOTHROW(execute(memory, &ac, &ic, &ir, &opCode, &operand, {}));

    // Branched to 05, so the final instruction was memory[5] = 4300
    CHECK(ic == 5);      // after the branch, executed the halt at index 5
    CHECK(opCode == 43); // last opcode was halt
}

TEST_CASE("execute - halt only", "[execute][halt]") {
    // memory[0] = 4300 => halt
    std::array<int, memorySize> memory{};
    memory[0] = 4300;

    int ac = 0;
    size_t ic = 0;
    int ir = 0;
    size_t opCode = 0;
    size_t operand = 0;

    // Shouldn't throw
    REQUIRE_NOTHROW(execute(memory, &ac, &ic, &ir, &opCode, &operand, {}));
    CHECK(ic == 0);      // never advanced the instruction counter
    CHECK(opCode == 43); // last operation was halt
}
