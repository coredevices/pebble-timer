/*******************************************************************************
 * FILENAME :        popup_window.h
 *
 * DESCRIPTION :
 *      Displays a pop-up window with a PDC or image, some text, and an optional
 *      ActionBar. It also can auto-close after a customizable length of time.
 *      This could be used in any project.
 *
 * PUBLIC FUNCTIONS :
 *      PopupWindow     *popup_window_create(void);
 *      void            popup_window_destroy(PopupWindow *popup_window);
 *      void            popup_window_push(PopupWindow *popup_window,
 *                          bool animated);
 *      void            popup_window_pop(PopupWindow *popup_window,
 *                          bool animated);
 *      bool            popup_window_get_topmost_window(PopupWindow
 *                      *popup_window);
 *      void            popup_window_set_auto_close_duration(PopupWindow
 *                          *popup_window, int64_t duration);
 *      void            popup_window_set_countdown_timer(PopupWindow
 *                          *popup_window, CountdownTimer *countdown_timer);
 *      void            popup_window_refresh(PopupWindow *popup_window);
 *      void            popup_window_set_pdc(PopupWindow *popup_window,
 *                          uint32_t resource_id, bool endless);
 *      int64_t         popup_window_get_pdc_duration(PopupWindow
 *                          *popup_window);
 *      void            popup_window_set_image(PopupWindow *popup_window,
 *                          uint32_t resource_id);
 *      void            popup_window_set_title(PopupWindow *popup_window,
 *                          const char *text);
 *      void            popup_window_set_highlight_color(PopupWindow
 *                          *popup_window, GColor color);
 *      void            popup_window_add_action_bar(PopupWindow *popup_window);
 *      void            popup_window_remove_action_bar(PopupWindow
 *                          *popup_window);
 *      void            popup_window_set_action_bar_callbacks(PopupWindow
 *                          *popup_window, PopupWindowCallbacks callbacks);
 *      void            popup_window_remove_action_bar_callbacks(PopupWindow
 *                          *popup_window);
 *
 * AUTHOR :     Eric Phillips        START DATE :    07/13/15
 *
 */

#pragma once

#include <pebble.h>
#include "countdown_timer.h"



/*******************************************************************************
 * CALLBACK DECLARATIONS
 */

/*
 * Callback:    PopupWindowUpClick
 * -------------------------------
 * called when the UP button is pressed
 */

typedef void (*PopupWindowUpClick)(CountdownTimer *countdown_timer, void *context);



/*
 * Callback:    PopupWindowSelectClick
 * -----------------------------------
 * called when the SELECT button is pressed
 */

typedef void (*PopupWindowSelectClick)(void *context);



/*
 * Callback:    PopupWindowDownClick
 * ---------------------------------
 * called when the DOWN button is pressed
 */

typedef void (*PopupWindowDownClick)(void *context);



/*
 * Structure:   PopupWindowCallbacks
 * ---------------------------------
 * structure containing all PopupWindow callbacks
 */

typedef struct PopupWindowCallbacks {
  PopupWindowUpClick up_click;
  PopupWindowSelectClick select_click;
  PopupWindowDownClick down_click;
} PopupWindowCallbacks;



/*******************************************************************************
 * STRUCTURE DECLARATION
 */

/*
 * Structure:   PopupWindow
 * ------------------------
 * main structure containing all data for a PopupWindow
 */

typedef struct PopupWindow PopupWindow;



/*******************************************************************************
 * API FUNCTIONS
 */

/*
 * Function:    popup_window_create
 * --------------------------------
 * creates a new PopupWindow in memory but does not push it into view
 *
 *  returns: a pointer to a new PopupWindow structure
 */

PopupWindow *popup_window_create(void);



/*
 * Function:    popup_window_destroy
 * ---------------------------------
 * destroys an existing PopupWindow
 *
 *  popup_window: a pointer to the PopupWindow being destroyed
 */

void popup_window_destroy(PopupWindow *popup_window);



/*
 * Function:    popup_window_push
 * ------------------------------
 * push the window onto the stack
 *
 *  popup_window: a pointer to the PopupWindow being pushed
 *  animated: whether to animate the push or not
 */

void popup_window_push(PopupWindow *popup_window, bool animated);



/*
 * Function:    popup_window_pop
 * -----------------------------
 * pop the window off the stack
 *
 *  popup_window: a pointer to the PopupWindow to pop
 *  animated: whether to animate the pop or not
 */

void popup_window_pop(PopupWindow *popup_window, bool animated);



/*
 * Function:    popup_window_get_topmost_window
 * --------------------------------------------
 * gets whether it is the topmost window or not
 *
 *  popup_window: a pointer to the PopupWindow being checked
 *
 *  returns: a boolean indicating if it is the topmost window
 */

bool popup_window_get_topmost_window(PopupWindow *popup_window);



/*
 * Function:    popup_window_set_auto_close_duration
 * -------------------------------------------------
 * optionally sets the time after which the PopupWindow
 * will automatically pop itself if set to 0, never pop
 *
 *  popup_window: a pointer to the PopupWindow that will be closed
 *  duration: an int64_t for the millisecond duration after which to pop
 */

void popup_window_set_auto_close_duration(PopupWindow *popup_window, int64_t duration);

/*
 * Function:    popup_window_set_vibes
 * -------------------------------------------------
 * sets up an app_timer callback to run multiple instances
 * of a vibration pattern
 */

void popup_window_set_vibes();


/*
 * Function:    popup_window_set_countdown_timer
 * ----------------------------------------------
 * sets the CountdownTimer associated with the PopupWindow
 *
 *  popup_window: a pointer to the PopupWindow the timer will be assigned to
 *  countdown_timer: a pointer to the CountdownTimer being assigned the window
 */

void popup_window_set_countdown_timer(PopupWindow *popup_window, CountdownTimer *countdown_timer);



/*
 * Function:    popup_window_refresh
 * ---------------------------------
 * redraws the PopupWindow and steps any ongoing PDC animations
 *
 *  popup_window: a pointer to the PopupWindow that will be refreshed
 */

void popup_window_refresh(PopupWindow *popup_window);



/*
 * Function:    popup_window_set_pdc
 * ---------------------------------
 * sets the animating PDC displayed in the center of the screen
 * currently only supported on Basalt
 *
 *  popup_window: a pointer to the PopupWindow the PDC is set to
 *  resource_id: a RESOURCE_ID_XXX to the PDC to be displayed
 */

void popup_window_set_pdc(PopupWindow *popup_window, uint32_t resource_id, bool endless);



/*
 * Function:    popup_window_get_pdc_duration
 * ------------------------------------------
 * gets the duration of the currently assigned PDC
 *
 *  popup_window: a pointer to the PopupWindow to check the PDC of
 *
 *  returns: the duration in milliseconds of the PDC
 */

int64_t popup_window_get_pdc_duration(PopupWindow *popup_window);

/*
 * Function:    popup_window_set_image
 * -----------------------------------
 * sets the image displayed in the center of the PopupWindow
 * used for Aplite where PDCs are not yet implemented
 *
 *  popup_window: a pointer to the PopupWindow the image is set to
 *  resource_id: a RESOURCE_ID_XXX to the image to be displayed
 */

void popup_window_set_image(PopupWindow *popup_window, uint32_t resource_id);

/*
 * Function:    popup_window_set_title
 * -----------------------------------
 * set the title of a PopupWindow
 * which is the text at the bottom beneath the PDC/image
 *
 *  popup_window: a pointer to the PopupWindow to set the title of
 *  text: the text to set the title to
 */

void popup_window_set_title(PopupWindow *popup_window, const char *text);



/*
 * Function:    popup_window_set_highlight_color
 * ---------------------------------------------
 * sets the over-all color scheme of the PopupWindow
 *
 *  color: the GColor to set the highlight to
 */

void popup_window_set_highlight_color(PopupWindow *popup_window, GColor color);



/*
 * Function:    popup_window_add_action_bar
 * ----------------------------------------
 * adds an ActionBar to the PopupWindow, enabling user feedback
 * this automatically checks for an existing ActionBarLayer and
 * only adds one if none are visible
 *
 *  popup_window: a pointer to the PopupWindow to add the ActionBar to
 */

void popup_window_add_action_bar(PopupWindow *popup_window);



/*
 * Function:    popup_window_remove_action_bar
 * -------------------------------------------
 * removes the ActionBar from the PopupWindow
 * automatically checks for no action bar and is a no-op if none are showing
 *
 *  popup_window: the PopupWindow to remove the action bar from
 */

void popup_window_remove_action_bar(PopupWindow *popup_window);



/*
 * Function:    popup_window_set_action_bar_callbacks
 * --------------------------------------------------
 * sets the callbacks for the ActionBar if there is one added to the PopupWindow
 *
 *  popup_window: a pointer to the PopupWindow to add the callbacks to
 *  callbacks: the callbacks to assign the ActionBar in the PopupWindow
 */

void popup_window_set_action_bar_callbacks(PopupWindow *popup_window,
                                           PopupWindowCallbacks callbacks);



/*
 * Function:    popup_window_remove_action_bar_callbacks
 * -----------------------------------------------------
 * removes the callbacks for the ActionBar
 * not essential other than for good practice
 *
 *  popup_window: a pointer to the PopupWindow to remove the callbacks from
 */

void popup_window_remove_action_bar_callbacks(PopupWindow *popup_window);
