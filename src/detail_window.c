/*******************************************************************************
 * FILENAME :        detail_window.c
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

#include <pebble.h>
#include "detail_window.h"
#include "countdown_timer.h"

#define TEXT_LAYER_MAX_LARGE_CHARACTERS 5



/*******************************************************************************
 * STRUCTURE DEFINITION
 */

/*
 * the structure of a DetailWindow
 */

struct DetailWindow {
  Window      *window;    //< main window
  Layer       *layer;     //< drawing layer
  TextLayer   *main_text; //< main, larger text
  TextLayer   *sub_text;  //< footer, small text
  ActionBarLayer *action; //< action bar
  GBitmap     *edit_icon, *play_icon, *pause_icon, *delete_icon;  //< icons
  GFont       large_font, medium_font, small_font; //< fonts
  GColor      highlight_color;        //< main color for highlights
  StatusBarLayer *status;             //< status bar for SDK 3
  DetailWindowCallbacks callbacks;    //< callbacks for button presses

  char        main_buff[12];          //< text buffer for main_text
  char        sub_buff[12];           //< text buffer for sub_text

  bool        animation_update_needed;    //< whether it needs to be refreshed

  CountdownTimer *countdown_timer;        //< the CountdownTimer being shown
};



/*******************************************************************************
 * PRIVATE FUNCTIONS
 */

/*
 * layer update proc
 * for animating bubbles in background
 */

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // get DetailWindow pointer from layer data
  DetailWindow *detail_window = (*(DetailWindow**)layer_get_data(layer));
  int64_t current_time = countdown_timer_get_current_time(detail_window->countdown_timer);
  int64_t total_time = countdown_timer_get_duration(detail_window->countdown_timer);
  int16_t water_level = layer_get_bounds(layer).size.h - layer_get_bounds(layer).size.h *
    current_time / total_time;

  // draw background
#ifdef PBL_ROUND
  graphics_context_set_fill_color(ctx, detail_window->highlight_color);
  GRect bounds = layer_get_bounds(layer);
  graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, bounds.size.w / 2,
    TRIG_MAX_ANGLE - TRIG_MAX_ANGLE * current_time / total_time, TRIG_MAX_ANGLE);
#else
  graphics_context_set_fill_color(ctx, detail_window->highlight_color);
  graphics_fill_rect(ctx, GRect(0, water_level, layer_get_bounds(layer).size.w,
    layer_get_bounds(layer).size.h - water_level), 1, GCornerNone);
#endif
}



/*******************************************************************************
 * CALLBACKS
 */

/*
 * UP click handler callback
 *
 * edits the timer
 */

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  DetailWindow *detail_window = (DetailWindow*)context;
  return detail_window->callbacks.edit_timer(detail_window->countdown_timer, context);
}



/*
 * SELECT click handler callback
 *
 * plays or pauses the timer
 */

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  DetailWindow *detail_window = (DetailWindow*)context;
  return detail_window->callbacks.playpause_timer(detail_window->countdown_timer, context);
}



/*
 * DOWN click handler callback
 *
 * deletes the timer
 */

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  DetailWindow *detail_window = (DetailWindow*)context;
  return detail_window->callbacks.delete_timer(detail_window->countdown_timer, context);
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



/*******************************************************************************
 * API FUNCTIONS
 */

static void prv_window_load(Window* window){
  DetailWindow *detail_window = window_get_user_data(window);
  window_set_background_color(detail_window->window, GColorLightGray);

  // load resources
  detail_window->edit_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EDIT);
  detail_window->play_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLAY);
  detail_window->pause_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
  detail_window->delete_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DELETE);
  detail_window->large_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_LECO_REGULAR_SUBSET_36));
  detail_window->medium_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_LECO_REGULAR_SUBSET_26));
  detail_window->small_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_LECO_REGULAR_SUBSET_20));
  // get window parameters
  Layer *root = window_get_root_layer(detail_window->window);
  GRect bounds = layer_get_frame(root);
  // create animation layer
  // IMPORTANT: must be created with data for the DetailWindow pointer
  // so that it can be accessed in the layer_update_proc callback
  detail_window->layer = layer_create_with_data(bounds, sizeof(DetailWindow*));
  DetailWindow **layer_data = (DetailWindow**)layer_get_data(detail_window->layer);
  (*layer_data) = detail_window;
  layer_set_update_proc(detail_window->layer, layer_update_proc);
  layer_add_child(root, detail_window->layer);
  // create main text
#ifdef PBL_ROUND
  detail_window->main_text = text_layer_create(
    GRect(0, 65, bounds.size.w - ACTION_BAR_WIDTH, 36));
#else
  detail_window->main_text = text_layer_create(
    GRect(0, 20, bounds.size.w - ACTION_BAR_WIDTH, 36));
#endif
  text_layer_set_font(detail_window->main_text, detail_window->large_font);
  text_layer_set_text(detail_window->main_text, "00:00");
  text_layer_set_text_alignment(detail_window->main_text, GTextAlignmentCenter);
  text_layer_set_background_color(detail_window->main_text, GColorClear);
  layer_add_child(root, text_layer_get_layer(detail_window->main_text));
  // create sub text
#ifdef PBL_ROUND
  detail_window->sub_text = text_layer_create(
    GRect(0, 145, bounds.size.w, 20));
    text_layer_set_text_alignment(detail_window->sub_text, GTextAlignmentCenter);
#else
  detail_window->sub_text = text_layer_create(
    GRect(10, 138, bounds.size.w - ACTION_BAR_WIDTH, 20));
    text_layer_set_text_alignment(detail_window->sub_text, GTextAlignmentLeft);
#endif
  text_layer_set_font(detail_window->sub_text, detail_window->small_font);
  text_layer_set_text(detail_window->sub_text, "00:00");
  text_layer_set_background_color(detail_window->sub_text, GColorClear);
  layer_add_child(root, text_layer_get_layer(detail_window->sub_text));
  // create action bar
  detail_window->action = action_bar_layer_create();
  action_bar_layer_add_to_window(detail_window->action, detail_window->window);
  action_bar_layer_set_click_config_provider(detail_window->action, click_config_provider);
  action_bar_layer_set_context(detail_window->action, detail_window);
  action_bar_layer_set_icon(detail_window->action, BUTTON_ID_UP, detail_window->edit_icon);
  action_bar_layer_set_icon(detail_window->action, BUTTON_ID_SELECT, detail_window->pause_icon);
  action_bar_layer_set_icon(detail_window->action, BUTTON_ID_DOWN, detail_window->delete_icon);
  // create status bar
#ifdef PBL_ROUND
  int16_t horiz_off = 0;
#else
  int16_t horiz_off = ACTION_BAR_WIDTH;
#endif
  detail_window->status = status_bar_layer_create();
  layer_set_frame(status_bar_layer_get_layer(detail_window->status),
    GRect(0, 0, bounds.size.w - horiz_off, STATUS_BAR_LAYER_HEIGHT));
  status_bar_layer_set_colors(detail_window->status, GColorClear, GColorBlack);
  layer_add_child(root, status_bar_layer_get_layer(detail_window->status));
}

static void prv_window_unload(Window* window){
  DetailWindow *detail_window = window_get_user_data(window);
  status_bar_layer_destroy(detail_window->status);
  action_bar_layer_destroy(detail_window->action);
  text_layer_destroy(detail_window->sub_text);
  text_layer_destroy(detail_window->main_text);
  layer_destroy(detail_window->layer);
  window_destroy(detail_window->window);
  gbitmap_destroy(detail_window->edit_icon);
  gbitmap_destroy(detail_window->play_icon);
  gbitmap_destroy(detail_window->pause_icon);
  gbitmap_destroy(detail_window->delete_icon);
  fonts_unload_custom_font(detail_window->large_font);
  fonts_unload_custom_font(detail_window->medium_font);
  fonts_unload_custom_font(detail_window->small_font);
  detail_window->window = NULL;
}


/*
 * create a new DetailWindow and return a pointer to it
 * this includes creating all its children layers but
 * does not push it onto the window stack
 */

DetailWindow *detail_window_create(DetailWindowCallbacks detail_window_callbacks) {
  DetailWindow *detail_window = (DetailWindow*)malloc(sizeof(DetailWindow));
  // error handling
  if (detail_window == NULL) {
    return NULL;
  }

  *detail_window = (DetailWindow) { .callbacks = detail_window_callbacks };
  
  return detail_window;
}



/*
 * destroy a previously created DetailWindow
 */

void detail_window_destroy(DetailWindow *detail_window) {
  if (detail_window != NULL) {
    free(detail_window);
    detail_window = NULL;
    return;
  }
}



/*
 * push the window onto the stack
 */

void detail_window_push(DetailWindow *detail_window, bool animated) {
  if (detail_window->window == NULL) {
    detail_window->window = window_create();
    window_set_user_data(detail_window->window, detail_window);
    window_set_window_handlers(detail_window->window,
      (WindowHandlers){
        .load = prv_window_load,
        .unload = prv_window_unload
      });
  }
  if (detail_window->window) {
    window_stack_push(detail_window->window, animated);
  }
}



/*
 * pop the window off the stack
 */

void detail_window_pop(DetailWindow *detail_window, bool animated) {
  if (detail_window->window) {
    window_stack_remove(detail_window->window, animated);
  }
}



/*
 * gets whether it is the topmost window on the stack
 */

bool detail_window_get_topmost_window(DetailWindow *detail_window) {
  return window_stack_get_top_window() == detail_window->window;
}



/*
 * set the timer associated with the window
 */

void detail_window_set_countdown_timer(DetailWindow *detail_window,
                                       CountdownTimer *countdown_timer) {
  detail_window->countdown_timer = countdown_timer;
}



/*
 * refresh the provided DetailWindow
 */

void detail_window_refresh(DetailWindow *detail_window) {
  if (detail_window->window == NULL) {
    return;
  }

  layer_mark_dirty(detail_window->layer);
  // main text
  countdown_timer_format_text(countdown_timer_get_current_time(detail_window->countdown_timer),
    detail_window->main_buff, sizeof(detail_window->main_buff));
  text_layer_set_text(detail_window->main_text, detail_window->main_buff);
  if (strlen(detail_window->main_buff) > TEXT_LAYER_MAX_LARGE_CHARACTERS) {
    text_layer_set_font(detail_window->main_text, detail_window->medium_font);
  } else {
    text_layer_set_font(detail_window->main_text, detail_window->large_font);
  }
  // sub text
  countdown_timer_format_text(countdown_timer_get_duration(detail_window->countdown_timer),
    detail_window->sub_buff, sizeof(detail_window->sub_buff));
  text_layer_set_text(detail_window->sub_text, detail_window->sub_buff);
}



/*
 * deep refresh the window, updating icons etc.
 */

void detail_window_deep_refresh(DetailWindow *detail_window) {
  if (detail_window->window != NULL && detail_window->countdown_timer != NULL) {
    action_bar_layer_set_icon(detail_window->action, BUTTON_ID_SELECT,
      countdown_timer_get_paused(detail_window->countdown_timer) ?
      detail_window->play_icon : detail_window->pause_icon);
    detail_window_refresh(detail_window);
    return;
  }
}



/*
 * set highlight color of this window
 * this is the overall color scheme used
 */

void detail_window_set_highlight_color(DetailWindow *detail_window, GColor color) {
  detail_window->highlight_color = color;
}



/*
 * gets whether it needs to be updated for the animations
 */

bool detail_window_get_update_needed(DetailWindow *detail_window) {
  return detail_window->animation_update_needed ||
    !countdown_timer_get_paused(detail_window->countdown_timer);
}
