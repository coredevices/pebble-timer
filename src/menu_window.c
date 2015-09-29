// @file menu_window.c
// @brief Main list view for timers
//
// Contains code to display all current timers in a MenuLayer with simple progress bars and a
// "+" button for adding new timers. Modeled after the alarm app
//
// @author Eric D. Phillips
// @date September 13, 2015
// @bugs No known bugs

#include "menu_window.h"
#include "countdown_timer.h"
#include "utility.h"

// Constants
#define MENU_WINDOW_NUM_SECTIONS 1
#define MENU_WINDOW_FOREGROUND_COLOR GColorBlack
#define MENU_WINDOW_PROGRESS_BAR_COLOR COLOR_FALLBACK(GColorLightGray, GColorWhite)
#define MENU_WINDOW_PROGRESS_BAR_SELECTED_COLOR GColorWhite
#define MENU_WINDOW_PROGRESS_BAR_BORDER 10
#define MENU_WINDOW_PROGRESS_BAR_THICKNESS 2

// Menu window user data
typedef struct {
  MenuLayer     *menu_layer;      //< Main MenuLayer to display all timers
  GBitmap       *image_play;      //< Play icon next to row text
  GBitmap       *image_pause;     //< Pause icon next to row text
} MenuWindowData;

// Comparability functions for SDK 2.0
#ifdef PBL_SDK_2
void menu_layer_set_highlight_colors(MenuLayer *menu_layer, GColor background, GColor foreground){}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//

// Get the number of sections for the MenuLayer
static uint16_t menu_layer_get_num_sections_callback(MenuLayer *menu_layer, void *contrxt) {
  // in this setup, we only have one section with all the timers under it,
  // so we will always return "1" for the number of sections
  return MENU_WINDOW_NUM_SECTIONS;
}

// Get the number of rows for the MenuLayer
static uint16_t menu_layer_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index,
                                                 void *context) {
  // return the current number of timers plus one extra.
  // the extra addition is for the "+" row which must always be accounted for in the MenuLayer
  return countdown_timer_get_timer_count() + 1;
}

// Draw each row for MenuLayer
static void menu_layer_draw_row_callback(GContext *ctx, const Layer *cell_layer,
                                         MenuIndex *cell_index, void *context) {
  // get the size of the cell to draw
  MenuWindowData *user_data = window_get_user_data((Window*)context);
  GRect bounds = layer_get_bounds(cell_layer);
  // draw the current cell
  if (cell_index->row == 0) {
    // if first cell, draw "+" symbol for adding timers
    graphics_context_set_text_color(ctx, MENU_WINDOW_FOREGROUND_COLOR);
    graphics_draw_text(ctx, "+", fonts_get_system_font(FONT_KEY_GOTHIC_28), bounds,
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  } else {
    // get CountdownTimer (offset index by one to account for "+" row)
    uint32_t countdown_timer_id = countdown_timer_get_id_from_index(cell_index->row - 1);
    // get formatted timer text
    char buff[9];
    countdown_timer_format_as_string(countdown_timer_id, buff, sizeof(buff));
    // draw timer text and image
    GBitmap *icon = countdown_timer_get_paused(countdown_timer_id) ?
                    user_data->image_pause : user_data->image_play;
    menu_cell_basic_draw(ctx, cell_layer, buff, NULL, icon);
    // draw progress bar
    bool selected = menu_layer_get_selected_index(user_data->menu_layer).row == cell_index->row;
    graphics_context_set_fill_color(ctx, selected ? MENU_WINDOW_PROGRESS_BAR_SELECTED_COLOR :
                                    MENU_WINDOW_PROGRESS_BAR_COLOR);
    graphics_fill_rect(ctx, GRect(MENU_WINDOW_PROGRESS_BAR_BORDER,
                                  bounds.size.h - MENU_WINDOW_PROGRESS_BAR_BORDER,
                                  bounds.size.w - 2 * MENU_WINDOW_PROGRESS_BAR_BORDER,
                                  MENU_WINDOW_PROGRESS_BAR_THICKNESS),
                       1, GCornerNone);
    graphics_context_set_fill_color(ctx, MENU_WINDOW_FOREGROUND_COLOR);
    int64_t progress_bar_width = bounds.size.w - 2 * MENU_WINDOW_PROGRESS_BAR_BORDER;
    progress_bar_width = progress_bar_width * countdown_timer_get_current_value(countdown_timer_id)
                         / countdown_timer_get_total_length(countdown_timer_id);
    graphics_fill_rect(ctx, GRect(MENU_WINDOW_PROGRESS_BAR_BORDER,
                                  bounds.size.h - MENU_WINDOW_PROGRESS_BAR_BORDER,
                                  progress_bar_width,
                                  MENU_WINDOW_PROGRESS_BAR_THICKNESS),
                       1, GCornerNone);
  }
}

// Select click callback
static void menu_layer_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index,
                                       void  *context) {
  // TODO: Do something when clicked
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// API Interface
//

// Render a menu window
void menu_window_render(Window *window) {
  // check if visible
  if (window != window_stack_get_top_window()) {
    return;
  }

  // do other stuff
}

// Create a menu Window
Window *menu_window_create(void) {
  // create a window
  Window *window = window_create();
  ASSERT(window);
  Layer *window_root = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_root);
  // create user data
  MenuWindowData *user_data = MALLOC(sizeof(MenuWindowData));
  window_set_user_data(window, user_data);
  // create menu layer
  user_data->menu_layer = menu_layer_create(window_bounds);
  ASSERT(user_data->menu_layer);
  menu_layer_set_callbacks(user_data->menu_layer, window, (MenuLayerCallbacks) {
    .get_num_sections = menu_layer_get_num_sections_callback,
    .get_num_rows = menu_layer_get_num_rows_callback,
    .draw_row = menu_layer_draw_row_callback,
    .select_click = menu_layer_select_callback,
  });
  menu_layer_set_highlight_colors(user_data->menu_layer, GLOBAL_APP_HIGHLIGHT_COLOR,
                                  MENU_WINDOW_FOREGROUND_COLOR);
  menu_layer_set_click_config_onto_window(user_data->menu_layer, window);
  layer_add_child(window_root, menu_layer_get_layer(user_data->menu_layer));

  // use different code for different platforms
#ifdef PBL_DISP_SHAPE_ROUND
  menu_layer_set_center_focused(user_data->menu_layer, true);
#endif

  // load resources
  user_data->image_play = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLAY);
  ASSERT(user_data->image_play);
  user_data->image_pause = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE);
  ASSERT(user_data->image_pause);

  // return pointer
  // Note: this can never return NULL, as it will assert if malloc fails
  return window;
}

// Destroy a menu Window
void menu_window_destroy(Window *window) {
  // get the window user data
  MenuWindowData *user_data = window_get_user_data(window);
  // destroy layers
  menu_layer_destroy(user_data->menu_layer);
  // destroy resources
  gbitmap_destroy(user_data->image_play);
  gbitmap_destroy(user_data->image_pause);
  // free user data and window
  free(user_data);
  window_destroy(window);
}
