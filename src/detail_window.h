/*******************************************************************************
 * FILENAME :        detail_window.h
 *
 * DESCRIPTION :
 *      Display a timer with controls to modify it
 *
 * PUBLIC FUNCTIONS :
 *      DetailWindow    *detail_window_create(
 *                          DetailWindowCallbacks detail_window_callbacks);
 *      void            detail_window_destroy(DetailWindow *detail_window);
 *      void            detail_window_push(DetailWindow *detail_window,
 *                          bool animated);
 *      void            detail_window_pop(DetailWindow *detail_window,
 *                          bool animated);
 *      bool            detail_window_get_topmost_window(DetailWindow
 *                          *detail_window);
 *      void            detail_window_set_countdown_timer(DetailWindow
 *                          *detail_window, CountdownTimer *countdown_timer);
 *      void            detail_window_refresh(DetailWindow *detail_window);
 *      void            detail_window_deep_refresh(DetailWindow *detail_window);
 *      void            detail_window_set_highlight_color(DetailWindow
 *                          *detail_window, GColor color);
 *      bool            detail_window_get_update_needed(DetailWindow
 *                          *detail_window);
 *
 * NOTES :      The actual timer structure definition is not exposed to
 *              prevent direct modification of the structure.
 *
 * AUTHOR :     Eric Phillips        START DATE :    07/11/15
 *
 */

#pragma once

#include <pebble.h>
#include "countdown_timer.h"



/*******************************************************************************
 * CALLBACK DECLARATIONS
 */

/*
 * Callback:    DetailWindowEditTimer
 * ----------------------------------
 * called when the edit button is pressed
 */

typedef void (*DetailWindowEdit)(CountdownTimer *countdown_timer, void *context);



/*
 * Callback:    DetailWindowPlayPause
 * ----------------------------------
 * called when the play/pause button is pressed
 */

typedef void (*DetailWindowPlayPause)(CountdownTimer *countdown_timer, void *context);



/*
 * Callback:    DetailWindowDelete
 * -------------------------------
 * called when the delete buttons is pressed
 */

typedef void (*DetailWindowDelete)(CountdownTimer *countdown_timer, void *context);



/*
 * Structure:   DetailWindowCallbacks
 * ----------------------------------
 * structure containing all DetailWindow callbacks
 */

typedef struct DetailWindowCallbacks {
  DetailWindowEdit edit_timer;
  DetailWindowPlayPause playpause_timer;
  DetailWindowDelete delete_timer;
} DetailWindowCallbacks;



/*******************************************************************************
 * STRUCTURE DECLARATION
 */

/*
 * Structure:   DetailWindow
 * -----------------------
 * main structure containing all data for a DetailWindow
 */

typedef struct DetailWindow DetailWindow;



/*******************************************************************************
 * API FUNCTIONS
 */

/*
 * Function:    detail_window_create
 * -------------------------------
 * creates a new DetailWindow in memory but does not push it into view
 *
 *  detail_window_callbacks: callbacks for communication
 *
 *  returns: a pointer to a new DetailWindow structure
 */

DetailWindow *detail_window_create(DetailWindowCallbacks detail_window_callbacks);



/*
 * Function:    detail_window_destroy
 * ----------------------------------
 * destroys an existing DetailWindow
 *
 *  detail_window: a pointer to the DetailWindow being destroyed
 */

void detail_window_destroy(DetailWindow *detail_window);



/*
 * Function:    detail_window_push
 * -------------------------------
 * push the window onto the stack
 *
 *  detail_window: a pointer to the DetailWindow being pushed
 *  animated: whether to animate the push or not
 */

void detail_window_push(DetailWindow *detail_window, bool animated);



/*
 * Function:    detail_window_pop
 * ------------------------------
 * pop the window off the stack
 *
 *  detail_window: a pointer to the DetailWindow to pop
 *  animated: whether to animate the pop or not
 */

void detail_window_pop(DetailWindow *detail_window, bool animated);



/*
 * Function:    detail_window_get_topmost_window
 * ---------------------------------------------
 * gets whether it is the topmost window or not
 *
 *  detail_window: a pointer to the DetailWindow being checked
 *
 *  returns: a boolean indicating if it is the topmost window
 */

bool detail_window_get_topmost_window(DetailWindow *detail_window);



/*
 * Function:    detail_window_set_countdown_timer
 * ----------------------------------------------
 * sets the CountdownTimer associated with the detail window
 *
 *  detail_window: a pointer to the DetailWindow the timer will be assigned to
 *  countdown_timer: a pointer to the timer being assigned the window
 */

void detail_window_set_countdown_timer(DetailWindow *detail_window,
                                       CountdownTimer *countdown_timer);



/*
 * Function:    detail_window_refresh
 * --------------------------------
 * redraws the detail window
 *
 *  detail_window: a pointer to the DetailWindow to refresh
 */

void detail_window_refresh(DetailWindow *detail_window);



/*
 * Function:    detail_window_deep_refresh
 * ---------------------------------------
 * refreshes all aspects of a DetailWindow that may change
 * including reassigning the ActionBarLayer icons for play and pause
 *
 *  detail_window: a pointer to the DetailWindow to refresh
 */

void detail_window_deep_refresh(DetailWindow *detail_window);



/*
 * Function:    detail_window_set_highlight_color
 * --------------------------------------------
 * sets the over-all color scheme of the window
 *
 *  color: the GColor to set the highlight to
 */

void detail_window_set_highlight_color(DetailWindow *detail_window, GColor color);



/*
 * Function:    detail_window_get_update_needed
 * --------------------------------------------
 * gets whether a refresh is needed for the animation
 * to update
 *
 *  detail_window: a pointer to the DetailWindow to check
 *
 *  returns: a boolean representing whether it needs to be refreshed
 */

bool detail_window_get_update_needed(DetailWindow *detail_window);
