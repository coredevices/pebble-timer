// @file menu_window.c
// @brief Main list view for timers
//
// Contains code to display all current timers in a MenuLayer with simple progress bars and a
// "+" button for adding new timers. Modeled after the alarm app
//
// @author Eric D. Phillips
// @date September 13, 2015
// @bugs No known bugs

#include "menu_window.h"
#include "utility.h"

// Menu window user data
typedef struct {
  MenuLayer     *menu_layer;     //< Main MenuLayer to display all timers
} MenuWindowData;


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//


////////////////////////////////////////////////////////////////////////////////////////////////////
// API Interface
//

// Render a menu window
void menu_window_render(Window *window) {
  // check if visible
  if (window != window_stack_get_top_window()) {
    return;
  }

  // do other stuff
}

// Create a menu Window
Window *menu_window_create(void) {
  // create a window
  Window *window = window_create();
  ASSERT(window);
  Layer *window_root = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_root);
  // create user data
  MenuWindowData *user_data = MALLOC(sizeof(MenuWindowData));
  window_set_user_data(window, user_data);
  // create menu layer
  user_data->menu_layer = menu_layer_create(window_bounds);
  ASSERT(user_data->menu_layer);
  //layer_add_child(window_root, menu_layer_get_layer(user_data->menu_layer));
  // return pointer
  // Note: this can never return NULL, as it will assert if malloc fails
  return window;
}

// Destroy a menu Window
void menu_window_destroy(Window *window) {
  // get the window user data
  MenuWindowData *user_data = window_get_user_data(window);
  // destroy layers
  menu_layer_destroy(user_data->menu_layer);
  // free user data and window
  free(user_data);
  window_destroy(window);
}
