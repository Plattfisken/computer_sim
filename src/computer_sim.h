#ifndef COMPUTER_SIM_H
#define COMPUTER_SIM_H

#include "../third_party/useful_things.h"
#include <stdint.h>

#define COMPUTER_RAM_SIZE KILOBYTES(64)
#define COMPUTER_MAX_PROGRAM_SIZE KILOBYTES(16)
#define COMPUTER_PROGRAM_ENTRY_POINT (COMPUTER_RAM_SIZE - COMPUTER_MAX_PROGRAM_SIZE)
#define COMPUTER_SCREEN_BUFFER_WIDTH 64
#define COMPUTER_SCREEN_BUFFER_HEIGHT 36
#define COMPUTER_SCREEN_BUFFER_SIZE (COMPUTER_SCREEN_BUFFER_WIDTH * COMPUTER_SCREEN_BUFFER_HEIGHT)
#define COMPUTER_SCREEN_BUFFER_DATA (COMPUTER_PROGRAM_ENTRY_POINT - COMPUTER_SCREEN_BUFFER_SIZE)

typedef struct {
    uint16_t registers[7];
    uint16_t program_counter;
    void *memory_base;
} Computer;

void computer_init(Computer *computer);

typedef struct {
    bool success;
    UT_String error_message;
} AssemblerResult;
AssemblerResult computer_assemble_and_load_program(Computer *computer, UT_String assembly);

void computer_execute_next_instruction(Computer *computer);

void *computer_pointer_from_virtual_address(Computer *computer, uint16_t address);

#endif //COMPUTER_SIM_H
