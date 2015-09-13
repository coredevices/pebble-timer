// @file main.c
// @brief Entry point and main logic
//
// Contains main logic and execution loop. Creates and destroys all windows.
//
// @author Eric D. Phillips
// @date September 13, 2015
// @bugs No known bugs

#include "menu_window.h"

// Main data object for application
static struct {
  Window      *menu_window;     //< Pointer to window for menu window
} main_app_data;


////////////////////////////////////////////////////////////////////////////////////////////////////
// Loading and Unloading
//

// Initialize
static void prv_initialize(void) {
  // create menu window
  main_app_data.menu_window = menu_window_create();
  window_stack_push(main_app_data.menu_window, true);
}

// Terminate
static void prv_terminate(void) {
  // destroy windows
  menu_window_destroy(main_app_data.menu_window);
}

// Entry point
int main(void) {
  prv_initialize();
  app_event_loop();
  prv_terminate();
}
