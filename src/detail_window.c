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

 #define BUBBLE_DELTA 60
 #define BUBBLE_MAX_RADIUS 10
 #define BUBBLE_MIN_RADIUS 4
 #define BUBBLE_MAX_DELAY_MS 2000
 #define BUBBLE_MIN_DELAY_MS 600

 #define TEXT_LAYER_MAX_LARGE_CHARACTERS 5

#ifdef PBL_COLOR
#define BUBBLE_FILL_COLOR GColorCeleste
#define BUBBLE_POPPED_FILL_COLOR GColorLightGray
#else
#define BUBBLE_FILL_COLOR GColorWhite
#define BUBBLE_POPPED_FILL_COLOR GColorWhite
#endif



/*******************************************************************************
 * STRUCTURE DEFINITION
 */

 /*
  * the structure of a bubble for the animations
  */

typedef struct Bubble {
  GPoint      pt1;    //< actual point of bubble, moves side to side
  GPoint      pt2;    //< master point of bubble, moves straight up from bottom
  uint16_t    rad;    //< radius of bubble
  bool        live;   //< whether the bubble has popped or not
} Bubble;

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
  GFont       *large_font, *medium_font, *small_font; //< fonts
  GColor      highlight_color;        //< main color for highlights
#ifdef PBL_SDK_3
  StatusBarLayer *status;             //< status bar for SDK 3
#else
  GBitmap     *waves_image;           //< image for Aplite water level
#endif
  DetailWindowCallbacks callbacks;    //< callbacks for button presses

  char        main_buff[12];          //< text buffer for main_text
  char        sub_buff[12];           //< text buffer for sub_text

  Bubble      bubbles[4];                 //< array of Bubble structures
  int64_t     bubble_last_release;        //< time since last bubble release
  bool        animation_update_needed;    //< whether it needs to be refreshed
  bool        power_saver_mode;           //< whether it is in power saver mode or not

  CountdownTimer *countdown_timer;        //< the CountdownTimer being shown
};



/*******************************************************************************
 * PRIVATE FUNCTIONS
 */

/*
 * draw a thick line regardless of platform
 */

static void draw_thick_line(GContext *ctx, GPoint p0, GPoint p1) {
#ifdef PBL_SDK_3
  graphics_draw_line(ctx, p0, p1);
#else
  graphics_draw_line(ctx, p0, p1);
  graphics_draw_line(ctx, GPoint(p0.x + 1, p0.y), GPoint(p1.x + 1, p1.y));
  graphics_draw_line(ctx, GPoint(p0.x - 1, p0.y), GPoint(p1.x - 1, p1.y));
  graphics_draw_line(ctx, GPoint(p0.x, p0.y + 1), GPoint(p1.x, p1.y + 1));
  graphics_draw_line(ctx, GPoint(p0.x, p0.y - 1), GPoint(p1.x, p1.y - 1));
#endif
}



/*
 * draw bubbles on graphics context
 */

static void draw_bubbles(DetailWindow *detail_window, GContext *ctx, int16_t level) {
  // set up drawing environment
  graphics_context_set_stroke_color(ctx, GColorBlack);
#ifdef PBL_SDK_3
  graphics_context_set_stroke_width(ctx, 3);
#endif

  Bubble *t_bubble = detail_window->bubbles;
  for (uint8_t ii = 0; ii < ARRAY_LENGTH(detail_window->bubbles); ii++, t_bubble++) {
    // check if on screen
    if (t_bubble->live){
      // check if popped
      if (t_bubble->pt2.y > level){
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_circle(ctx, t_bubble->pt1, t_bubble->rad);
        graphics_context_set_fill_color(ctx, BUBBLE_FILL_COLOR);
        graphics_fill_circle(ctx, t_bubble->pt1, t_bubble->rad - 3);
      }
      else if (level < t_bubble->pt2.y + 15){
      // draw divot
        graphics_context_set_fill_color(ctx, BUBBLE_POPPED_FILL_COLOR);
        graphics_fill_circle(ctx, GPoint(t_bubble->pt1.x, t_bubble->pt2.y), t_bubble->rad - 2);
        // draw lines
        for (uint8_t jj = 0; jj < 4; jj++){
          // calculate based on angle
          GPoint pt1 = GPoint(t_bubble->pt1.x + (level - t_bubble->pt2.y + t_bubble->rad) *
            cos_lookup(jj * TRIG_MAX_ANGLE / 8 + TRIG_MAX_ANGLE / 16) / TRIG_MAX_RATIO,
            t_bubble->pt1.y - (level - t_bubble->pt2.y + t_bubble->rad) *
            sin_lookup(jj * TRIG_MAX_ANGLE / 8 + TRIG_MAX_ANGLE / 16) / TRIG_MAX_RATIO);
          GPoint pt2 = GPoint(t_bubble->pt1.x + (level - t_bubble->pt2.y + t_bubble->rad + 5) *
            cos_lookup(jj * TRIG_MAX_ANGLE / 8 + TRIG_MAX_ANGLE / 16) / TRIG_MAX_RATIO,
            t_bubble->pt1.y - (level - t_bubble->pt2.y + t_bubble->rad + 5) *
            sin_lookup(jj * TRIG_MAX_ANGLE / 8 + TRIG_MAX_ANGLE / 16) / TRIG_MAX_RATIO);
          draw_thick_line(ctx, pt1, pt2);
        }
      }
    }
  }
}



/*
 * ease in/out quadratic
 *
 * returns a value that oscillates like a sign wave
 */

static int32_t ease_in_out_quad(int32_t oT, int32_t b, int32_t c, int32_t d) {
  // this is a standard easing algorithm that can be found online
  // simply search for "ease in out quad" and you can find many examples
  int32_t t = oT * 1000;
  t /= d / 2;
  if (t < 1000) {
    return c / 2 * t * t / 1000000 + b;
  }
  t -= 1000;
  return -c/2 * (t*(t-2000) - 1000000) / 1000000 + b;
};



/*
 * step bubbles
 *
 * increment the bubbles position and state so that they move side
 * to side and upward
 */

static void step_bubbles(DetailWindow *detail_window, int16_t level) {
  int64_t epoch = countdown_timer_get_epoch_ms();
  GRect bounds = layer_get_bounds(detail_window->layer);

  detail_window->animation_update_needed =
    !countdown_timer_get_paused(detail_window->countdown_timer);

  Bubble *t_bubble = detail_window->bubbles;
  for (uint8_t ii = 0; ii < ARRAY_LENGTH(detail_window->bubbles); ii++, t_bubble++){
    // check if on screen
    if (t_bubble->live){
      // move bubble
      if (t_bubble->pt2.y > level){
        t_bubble->pt1.y -= 3;
        t_bubble->pt1.x = ease_in_out_quad(abs(t_bubble->pt1.y %
          (BUBBLE_DELTA * 2) - BUBBLE_DELTA), t_bubble->pt2.x, 15, BUBBLE_DELTA);
      }
      t_bubble->pt2.y -= 3;
      // check if dead
      if (t_bubble->pt2.y < 0){
        // set bubble to be dead
        t_bubble->live = false;
      }
      else { // animation steps are still needed
        detail_window->animation_update_needed = true;
      }
    }
    else if (epoch > detail_window->bubble_last_release &&
             !countdown_timer_get_paused(detail_window->countdown_timer)) {
      // create bubble
      t_bubble->rad = rand() % (BUBBLE_MAX_RADIUS - BUBBLE_MIN_RADIUS) + BUBBLE_MIN_RADIUS;
      // set bubble last time
      detail_window->bubble_last_release = epoch +
        rand() % (BUBBLE_MAX_DELAY_MS - BUBBLE_MIN_DELAY_MS) + BUBBLE_MIN_DELAY_MS;
      // continue creation
      t_bubble->pt1 = t_bubble->pt2 = GPoint(rand() % (bounds.size.w - ACTION_BAR_WIDTH) -
        BUBBLE_DELTA / 5, bounds.size.h + t_bubble->rad * 2);
      t_bubble->live = true;
    }
  }
}



/*
 * layer update proc
 * for animating bubbles in background
 */

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // get DetailWindow pointer from layer data
  DetailWindow *detail_window = (*(DetailWindow**)layer_get_data(layer));
  int64_t current_time = countdown_timer_get_current_time(detail_window->countdown_timer);
  int16_t water_level = layer_get_bounds(layer).size.h - layer_get_bounds(layer).size.h * current_time /
    countdown_timer_get_duration(detail_window->countdown_timer);

  // draw background
#ifdef PBL_DISP_SHAPE_ROUND
  graphics_context_set_fill_color(ctx, detail_window->highlight_color);
  GRect bounds = layer_get_bounds(layer);
  graphics_fill_radial(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 0, bounds.size.w / 2 + 5,
    TRIG_MAX_ANGLE - TRIG_MAX_ANGLE * current_time / countdown_timer_get_duration
    (detail_window->countdown_timer), TRIG_MAX_ANGLE);
#elif PBL_COLOR
  graphics_context_set_fill_color(ctx, detail_window->highlight_color);
  graphics_fill_rect(ctx, GRect(0, water_level, layer_get_bounds(layer).size.w,
    layer_get_bounds(layer).size.h - water_level), 1, GCornerNone);
#else
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, water_level, layer_get_bounds(layer).size.w, 2), 1, GCornerNone);
//  GRect img_frame = gbitmap_get_bounds(detail_window->waves_image);
//  img_frame.size.w = layer_get_bounds(layer).size.w;
//  img_frame.origin = GPoint(0, water_level - img_frame.size.h);
//  graphics_draw_bitmap_in_rect(ctx, detail_window->waves_image, img_frame);
#endif


//  if (!detail_window->power_saver_mode) {
//    // step bubbles
//    step_bubbles(detail_window, water_level);
//    // draw bubbles
//    draw_bubbles(detail_window, ctx, water_level);
//  }
//  else {
//    // draw text
//    graphics_context_set_text_color(ctx, GColorBlack);
//#ifdef PBL_SDK_3
//    graphics_draw_text(ctx, "Power Saver", fonts_get_system_font(FONT_KEY_GOTHIC_14),
//      GRect(0, 60, layer_get_bounds(detail_window->layer).size.w - ACTION_BAR_WIDTH, 35),
//      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
//#else
//    graphics_draw_text(ctx, "Power Saver", fonts_get_system_font(FONT_KEY_GOTHIC_14),
//      GRect(0, 44, layer_get_bounds(detail_window->layer).size.w - ACTION_BAR_WIDTH, 35),
//      GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
//#endif
//  }
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

/*
 * create a new DetailWindow and return a pointer to it
 * this includes creating all its children layers but
 * does not push it onto the window stack
 */

DetailWindow *detail_window_create(DetailWindowCallbacks detail_window_callbacks) {
  DetailWindow *detail_window = (DetailWindow*)malloc(sizeof(DetailWindow));
  // error handling
  if (detail_window == NULL) {
    // error handling
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create DetailWindow");
    return NULL;
  }

  // create window
  detail_window->window = window_create();
  // error handling
  if (detail_window->window == NULL) {
    free(detail_window);
    // error handling
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create window for DetailWindow");
    return NULL;
  }

#ifdef PBL_COLOR
  window_set_background_color(detail_window->window, GColorLightGray);
#endif
detail_window->callbacks = detail_window_callbacks;
  // load resources
  detail_window->edit_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EDIT);
  detail_window->play_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLAY);
  detail_window->pause_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
  detail_window->delete_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DELETE);
#ifndef PBL_COLOR
  detail_window->waves_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WAVES);
#endif
  detail_window->large_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_LECO_REGULAR_SUBSET_36));
  detail_window->medium_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_LECO_REGULAR_SUBSET_26));
  detail_window->small_font = fonts_load_custom_font(
    resource_get_handle(RESOURCE_ID_FONT_LECO_REGULAR_SUBSET_20));
  // zero some values
  detail_window->countdown_timer = NULL;
  memset(detail_window->bubbles, 0, sizeof(detail_window->bubbles));
  detail_window->bubble_last_release = 0;
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
#ifdef PBL_DISP_SHAPE_ROUND
  detail_window->main_text = text_layer_create(
    GRect(0, 65, bounds.size.w - ACTION_BAR_WIDTH, 36));
#elif PBL_SDK_3
  detail_window->main_text = text_layer_create(
    GRect(0, 20, bounds.size.w - ACTION_BAR_WIDTH, 36));
#else
  detail_window->main_text = text_layer_create(
    GRect(0, 7, bounds.size.w - ACTION_BAR_WIDTH, 36));
#endif
  //text_layer_set_text_color(detail_window->main_text, GColorBlack);
  text_layer_set_font(detail_window->main_text, detail_window->large_font);
  text_layer_set_text(detail_window->main_text, "00:00");
  text_layer_set_text_alignment(detail_window->main_text, GTextAlignmentCenter);
  text_layer_set_background_color(detail_window->main_text, GColorClear);
  layer_add_child(root, text_layer_get_layer(detail_window->main_text));
  // create sub text
#ifdef PBL_DISP_SHAPE_ROUND
  detail_window->sub_text = text_layer_create(
    GRect(0, 145, bounds.size.w, 20));
    text_layer_set_text_alignment(detail_window->sub_text, GTextAlignmentCenter);
#elif PBL_SDK_3
  detail_window->sub_text = text_layer_create(
    GRect(10, 138, bounds.size.w - ACTION_BAR_WIDTH, 20));
    text_layer_set_text_alignment(detail_window->sub_text, GTextAlignmentLeft);
#else
  detail_window->sub_text = text_layer_create(
    GRect(10, 122, bounds.size.w - ACTION_BAR_WIDTH, 20));
  text_layer_set_text_alignment(detail_window->sub_text, GTextAlignmentLeft);
#endif
  //text_layer_set_text_color(detail_window->main_text, GColorBlack);
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
#ifdef PBL_SDK_3
#ifdef PBL_DISP_SHAPE_ROUND
  int16_t horiz_off = 0;
#else
  int16_t horiz_off = ACTION_BAR_WIDTH;
#endif
  detail_window->status = status_bar_layer_create();
  layer_set_frame(status_bar_layer_get_layer(detail_window->status),
    GRect(0, 0, bounds.size.w - horiz_off, STATUS_BAR_LAYER_HEIGHT));
  status_bar_layer_set_colors(detail_window->status, GColorClear, GColorBlack);
  layer_add_child(root, status_bar_layer_get_layer(detail_window->status));
#endif
  return detail_window;
}



/*
 * destroy a previously created DetailWindow
 */

void detail_window_destroy(DetailWindow *detail_window) {
  if (detail_window != NULL) {
#ifdef PBL_SDK_3
    status_bar_layer_destroy(detail_window->status);
#endif
    action_bar_layer_destroy(detail_window->action);
    text_layer_destroy(detail_window->sub_text);
    text_layer_destroy(detail_window->main_text);
    layer_destroy(detail_window->layer);
    window_destroy(detail_window->window);
    gbitmap_destroy(detail_window->edit_icon);
    gbitmap_destroy(detail_window->play_icon);
    gbitmap_destroy(detail_window->pause_icon);
    gbitmap_destroy(detail_window->delete_icon);
#ifndef PBL_COLOR
    gbitmap_destroy(detail_window->waves_image);
#endif
    fonts_unload_custom_font(detail_window->large_font);
    fonts_unload_custom_font(detail_window->medium_font);
    fonts_unload_custom_font(detail_window->small_font);
    free(detail_window);
    detail_window = NULL;
    return;
  }
  // error handling
  APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted to free NULL DetailWindow");
}



/*
 * push the window onto the stack
 */

void detail_window_push(DetailWindow *detail_window, bool animated) {
  window_stack_push(detail_window->window, animated);
}



/*
 * pop the window off the stack
 */

void detail_window_pop(DetailWindow *detail_window, bool animated) {
  window_stack_remove(detail_window->window, animated);
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
  if (detail_window->countdown_timer != NULL) {
    action_bar_layer_set_icon(detail_window->action, BUTTON_ID_SELECT,
      countdown_timer_get_paused(detail_window->countdown_timer) ?
      detail_window->play_icon : detail_window->pause_icon);
    detail_window_refresh(detail_window);
    return;
  }
  // error handling
  APP_LOG(APP_LOG_LEVEL_ERROR, "Tried to deep refresh DetailWindow with NULL countdown_timer");
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



/*
 * sets whether it is in power saver mode
 */

void detail_window_set_power_saver_mode(DetailWindow *detail_window, bool mode) {
  detail_window->power_saver_mode = mode;
}
