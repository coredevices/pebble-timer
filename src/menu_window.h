/*******************************************************************************
 * FILENAME :        menu_window.h
 *
 * DESCRIPTION :
 *      Create, destroy, and manage a MenuWindow to display
 *      a list of CountdownTimers
 *
 * PUBLIC FUNCTIONS :
 *      MenuWindow  *menu_window_create(MenuWindowCallbacks
 *                      menu_window_callbacks, bool animated);
 *      void        menu_window_destroy(MenuWindow *menu_window);
 *      bool        menu_window_get_topmost_window(MenuWindow *menu_window);
 *      void        menu_window_refresh(MenuWindow *menu_window);
 *      void        menu_window_reload_data(MenuWindow *menu_window);
 *      void        menu_window_set_highlight_color(MenuWindow *menu_window,
 *                      GColor color);
 *
 * AUTHOR :     Eric Phillips        START DATE :    07/10/15
 *
 */

#pragma once

#include <pebble.h>
#include "countdown_timer.h"



/*******************************************************************************
 * CALLBACK DECLARATIONS
 */

/*
 * Callback:    MenuWindowGetTimer
 * -------------------------------
 * gets a pointer to a specific CountdownTimer
 */

typedef CountdownTimer* (*MenuWindowGetTimer)(uint8_t index, void *context);



/*
 * Callback:    MenuWindowGetTimerCount
 * ------------------------------------
 * gets the number of CountdownTimers to be displayed in the menu
 */

typedef uint8_t (*MenuWindowGetTimerCount)(void *context);

/*
 * Callback:    MenuWindowGetSortMode
 * ----------------------------------
 * gets the current timer sorting mode for the menu list
 *
 * returns:
 *   0 = created at (added order)
 *   1 = duration (shortest to longest)
 */

typedef uint8_t (*MenuWindowGetSortMode)(void *context);



/*
 * Callback:    MenuWindowClickCallback
 * ------------------------------------
 * called when a timer is clicked on in the menu layer
 */

typedef void (*MenuWindowClickCallback)(uint8_t index, void *context);



/*
 * Structure:   MenuWindowCallbacks
 * --------------------------------
 * structure containing all MenuWindow callbacks
 */

typedef struct MenuWindowCallbacks {
  MenuWindowGetTimer get_timer;
  MenuWindowGetTimerCount get_timer_count;
  MenuWindowGetSortMode get_sort_mode;
  MenuWindowClickCallback clicked;
} MenuWindowCallbacks;



/*******************************************************************************
 * STRUCTURE DECLARATION
 */

/*
 * Structure:   MenuWindow
 * -----------------------
 * main structure containing all data for a MenuWindow
 */

typedef struct MenuWindow MenuWindow;



/*******************************************************************************
 * API FUNCTIONS
 */

/*
 * Function:    menu_window_create
 * -------------------------------
 * creates a new MenuWindow on the stack
 *
 *  menu_window_callbacks: callbacks for communication
 *  animated: whether to push the new window with animation
 *
 *  returns: a pointer to a new MenuWindow structure
 */

MenuWindow *menu_window_create(MenuWindowCallbacks menu_window_callbacks, bool animated);



/*
 * Function:    menu_window_destroy
 * --------------------------------
 * destroys an existing MenuWindow
 *
 *  menu_window: a pointer to the MenuWindow being destroyed
 */

void menu_window_destroy(MenuWindow *menu_window);



/*
 * Function:    menu_window_get_topmost_window
 * -------------------------------------------
 * gets whether it is the topmost window or not
 *
 *  menu_window: a pointer to the MenuWindow being checked
 *
 *  returns: a boolean indicating if it is the topmost window
 */

bool menu_window_get_topmost_window(MenuWindow *menu_window);



/*
 * Function:    menu_window_refresh
 * --------------------------------
 * redraws the menu window
 *
 *  menu_window: a pointer to the MenuWindow being refreshed
 */

void menu_window_refresh(MenuWindow *menu_window);



/*
 * Function:    menu_window_reload_data
 * ------------------------------------
 * reload the menu layer's data
 *
 *  menu_window: a pointer to the window for which to reload the data
 */

void menu_window_reload_data(MenuWindow *menu_window);



/*
 * Function:    menu_window_set_highlight_color
 * --------------------------------------------
 * sets the over-all color scheme of the window
 *
 *  color: the GColor to set the highlight to
 */

void menu_window_set_highlight_color(MenuWindow *menu_window, GColor color);
