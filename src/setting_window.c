/*******************************************************************************
 * FILENAME :        setting_window.c
 *
 * DESCRIPTION :
 *      Setting screen to select a time duration
 *
 * PUBLIC FUNCTIONS :
 *      SettingWindow   *setting_window_create(SettingWindowCallbacks
 *                          setting_window_callbacks);
 *      void            setting_window_destroy(SettingWindow *setting_window);
 *      void            setting_window_push(SettingWindow *setting_window,
 *                          bool animated);
 *      void            setting_window_pop(SettingWindow *setting_window,
 *                          bool animated);
 *      bool            setting_window_get_topmost_window(SettingWindow
 *                          *setting_window);
 *      void            setting_window_set_timer(SettingWindow *setting_window,
 *                          CountdownTimer *countdown_timer);
 *      CountdownTimer  *setting_window_get_timer(SettingWindow
 *                          *setting_window);
 *      void            setting_window_set_highlight_color(SettingWindow
 *                          *setting_window, GColor color);
 *
 * AUTHOR :         Eric Phillips        START DATE :    07/12/15
 *
 */

#include <pebble.h>
#include "setting_window.h"
#include "countdown_timer.h"
#include "selection_layer.h"

#define MSEC_IN_SEC 1000
#define MSEC_IN_MIN 60000
#define MSEC_IN_HR 3600000
#define MIN_IN_HR 60
#define HR_IN_DAY 24

#define REPEATING_CLICK_THRESHOLD 10

#define TIMER_MINIMUM_DURATION 5000 // milliseconds
#define TIMELINE_MINIMUM_DURATION 900000 // milliseconds



/*******************************************************************************
 * STRUCTURE DEFINITION
 */

/*
 * the structure of a SettingWindow
 */

struct SettingWindow {
  Window          *window;            //< main window
  TextLayer       *main_text;         //< title text at top of screen
  TextLayer       *sub_text;          //< sub text at bottom for messages
  Layer           *selection;         //< SelectionLayer for input
  GColor          highlight_color;    //< color for selection highlights
  StatusBarLayer  *status;            //< status bar for Basalt
  SettingWindowCallbacks callbacks;   //< callbacks

  CountdownTimer  *countdown_timer;   //< timer associated being set
  int32_t         field_values[3];    //< values of selection fields
  char            field_buffs[3][3];  //< buffers to draw field contents
  int8_t          field_selection;    //< index of selected field
};



/*******************************************************************************
 * PRIVATE FUNCTIONS
 */

/*
 * update the sub text
 *
 * updates whether the end time is shown in the setting sub text
 */

static void update_sub_text(SettingWindow *setting_window) {
  int64_t duration = (int64_t)setting_window->field_values[0] * MSEC_IN_HR +
    (int64_t)setting_window->field_values[1] * MSEC_IN_MIN +
    (int64_t)setting_window->field_values[2] * MSEC_IN_SEC;
  // check duration
  if (duration < TIMER_MINIMUM_DURATION) {
    text_layer_set_text(setting_window->sub_text, "");
    layer_set_hidden(text_layer_get_layer(setting_window->sub_text), false);
    return;
  } else if (duration < TIMELINE_MINIMUM_DURATION) {
    layer_set_hidden(text_layer_get_layer(setting_window->sub_text), true);
    return;
  } else {
    layer_set_hidden(text_layer_get_layer(setting_window->sub_text), false);
  }

  // format into time parts
  time_t end = ((int64_t)time(NULL) * 1000 + (int64_t)time_ms(NULL, NULL) + duration) / 1000;
  static char buff[] = "End: 00:00 AM";
  struct tm *tick_time = localtime(&end);
  if (clock_is_24h_style()) {
    strftime(buff, sizeof(buff), "End: %k:%M", tick_time);
  } else {
    strftime(buff, sizeof(buff), "End: %l:%M %p", tick_time);
  }
  // set text
  text_layer_set_text(setting_window->sub_text, buff);
}


/*******************************************************************************
 * CALLBACKS
 */

/*
 * selection layer get text callback
 */

static char* selection_handle_get_text(unsigned index, void *context) {
  SettingWindow *setting_window = (SettingWindow*)context;
  snprintf(setting_window->field_buffs[index], sizeof(setting_window->field_buffs[0]), "%02d",
    (int)setting_window->field_values[index]);
  return setting_window->field_buffs[index];
}



/*
 * selection layer complete callback
 */

static void selection_handle_complete(void *context) {
  SettingWindow *setting_window = (SettingWindow*)context;
  int64_t duration = (int64_t)setting_window->field_values[0] * MSEC_IN_HR +
    (int64_t)setting_window->field_values[1] * MSEC_IN_MIN +
    (int64_t)setting_window->field_values[2] * MSEC_IN_SEC;
  // call complete callback
  setting_window->callbacks.setting_complete(duration, setting_window);
}



/*
 * selection layer increment up callback
 */

static void selection_handle_inc(unsigned index, uint8_t clicks, void *context) {
  SettingWindow *setting_window = (SettingWindow*)context;
  setting_window->field_values[index] += (clicks > REPEATING_CLICK_THRESHOLD) ? 2 : 1;
  int8_t max_value = (index == 0) ? HR_IN_DAY : MIN_IN_HR;
  if (setting_window->field_values[index] >= max_value) {
    setting_window->field_values[index] -= max_value;
  }
  // update text
  update_sub_text(setting_window);
}



/*
 * selection layer decrement callback
 */

static void selection_handle_dec(unsigned index, uint8_t clicks, void *context) {
  SettingWindow *setting_window = (SettingWindow*)context;
  setting_window->field_values[index] -= (clicks > REPEATING_CLICK_THRESHOLD) ? 2 : 1;
  int8_t max_value = (index == 0) ? HR_IN_DAY : MIN_IN_HR;
  if (setting_window->field_values[index] < 0) {
    setting_window->field_values[index] += max_value;
  }
  // update text
  update_sub_text(setting_window);
}



/*******************************************************************************
 * API FUNCTIONS
 */

/*
 * create a new SettingWindow and return a pointer to it
 * this includes creating all its children layers but
 * does not push it onto the window stack
 */

SettingWindow *setting_window_create(SettingWindowCallbacks setting_window_callbacks) {
  SettingWindow *setting_window = (SettingWindow*)malloc(sizeof(SettingWindow));
  if (setting_window != NULL) {

    *setting_window = (SettingWindow) { .callbacks = setting_window_callbacks };

    return setting_window;
  }
  return NULL;
}



/*
 * destroy a previously created SettingWindow
 */

void setting_window_destroy(SettingWindow *setting_window) {
  if (setting_window != NULL) {
    free(setting_window);
    setting_window = NULL;
    return;
  }
}


static void prv_window_load(Window* window){
  SettingWindow *setting_window = window_get_user_data(window);
  // get window parameters
  Layer *root = window_get_root_layer(setting_window->window);
  GRect bounds = layer_get_frame(root);
  // main text
  setting_window->main_text = text_layer_create(GRect(0, 30, bounds.size.w, 40));
  text_layer_set_text(setting_window->main_text, "Set Timer");
  text_layer_set_font(setting_window->main_text,
    fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(setting_window->main_text, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(setting_window->main_text));
  // sub text
  setting_window->sub_text = text_layer_create(GRect(1, 125, bounds.size.w, 40));
  text_layer_set_text_alignment(setting_window->sub_text, GTextAlignmentCenter);
  text_layer_set_font(setting_window->sub_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(root, text_layer_get_layer(setting_window->sub_text));
  // create selection layer
  uint8_t num_cells = 3; // hours, minutes, seconds
#ifdef PBL_ROUND
  setting_window->selection = selection_layer_create(GRect(26, 75, bounds.size.w-52, 34), num_cells);
#else
  setting_window->selection = selection_layer_create(GRect(8, 75, bounds.size.w-16, 34), num_cells);
#endif
  for (int i = 0; i < num_cells; i++) {
    selection_layer_set_cell_width(setting_window->selection, i, 40);
  }
  selection_layer_set_cell_padding(setting_window->selection, 4);
  selection_layer_set_active_bg_color(setting_window->selection, setting_window->highlight_color);
  selection_layer_set_inactive_bg_color(setting_window->selection, GColorDarkGray);
  selection_layer_set_click_config_onto_window(
    setting_window->selection, setting_window->window);
  selection_layer_set_callbacks(setting_window->selection, setting_window,
    (SelectionLayerCallbacks) {
      .get_cell_text = selection_handle_get_text,
      .complete = selection_handle_complete,
      .increment = selection_handle_inc,
      .decrement = selection_handle_dec,
    });
  layer_add_child(window_get_root_layer(setting_window->window), setting_window->selection);

  // create status bar
  setting_window->status = status_bar_layer_create();
  status_bar_layer_set_colors(setting_window->status, GColorClear, GColorBlack);
  layer_add_child(root, status_bar_layer_get_layer(setting_window->status));

  update_sub_text(setting_window);
}

static void prv_window_unload(Window* window){
  SettingWindow *setting_window = window_get_user_data(window);
  status_bar_layer_destroy(setting_window->status);
  selection_layer_destroy(setting_window->selection);
  text_layer_destroy(setting_window->sub_text);
  text_layer_destroy(setting_window->main_text);
  window_destroy(setting_window->window);
  setting_window->window = NULL;
}



/*
 * push the window onto the stack
 */
void setting_window_push(SettingWindow *setting_window, bool animated) {
  if (setting_window->window == NULL) {
    setting_window->window = window_create();
    window_set_user_data(setting_window->window, setting_window);
    window_set_window_handlers(setting_window->window,
      (WindowHandlers){
        .load = prv_window_load,
        .unload = prv_window_unload
      });
  }
  if (setting_window->window) {
    window_stack_push(setting_window->window, animated);
  }
}



/*
 * pop the window off the stack
 */

void setting_window_pop(SettingWindow *setting_window, bool animated) {
  if (setting_window->window) {
    window_stack_remove(setting_window->window, animated);
  }
}



/*
 * gets whether it is the topmost window on the stack
 */

bool setting_window_get_topmost_window(SettingWindow *setting_window) {
  return window_stack_get_top_window() == setting_window->window;
}



/*
 * sets the CountdownTimer associated with the SettingWindow
 *
 * used to identify if this was an update to an existing timer or a new one
 */

void setting_window_set_timer(SettingWindow *setting_window, CountdownTimer *countdown_timer) {
  setting_window->countdown_timer = countdown_timer;
  // set selection values if a timer was passed in
  int64_t duration = 0;
  if (setting_window->countdown_timer) {
    duration = countdown_timer_get_duration(setting_window->countdown_timer);
  }
  setting_window->field_values[0] = duration / MSEC_IN_HR;
  setting_window->field_values[1] = duration % MSEC_IN_HR / MSEC_IN_MIN;
  setting_window->field_values[2] = duration % MSEC_IN_MIN / MSEC_IN_SEC;
  // change text
  if (setting_window->window) {
    update_sub_text(setting_window);
  }
}



/*
 * gets the CountdownTimer associated with this SettingWindow
 *
 * used to identify if this was an update to an existing timer or a new one
 */

CountdownTimer *setting_window_get_timer(SettingWindow *setting_window) {
  return setting_window->countdown_timer;
}



/*
 * set highlight color of this window
 * this is the overall color scheme used
 */

void setting_window_set_highlight_color(SettingWindow *setting_window, GColor color) {
  setting_window->highlight_color = color;
}
