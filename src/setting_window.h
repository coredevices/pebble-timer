//! @file setting_window.h
//! @brief Allows user to enter a timer duration
//!
//! Displays a series of selectable number fields, like the alarms app, for the user to enter
//! a timer duration into. Used to create new timers and edit existing ones.
//!
//! @author Eric D. Phillips
//! @date September 19, 2015
//! @bugs No known bugs

#include <pebble.h>


////////////////////////////////////////////////////////////////////////////////////////////////////
// API Interface
//

//! Render a setting Window
//! @param window A pointer to the window to render
void setting_window_render(Window *window);

//! Create a setting Window
//! @return A pointer to the window
Window *setting_window_create(void);

//! Destroy a setting Window
//! @param window A pointer to the window to destroy
void setting_window_destroy(Window *window);
