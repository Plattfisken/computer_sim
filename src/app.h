#include <stdbool.h>
#include "computer_sim.h"

#ifndef APP_H
#define APP_H

typedef struct {
    Computer computer;
    AssemblerResult last_assemble_status;

    bool show_editor;
    bool show_assembler_error;
    bool left;
    bool right;
    bool up;
    bool down;
} AppState;

#endif // APP_H
