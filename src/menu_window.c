/*******************************************************************************
 * FILENAME :        menu_window.c
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

 #include <pebble.h>
 #include "menu_window.h"
 #include "countdown_timer.h"



/*******************************************************************************
 * STRUCTURE DEFINITION
 */

/*
 * the structure of a MenuWindow
 */

struct MenuWindow {
  Window      *window;    //< main window
  MenuLayer   *menu;      //< menu layer displaying timer list
  TextLayer   *text;      //< text layer which displays "No Timers"
  GBitmap     *play_icon, *pause_icon;    //< menu layer icons
#ifdef PBL_SDK_3
  StatusBarLayer      *status;            //< status bar for Basalt
#endif
  MenuWindowCallbacks callbacks;          //< menu layer callbacks
};



/*******************************************************************************
 * PRIVATE FUNCTIONS
 */

/*
 * get number of sections for menu layer
 * this is always zero for this application
 */

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *context) {
  return 1;
}



/*
 * get number of rows per section for menu layer
 * since there is only one section, no need to take sections into account
 */

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index,
                                           void *context) {
  MenuWindow *menu_window = (MenuWindow*)context;
  return menu_window->callbacks.get_timer_count(context) + 1;
}



/*
 * draw each row for menu layer
 */

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index,
                                   void *context) {
  MenuWindow *menu_window = (MenuWindow*)context;
  GSize size = layer_get_frame(cell_layer).size;
  // draw "+" at top and then timers
  if (cell_index->row == 0){
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, "+", fonts_get_system_font(FONT_KEY_GOTHIC_28),
      GRect(0, 1, size.w, size.h), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }
  else {
    CountdownTimer *countdown_timer =
      menu_window->callbacks.get_timer(cell_index->row - 1, context);
    // check if valid
    if (countdown_timer == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error retrieving CountdownTimer for menu drawing.");
      return;
    }
    char *buff = countdown_timer_format_own_buff(countdown_timer);
    menu_cell_basic_draw(ctx, cell_layer, buff, NULL, countdown_timer_get_paused(countdown_timer) ?
      menu_window->pause_icon : menu_window->play_icon);
    // exit if not drawing progress bar
    if (countdown_timer_get_start(countdown_timer) == 0) {
      return;
    }
    // otherwise draw progress bar
#ifdef PBL_DISP_SHAPE_ROUND
    int16_t prog_bar_border = 25;
#else
    int16_t prog_bar_border = 10;
#endif
#ifdef PBL_COLOR
    if (menu_layer_get_selected_index(menu_window->menu).row == cell_index->row) {
      graphics_context_set_fill_color(ctx, GColorWhite);
    }
    else {
      graphics_context_set_fill_color(ctx, GColorLightGray);
    }
    graphics_fill_rect(ctx, GRect(prog_bar_border, size.h - prog_bar_border,
      (size.w - prog_bar_border * 2), 2), 0, GCornerNone);
#endif
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(prog_bar_border, size.h - prog_bar_border,
      (size.w - prog_bar_border * 2) * countdown_timer_get_current_time(countdown_timer) /
      countdown_timer_get_duration(countdown_timer), 2), 0, GCornerNone);
  }
}



/*
 * menu layer clicked callback
 */

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  MenuWindow *menu_window = (MenuWindow*)context;
  menu_window->callbacks.clicked(cell_index->row, context);
}



/*
 * menu window initialize
 */

static MenuWindow *menu_window_init(MenuWindow *menu_window,
                                    MenuWindowCallbacks menu_window_callbacks, bool animated) {
  // load resources
  menu_window->play_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLAY_TRANS_WHITE);
  menu_window->pause_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PAUSE_TRANS_WHITE);
  // create window
  menu_window->window = window_create();
  menu_window->callbacks = menu_window_callbacks;
  if (menu_window->play_icon && menu_window->pause_icon && menu_window->window) {
    // get window parameters
    Layer *root = window_get_root_layer(menu_window->window);
    GRect bounds = layer_get_frame(root);
    // create menu layer
#ifdef PBL_SDK_3
    uint16_t vert_offset = DISP_SHAPE_SELECT(STATUS_BAR_LAYER_HEIGHT, 0);
    menu_window->menu = menu_layer_create(GRect(0, vert_offset, bounds.size.w, bounds.size.h -
      vert_offset));
#else
    menu_window->menu = menu_layer_create(bounds);
#endif
#ifdef PBL_DISP_SHAPE_ROUND
    menu_layer_set_center_focused(menu_window->menu, true);
#endif
    menu_layer_set_callbacks(menu_window->menu, menu_window, (MenuLayerCallbacks) {
      .get_num_sections = menu_get_num_sections_callback,
      .get_num_rows = menu_get_num_rows_callback,
      .draw_row = menu_draw_row_callback,
      .select_click = menu_select_callback,
    });
    menu_layer_set_click_config_onto_window(menu_window->menu, menu_window->window);
    layer_add_child(root, menu_layer_get_layer(menu_window->menu));
    // create text layer
#ifdef PBL_SDK_3
    menu_window->text = text_layer_create(GRect(0, 99, bounds.size.w, 20));
#else
    menu_window->text = text_layer_create(GRect(0, 85, bounds.size.w, 20));
#endif
    text_layer_set_text(menu_window->text, "No Timers");
    text_layer_set_font(menu_window->text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(menu_window->text, GTextAlignmentCenter);
    text_layer_set_background_color(menu_window->text, GColorClear);
    layer_add_child(root, text_layer_get_layer(menu_window->text));
    // create status bar
#ifdef PBL_SDK_3
    menu_window->status = status_bar_layer_create();
    status_bar_layer_set_colors(menu_window->status, GColorClear, GColorBlack);
    layer_add_child(root, status_bar_layer_get_layer(menu_window->status));
#endif
    // push window
    window_stack_push(menu_window->window, animated);
    return menu_window;
  }

  // free menu window
  free(menu_window);
  return NULL;
}



/*******************************************************************************
 * API FUNCTIONS
 */

/*
 * create a new MenuWindow and return a pointer to it
 * this includes creating all its children layers
 */

MenuWindow *menu_window_create(MenuWindowCallbacks menu_window_callbacks, bool animated) {
  MenuWindow *menu_window = (MenuWindow*)malloc(sizeof(MenuWindow));
  if (menu_window) {
    menu_window_init(menu_window, menu_window_callbacks, animated);
  }
  else {
    // error handling
    APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to create MenuWindow");
    return NULL;
  }
  return menu_window;
}



/*
 * destroy a previously created MenuWindow
 */

void menu_window_destroy(MenuWindow *menu_window) {
  if (menu_window != NULL) {
#ifdef PBL_SDK_3
    status_bar_layer_destroy(menu_window->status);
#endif
    text_layer_destroy(menu_window->text);
    menu_layer_destroy(menu_window->menu);
    window_destroy(menu_window->window);
    gbitmap_destroy(menu_window->play_icon);
    gbitmap_destroy(menu_window->pause_icon);
    free(menu_window);
    return;
  }
  // error handling
  APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted to free NULL MenuWindow");
}



/*
 * gets whether it is the topmost window on the stack
 */

bool menu_window_get_topmost_window(MenuWindow *menu_window) {
  return window_stack_get_top_window() == menu_window->window;
}



/*
 * refresh the provided MenuWindow
 */

void menu_window_refresh(MenuWindow *menu_window) {
  layer_mark_dirty(menu_layer_get_layer(menu_window->menu));
  uint8_t timer_count = menu_window->callbacks.get_timer_count(NULL);
  layer_set_hidden(text_layer_get_layer(menu_window->text), timer_count != 0);
}



/*
 * reload the data for the MenuWindow's MenuLayer
 */

void menu_window_reload_data(MenuWindow *menu_window) {
  // this makes the selected index go back to the top, but this is also desired
  menu_layer_reload_data(menu_window->menu);
}



/*
 * set highlight color of this window
 * this is the overall color scheme used
 */

void menu_window_set_highlight_color(MenuWindow *menu_window, GColor color) {
#ifdef PBL_COLOR
  menu_layer_set_highlight_colors(menu_window->menu, color, GColorBlack);
#endif
}
