// @file setting_window.h
// @brief Allows user to enter a timer duration
//
// Displays a series of selectable number fields, like the alarms app, for the user to enter
// a timer duration into. Used to create new timers and edit existing ones.
//
// @author Eric D. Phillips
// @date September 19, 2015
// @bugs No known bugs

#include "setting_window.h"
#include "countdown_timer.h"
#include "utility.h"

// Constants


// Setting window user data
typedef struct {

} SettingWindowData;


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//


////////////////////////////////////////////////////////////////////////////////////////////////////
// API Interface
//

// Render a setting window
void setting_window_render(Window *window) {
  // check if visible
  if (window != window_stack_get_top_window()) {
    return;
  }

  // do other stuff
}

// Create a setting Window
Window *setting_window_create(void) {
  // create a window
  Window *window = window_create();
  ASSERT(window);
  Layer *window_root = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_root);
  // create user data
  SettingWindowData *user_data = MALLOC(sizeof(SettingWindowData));
  window_set_user_data(window, user_data);
  // TODO: Create layers

  // return pointer
  // Note: this can never return NULL, as it will assert if malloc fails
  return window;
}

// Destroy a setting Window
void setting_window_destroy(Window *window) {
  // get the window user data
  SettingWindowData *user_data = window_get_user_data(window);
  // TODO: Destroy layers
  // free user data and window
  free(user_data);
  window_destroy(window);
}
