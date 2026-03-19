#include "app.h"
#include <raylib.h>
#include "../third_party/useful_things.h"

#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

typedef struct UiLayoutState UiLayoutState;
// Call first time loading the library
typedef UiLayoutState *(init_ui_engine_func)(UT_Arena *arena, Font font);

// Call after every reload
typedef void (reload_ui_engine_func)(UiLayoutState *my_state);

// Call when you want to render (every frame)
typedef void (render_ui_layout_func)(AppState *app_state, UiLayoutState *my_state);

//init_ui_engine_func init_ui_engine;
//render_ui_layout_func render_ui_layout;

#endif // UI_LAYOUT_H
