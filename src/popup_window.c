/*******************************************************************************
 * FILENAME :        popup_window.c
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

#include <pebble.h>
#include "popup_window.h"
#include "countdown_timer.h"

/*******************************************************************************
 * CONSTANTS
 */

#define NUM_VIBE_INTERVALS 6

/*******************************************************************************
 * MAIN LOCAL VARIABLES
 */

static AppTimer *s_app_timer = NULL;


/*******************************************************************************
 * STRUCTURE DECLARATION
 */

/*
 * the structure of a PopupWindow
 */

struct PopupWindow {
  Window          *window;        //< main window
  Layer           *layer;         //< drawing layer for PDC or image
  TextLayer       *text;          //< displays title text
  ActionBarLayer  *action;        //< optional action bar for dialogs
  PopupWindowCallbacks    callbacks;     //< callbacks for optional ActionBar
  GBitmap *snooze_icon,   *stop_icon;    //< icons for ActionBar

#ifndef PBL_PLATFORM_APLITE
  GDrawCommandSequence    *draw_sequence;     //< draw command sequence
  GDrawCommandFrame       *draw_frame;        //< current draw command
  uint32_t frame_count;           //< total number of frames in draw sequence
  uint32_t frame_index;           //< index of current frame (for endless)
  bool     endless;               //< if PDC is endless, having no duration
#else
  GBitmap  *image;                //< alternate image for Aplite
#endif

  CountdownTimer  *countdown_timer;    //< timer associated with PopupWindow
  int64_t     set_time, close_time;    //< time opened and time to close
  bool            action_visible;      //< whether the ActionBar is visible

  GColor highlight_color;
  const char* title;
  bool has_action_bar;
  uint32_t pdc_resource;
};



/*******************************************************************************
 * PRIVATE FUNCTIONS
 */

/*
 * layer update proc
 * PDC/image is drawn on here
 */

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // get DetailWindow pointer from layer data
  Window* window = layer_get_window(layer);
  PopupWindow *popup_window = window_get_user_data(window);
  // draw current PDC frame or image
#ifndef PBL_PLATFORM_APLITE
  if (popup_window->draw_sequence != NULL) {
    gdraw_command_frame_draw(ctx, popup_window->draw_sequence,
      popup_window->draw_frame, GPointZero);
  }
#else
  if (popup_window->image != NULL) {
    graphics_draw_bitmap_in_rect(ctx, popup_window->image, gbitmap_get_bounds(popup_window->image));
  }
#endif
}



/*
 * resize the layers based on PDC or image size
 * this centers the layers in the window, accounting for the potential
 * ActionBarLayer
 */

static void layers_center_in_window(PopupWindow *popup_window) {
#ifdef PBL_ROUND
  int16_t horiz_off = 0;
#else
  int16_t horiz_off = ACTION_BAR_WIDTH;
#endif
#ifndef PBL_PLATFORM_APLITE
  // change layer size based on PDC size, to center PDC
  GSize pdc_frame = gdraw_command_sequence_get_bounds_size(popup_window->draw_sequence);
  GRect window_frame = layer_get_frame(window_get_root_layer(popup_window->window));
  GRect layer_frame = GRect(0, 0, pdc_frame.w, pdc_frame.h);
  if (popup_window->action_visible) {
    layer_frame.origin.x = (window_frame.size.w - horiz_off) / 2 - pdc_frame.w / 2;
  } else {
    layer_frame.origin.x = window_frame.size.w / 2 - pdc_frame.w / 2;
  }
  layer_frame.origin.y = window_frame.size.h / 2 - pdc_frame.h / 2;
  layer_set_frame(popup_window->layer, layer_frame);
#else
  // change layer size based on image size, to center image
  GRect image_frame = gbitmap_get_bounds(popup_window->image);
  GRect window_frame = layer_get_frame(window_get_root_layer(popup_window->window));
  GRect layer_frame = image_frame;
  if (popup_window->action_visible) {
    layer_frame.origin.x = (window_frame.size.w - horiz_off) / 2 - image_frame.size.w / 2;
  } else {
    layer_frame.origin.x = window_frame.size.w / 2 - image_frame.size.w / 2;
  }
  layer_frame.origin.y = window_frame.size.h / 2 - image_frame.size.h / 2 - 7;
  layer_set_frame(popup_window->layer, layer_frame);
#endif

  // center the text layer
  GRect text_frame = layer_get_frame(text_layer_get_layer(popup_window->text));
  text_frame.size.w = layer_get_bounds(window_get_root_layer(popup_window->window)).size.w -
    ((popup_window->action_visible) ? horiz_off : 0);
  layer_set_frame(text_layer_get_layer(popup_window->text), text_frame);
  text_frame.origin.x = text_frame.origin.y = 0;
  layer_set_bounds(text_layer_get_layer(popup_window->text), text_frame);
}



 /*******************************************************************************
 * CALLBACKS
 */

 /*
 * Vibe callback
 *
 * Timer finished vibration split into separate intervals
 */
static void app_timer_vibe_callback(void *data) {
  int num_vibes_left = (int)data;
  if (num_vibes_left != 0) {
    // start vibration
    static const uint32_t vibe_seg[] = {300, 200, 300, 200, 300};
    const VibePattern pat_vibe = {
      .durations = vibe_seg,
      .num_segments = ARRAY_LENGTH(vibe_seg),
    };
    vibes_enqueue_custom_pattern(pat_vibe);

    --num_vibes_left;
    s_app_timer = app_timer_register(2300, app_timer_vibe_callback, (void*)num_vibes_left);
  }
}



/*
 * UP click handler callback
 *
 * snoozes the vibrating timer
 */

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  PopupWindow *popup_window = (PopupWindow*)context;
  if (popup_window->callbacks.up_click == NULL) {
    return;
  }
  return popup_window->callbacks.up_click(popup_window->countdown_timer, context);
}



/*
 * SELECT click handler callback
 *
 * nothing yet... here for completeness
 */

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  PopupWindow *popup_window = (PopupWindow*)context;
  if (popup_window->callbacks.select_click == NULL) {
    return;
  }
  return popup_window->callbacks.select_click(context);
}



/*
 * DOWN click handler callback
 *
 * stops the vibrating timer
 */

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  PopupWindow *popup_window = (PopupWindow*)context;
  if (popup_window->callbacks.down_click == NULL) {
    return;
  }
  return popup_window->callbacks.down_click(context);
}



/*
 * click configuration provider
 */

static void click_config_provider(void *context) {
  window_set_click_context(BUTTON_ID_UP, context);
  window_set_click_context(BUTTON_ID_SELECT, context);
  window_set_click_context(BUTTON_ID_DOWN, context);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void prv_window_load(Window* window){
  PopupWindow *popup_window = window_get_user_data(window);
  // get window parameters
  popup_window->stop_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISMISS);
  popup_window->snooze_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SNOOZE);

  // get window parameters
  Layer *root = window_get_root_layer(popup_window->window);
  GRect bounds = layer_get_frame(root);
  // create layer
  popup_window->layer = layer_create(bounds);
  layer_set_clips(popup_window->layer, false);
  layer_set_update_proc(popup_window->layer, layer_update_proc);
  layer_add_child(root, popup_window->layer);
  // text
#ifndef PBL_PLATFORM_APLITE
  const int text_layer_origin_y = 125;
#else
  const int text_layer_origin_y = 110;
#endif
  popup_window->text = text_layer_create(GRect(0, text_layer_origin_y, bounds.size.w, 36));
  text_layer_set_font(popup_window->text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(popup_window->text, GTextAlignmentCenter);
  text_layer_set_background_color(popup_window->text, GColorClear);
  text_layer_set_text(popup_window->text, popup_window->title);
  layer_add_child(root, text_layer_get_layer(popup_window->text));
  // create action bar
  popup_window->action = action_bar_layer_create();
  action_bar_layer_set_context(popup_window->action, popup_window);
  action_bar_layer_set_click_config_provider(popup_window->action, click_config_provider);
  action_bar_layer_set_icon(popup_window->action, BUTTON_ID_UP, popup_window->snooze_icon);
  action_bar_layer_set_icon(popup_window->action, BUTTON_ID_DOWN, popup_window->stop_icon);

  if (popup_window->action_visible) {
    action_bar_layer_add_to_window(popup_window->action, popup_window->window);
  }

  // re-center layers
  layers_center_in_window(popup_window);

  window_set_background_color(popup_window->window, popup_window->highlight_color);
}

static void prv_window_unload(Window* window){
  PopupWindow *popup_window = window_get_user_data(window);
  action_bar_layer_destroy(popup_window->action);
  text_layer_destroy(popup_window->text);
  layer_destroy(popup_window->layer);
  window_destroy(popup_window->window);
  gbitmap_destroy(popup_window->snooze_icon);
  gbitmap_destroy(popup_window->stop_icon);
#ifndef PBL_PLATFORM_APLITE
  if (popup_window->draw_sequence != NULL) {
    gdraw_command_sequence_destroy(popup_window->draw_sequence);
    popup_window->draw_sequence = NULL;
  }
#else
  if (popup_window->image != NULL) {
    gbitmap_destroy(popup_window->image);
    popup_window->image = NULL;
  }
#endif
  popup_window->window = NULL;

  app_timer_cancel(s_app_timer);
  s_app_timer = NULL;
  vibes_cancel();
}



/*******************************************************************************
 * API FUNCTIONS
 */

 

/*
 * create a new PopupWindow and return a pointer to it
 * this includes creating all its children layers but
 * does not push it onto the window stack
 */

PopupWindow *popup_window_create(void) {
  PopupWindow *popup_window = (PopupWindow*)malloc(sizeof(PopupWindow));

  // error handling
  if (popup_window == NULL) {
    // error handling
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create PopupWindow");
    return NULL;
  }

  // zero some values
  popup_window->countdown_timer = NULL;
  popup_window->action_visible = false;
#ifndef PBL_PLATFORM_APLITE
  popup_window->draw_sequence = NULL;
  popup_window->draw_frame = NULL;
  popup_window->frame_count = 0;
  popup_window->frame_index = 0;
#else
  popup_window->image = NULL;
#endif
  return popup_window;
}



/*
 * destroy a previously created PopupWindow
 */

void popup_window_destroy(PopupWindow *popup_window) {
  if (popup_window != NULL) {
    free(popup_window);
    popup_window = NULL;
    return;
  }
  // error handling
  APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted to free NULL PopupWindow");
}



/*
 * push the PopupWindow's window onto the window stack
 */

void popup_window_push(PopupWindow *popup_window, bool animated) {
  if (popup_window->window == NULL) {
    popup_window->window = window_create();
    window_set_user_data(popup_window->window, popup_window);
    window_set_window_handlers(popup_window->window,
      (WindowHandlers){
        .load = prv_window_load,
        .unload = prv_window_unload
      });
  }
  if (popup_window->window) {
    window_stack_push(popup_window->window, animated);
  }
}



/*
 * remove the PopupWindow's window from the window stack
 */

void popup_window_pop(PopupWindow *popup_window, bool animated) {
  if (popup_window->window) {
    window_stack_remove(popup_window->window, animated);
  }
}



/*
 * gets whether the PopupWindow's window is the topmost on the window stack
 */

bool popup_window_get_topmost_window(PopupWindow *popup_window) {
  return window_stack_get_top_window() == popup_window->window;
}



/*
 * set the time that the PopupWindow will automatically pop itself off the stack
 * it will auto-pop when a refresh event is called after this time
 */

void popup_window_set_auto_close_duration(PopupWindow *popup_window, int64_t duration) {
  popup_window->set_time = countdown_timer_get_epoch_ms();
  if (duration > 0) {
    popup_window->close_time = popup_window->set_time + duration;
  }
  else {
    popup_window->close_time = 0;
  }
}



/*
 * sets up an app_timer callback to run multiple instances of a vibration pattern
 */

void popup_window_set_vibes() {
  s_app_timer = app_timer_register(0, app_timer_vibe_callback, (void*)NUM_VIBE_INTERVALS);
}



/*
 * sets the CountdownTimer associated with this PopupWindow
 */

void popup_window_set_countdown_timer(PopupWindow *popup_window, CountdownTimer *countdown_timer) {
  popup_window->countdown_timer = countdown_timer;
}



/*
 * refresh the PopupWindow and step any ongoing PDC animation
 */

void popup_window_refresh(PopupWindow *popup_window) {
  int64_t current_time = countdown_timer_get_epoch_ms();
  // check if it should auto-pop
  if (popup_window->close_time != 0 && popup_window->close_time <= current_time) {
    popup_window_pop(popup_window, true);
  }

  // step animation
#ifndef PBL_PLATFORM_APLITE
  if (popup_window->endless) {
    popup_window->draw_frame = gdraw_command_sequence_get_frame_by_index(
      popup_window->draw_sequence, popup_window->frame_index);
  } else {
    popup_window->draw_frame = gdraw_command_sequence_get_frame_by_elapsed(
      popup_window->draw_sequence, current_time - popup_window->set_time);
  }
  popup_window->frame_index++;
  if (popup_window->frame_index >= popup_window->frame_count) {
    popup_window->frame_index = 0;
  }
#endif
  // refresh
  layer_mark_dirty(popup_window->layer);
}

#ifndef PBL_PLATFORM_APLITE

/*
 * sets the PDC that will be displayed
 * and resizes the layer containing it so that it is centered
 */

void popup_window_set_pdc(PopupWindow *popup_window, uint32_t resource_id, bool endless) {
  // temp variable
  GDrawCommandSequence *tmp_sequence = popup_window->draw_sequence;
  // load PDC
  popup_window->draw_sequence = gdraw_command_sequence_create_with_resource(resource_id);
  popup_window->frame_count = gdraw_command_sequence_get_num_frames(popup_window->draw_sequence);
  popup_window->frame_index = 0;
  popup_window->endless = endless;
  // destroy old pointer
  if (tmp_sequence != NULL) {
    gdraw_command_sequence_destroy(tmp_sequence);
  }
}

/*
 * gets the duration of the PDC currently assigned to the PopupWindow
 * returns 0 if the PDC is looping
 */

int64_t popup_window_get_pdc_duration(PopupWindow *popup_window) {
  return gdraw_command_sequence_get_total_duration(popup_window->draw_sequence);
}

#else

/*
 * sets the image that will be displayed
 * and resizes the layer containing it so that it is centered
 */

void popup_window_set_image(PopupWindow *popup_window, uint32_t resource_id) {
  // check if old image and destroy
  if (popup_window->image != NULL) {
    gbitmap_destroy(popup_window->image);
  }
  // load image
  popup_window->image = gbitmap_create_with_resource(resource_id);
}

#endif

/*
 * sets the text to display at the bottom of the PopupWindow
 */

void popup_window_set_title(PopupWindow *popup_window, const char *text) {
  popup_window->title = text;
}



/*
 * set the background color of the window
 */

void popup_window_set_highlight_color(PopupWindow *popup_window, GColor color) {
  popup_window->highlight_color = color;
}



/*
 * adds the ActionBarLayer to the PopupWindow
 * this ActionBarLayer was already created in the creation of the PopupWindow
 * it is merely added and removed from the PopupWindow's window
 */

void popup_window_add_action_bar(PopupWindow *popup_window) {
  popup_window->action_visible = true;
}



/*
 * removes the ActionBarLayer to the PopupWindow
 * this ActionBarLayer was already created in the creation of the PopupWindow
 * it is merely added and removed from the PopupWindow's window
 */

void popup_window_remove_action_bar(PopupWindow *popup_window) {
  popup_window->action_visible = false;
}



/*
 * sets the callbacks associated with this PopupWindow
 */

void popup_window_set_action_bar_callbacks(PopupWindow *popup_window,
                                           PopupWindowCallbacks callbacks) {
  popup_window->callbacks = callbacks;
}



/*
 * cleans up the callbacks associated with this PopupWindow
 */

void popup_window_remove_action_bar_callbacks(PopupWindow *popup_window) {
  popup_window->callbacks = (PopupWindowCallbacks) { NULL };
}
