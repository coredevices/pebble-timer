//! @file menu_window.h
//! @brief Main list view for timers
//!
//! Contains code to display all current timers in a MenuLayer with simple progress bars and a
//! "+" button for adding new timers. Modeled after the alarm app
//!
//! @author Eric D. Phillips
//! @date September 13, 2015
//! @bugs No known bugs

#include <pebble.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
// API Interface
//

//! Render a menu window
//! @param window A pointer to the window to render
void menu_window_render(Window *window);

//! Create a menu Window
//! @return A pointer to the window
Window *menu_window_create(void);

//! Destroy a menu Window
//! @param window A pointer to the window to destroy
void menu_window_destroy(Window *window);
