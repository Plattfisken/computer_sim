#include "computer_sim.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USEFUL_THINGS_STRIP_PREFIX
#include "../third_party/useful_things.h"

typedef enum {
    MOV_R0, // Move immediate to register
    MOV_R1,
    MOV_R2,
    MOV_R3,
    MOV_R4,
    MOV_R5,
    MOV_R6,
    //MOV_R7,
    STR, // Store to memory address stored in register a the value in register b
    LDR, // Load from memory address stored in register a to register b

    JMP,
    //NOT,
    //AND,
    //IOR,
    //XOR,

    //INC,
    //DEC,

    //ADD,
    //SUB,
    //CMP,
    OPCODES_COUNT
} OPCODES;

void computer_init(Computer *computer) {
    if(computer->memory_base) free(computer->memory_base);
    computer->memory_base = malloc(COMPUTER_RAM_SIZE);
    computer->program_counter = COMPUTER_PROGRAM_ENTRY_POINT;
    memset(computer->registers, 0, sizeof computer->registers);
    uint16_t offset = 0;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = MOV_R1;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0xCE;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0xFA;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = MOV_R2;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0x0D;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0xF0;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = MOV_R3;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0x00;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0x15;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = MOV_R4;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0x37;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0x13;

    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = MOV_R0;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = (uint8_t)COMPUTER_SCREEN_BUFFER_DATA;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = (uint8_t)(COMPUTER_SCREEN_BUFFER_DATA >> 8);
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = MOV_R1;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0xff;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 0xff;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = STR;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = 1;

    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = JMP;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = (uint8_t)COMPUTER_PROGRAM_ENTRY_POINT;
    ((uint8_t *)computer->memory_base)[computer->program_counter + offset++] = (uint8_t)(COMPUTER_PROGRAM_ENTRY_POINT >> 8);


}


uint8_t computer_read_next_instruction_byte(Computer *computer) {
    return ((uint8_t *)computer->memory_base)[computer->program_counter++];
}

void *computer_pointer_from_virtual_address(Computer *computer, uint16_t address) {
    return computer->memory_base + address;
}

// extracts a little endian uint16 from program counter and advances it by two bytes
uint16_t computer_read_uint16_instruction_bytes(Computer *computer) {
    uint16_t ret = *(uint16_t *)computer_pointer_from_virtual_address(computer, computer->program_counter);
    computer->program_counter += 2;
    return ret;
}

void computer_write_byte_to_address(Computer *computer, uint8_t byte, uint16_t address) {
    *(uint8_t *)computer_pointer_from_virtual_address(computer, address) = byte;
}

void computer_write_uint16_to_address(Computer *computer, uint16_t num, uint16_t address) {
    *(uint16_t *)computer_pointer_from_virtual_address(computer, address) = num;
}

void computer_execute_next_instruction(Computer *computer) {
    uint8_t opcode = computer_read_next_instruction_byte(computer);
    switch(opcode) {
        case MOV_R0:
        case MOV_R1:
        case MOV_R2:
        case MOV_R3:
        case MOV_R4:
        case MOV_R5:
        case MOV_R6: {
            uint8_t register_idx = opcode;
            uint8_t least_significant_byte = computer_read_next_instruction_byte(computer);
            uint8_t most_significant_byte = computer_read_next_instruction_byte(computer);
            uint16_t immediate_val = (most_significant_byte << 8) | least_significant_byte;
            computer->registers[register_idx] = immediate_val;
        } break;
        case LDR: {
            uint8_t registers = computer_read_next_instruction_byte(computer);
            uint32_t address_register = registers >> 4; // upper 4 bits
            uint32_t destination_register = registers & 0xf; // lower 4 bits
            assert(address_register < ARRAY_LENGTH(computer->registers) && "register number is out of bounds");
            assert(destination_register < ARRAY_LENGTH(computer->registers) && "register number is out of bounds");

            uint16_t load_address = computer->registers[address_register];
            computer->registers[destination_register] = *((uint16_t *)computer_pointer_from_virtual_address(computer, load_address));
        } break;
        case STR: {
            uint8_t registers = computer_read_next_instruction_byte(computer);
            uint32_t address_register = registers >> 4; // upper 4 bits
            uint32_t value_register = registers & 0xf; // lower 4 bits
            assert(address_register < ARRAY_LENGTH(computer->registers) && "register number is out of bounds");
            assert(value_register < ARRAY_LENGTH(computer->registers) && "register number is out of bounds");

            uint16_t store_address = computer->registers[address_register];
            *((uint16_t *)computer_pointer_from_virtual_address(computer, store_address)) = computer->registers[value_register];
        } break;
        case JMP: {
            uint16_t jump_address = computer_read_uint16_instruction_bytes(computer);
            computer->program_counter = jump_address;
        } break;
        default: {
            assert(0 && "Unimplemented instruction");
        } break;
    }
}

bool is_between(char c, char min_val, char max_val) {
    if(c >= min_val && c <= max_val) return true;
    else return false;
}

bool parse_hex_uint16(String s, uint16_t *out_result) {
// TODO: this doesn't handle overflow at all, you could enforce a max length on the string to make sure it won't overflow, but that would invalidate strings such as "0x00001" which isn't actually too big, what other ways to avoid overflowing?
    uint16_t result = 0;
    size_t cur = 0;
    // optional 0x/0X prefix
    if(s.length >= 2 && s.data[cur] == '0' && (s.data[cur + 1] == 'x' || s.data[cur + 1] == 'X')) {
        cur += 2;
    }
    // empty string or just "0x"/"0X", we don't accept this as a valid number
    if(cur >= s.length) return false;
    while(cur < s.length) {
        char c = s.data[cur];
        int digit_value = 0;
        if(is_between(c, '0', '9')) digit_value = c - '0';
        else if(is_between(c, 'A', 'F')) digit_value = c - 'A' + 0xa;
        else if(is_between(c, 'a', 'f')) digit_value = c - 'a' + 0xa;
        else return false; // invalid character
        result *= 0x10;
        result += digit_value;
        ++cur;
    }
    *out_result = result;
    return true;
}

//#define ASSEMBLER_ASSERT(expr, err_msg) do { if(!(expr)) ret.success = false; ret.error_message = STR((err_msg)) } while(0)
AssemblerResult computer_assemble_and_load_program(Computer *computer, String assembly) {
    AssemblerResult ret = {};
    ret.success = true;

    Arena *arena = arena_create();
    size_t command_count = 0;
    String *commands = split_string(assembly, '\n', &command_count, false, arena);

    uint16_t write_address = COMPUTER_PROGRAM_ENTRY_POINT;
    for(size_t i = 0; i < command_count; ++i) {
        size_t token_count = 0;
        String *command_tokens = split_string(commands[i], ' ', &token_count, false, arena);
        String instruction = command_tokens[0];
        if(strings_are_equal(instruction, STR("mov"))) {
            if(token_count != 3) {
                ret.success = false;
                ret.error_message = STR("Invalid instruction: Expected operands");
            }
            String operand1 = command_tokens[1];
            if(operand1.length != 2 || operand1.data[0] != 'r') {
                ret.success = false;
                ret.error_message = STR("Invalid operand: Expected register");
            }
            if(operand1.data[1] > '6' || operand1.data[1] < '0') {
                ret.success = false;
                ret.error_message = STR("Invalid operand: Register doesn't exist, has to be r0-r6");
            }
            uint8_t opcode = operand1.data[1] - '0';
            computer_write_byte_to_address(computer, opcode, write_address++);
            String operand2 = command_tokens[2];
            uint16_t immediate_val = 0;
            if(!parse_hex_uint16(operand2, &immediate_val)) {
                ret.success = false;
                ret.error_message = STR("Invalid operand: should be a hexadecimal value, ex. 0xabcd");
            }
            computer_write_uint16_to_address(computer, immediate_val, write_address);
            write_address += 2;
       // } else if(strings_are_equal(instruction, STR("str"))) {
       // } else if(strings_are_equal(instruction, STR("ldr"))) {
       // } else if(strings_are_equal(instruction, STR("jmp"))) {
        } else {
            ret.success = false;
            ret.error_message = STR("Invalid instruction");
        }
    }

    arena_free(arena);
    return ret;
}
