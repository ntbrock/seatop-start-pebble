#include <pebble.h>
#include "seatop.h"
//----------------------------------------------------------------
// Global Variables

static Window *s_main_window;
static TextLayer *s_time_layer;

// Implemented the Menu as a separate window to gain use of the back key automatically.
static Window *s_menu_window;
static MenuLayer *s_menu_layer;

//----------------------------------------------------------------
// Animation Handlers
// Using the pattern of 'on_' function names prefixes to represent handlers / callbacks.

void on_animation_stopped(Animation *anim, bool finished, void *context)
{
  // Free the Mallocs, required for aplite watch
  property_animation_destroy((PropertyAnimation*) anim);
}

void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay )
{
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);

  //Set characteristics
  animation_set_duration((Animation*) anim, duration);
  animation_set_delay((Animation*) anim, delay);

  //Set stopped handler to free memory, single array element
  AnimationHandlers handlers = { .stopped = (AnimationStoppedHandler) on_animation_stopped };
  animation_set_handlers((Animation*) anim, handlers, NULL);

  // Animation start immediately
  animation_schedule((Animation*) anim);
}



//----------------------------------------------------------------
// Menu Layer Handlers

#define NUM_MENU_SECTIONS 1

static int16_t on_menu_get_header_height(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void on_menu_draw_header(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  menu_cell_basic_header_draw(ctx, cell_layer, "Menu One");
}

static uint16_t on_menu_get_num_sections(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t on_menu_get_num_rows(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return 2;
}

static void on_menu_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data ) { 
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          // This is a basic menu item with a title and subtitle
	  menu_cell_basic_draw(ctx, cell_layer, "Item Alpha", "Subtitle Alpha", NULL);
          break;
        case 1:
          // This is a basic menu icon with a cycling icon
	  menu_cell_basic_draw(ctx, cell_layer, "Item Bravo", "Subtitle Bravo with a very long description that should clip.", NULL);
          break;
      }
      break;
  }
}


static void on_menu_select(MenuLayer *menu_layer, MenuIndex *cell_index, void *data ) { 
  switch ( cell_index->row ) { 
  case 0:
    vibes_short_pulse();  // As every thing else above for menuing, this needs to be replaced with app specific logic.
    layer_mark_dirty(menu_layer_get_layer(menu_layer));
    window_stack_pop(true); // Dont' need to rebuild the main window every time.
    break;
  case 1:
    vibes_double_pulse();
    layer_mark_dirty(menu_layer_get_layer(menu_layer));
    window_stack_pop(true);
    break;
  default:
    APP_LOG(APP_LOG_LEVEL_ERROR, ":89 Received a Menu Select that did not match case statement.");
  }
}

static void menu_config() { 

}

// Other locations in the application call this to bring the menu to front for selection
void show_menu_and_handle_clicks( Window * window ) {

  /** Old method of addding it as a layer */
  /*
  Layer *window_layer = window_get_root_layer(window);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
  */

  /** New method of initialziing the window and adding it ot teh stack */
  //BOOK
  window_stack_push(s_menu_window, true);
      
}

//----------------------------------------------------------------
// Click Handlers

static void on_select_click(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "on_select_click");
}

static void on_select_long_click(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "on_select_long_click");
}

static void on_up_click(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "on_up_click");
  // Bring menu to view and register click handlers.
  show_menu_and_handle_clicks( s_main_window );
}

static void on_down_click(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "on_down_click");
}

static void main_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, on_select_click);
  window_long_click_subscribe(BUTTON_ID_SELECT, 500, NULL, on_select_long_click);
  window_single_click_subscribe(BUTTON_ID_UP, on_up_click);
  window_single_click_subscribe(BUTTON_ID_DOWN, on_down_click);

}


//----------------------------------------------------------------
// AppMessage and BLE Handlers

/**
 * Simple convenience method to send one single key->pair integer tuple
 */

static void send_int_as_tuple ( uint8_t key, uint8_t cmd ) { 
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet value = TupletInteger(key, cmd);
  dict_write_tuplet(iter, &value);
  app_message_outbox_send();
}

/**
 * Generic processor layer to receive events, SAX Style
 */

static void on_inbox_received_tuple(Tuple *t) {
  // t->value->int32
  // t->value->cstring
  if ( t->key == APPMESSAGE_PING ) {
    send_int_as_tuple( APPMESSAGE_PONG, APPMESSAGE_PONG );
  }

}


static void on_inbox_received(DictionaryIterator *iterator, void *context) {

  (void) context;
  // Loop thorugh all tuples , every time. This is Java SAX style processor -vs- explicltly looking at a path, which is DOM style.
  Tuple *t = dict_read_first(iterator);
  while(t != NULL) {
    on_inbox_received_tuple(t);
    t = dict_read_next(iterator);
  }

  // Record time of arrival
  time_t temp = time(NULL);
  struct tm *tm = localtime(&temp);
}

static void on_inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void on_outbox_sent(DictionaryIterator *iterator, void *context) {
}

static void on_outbox_failed(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

//----------------------------------------------------------------
// App Worker Handlers

static void on_app_worker_message(uint16_t type, AppWorkerMessage *data) { 

}



//----------------------------------------------------------------
// Time Handlers (This IS a watch after all)

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
           "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}


static void on_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}



//----------------------------------------------------------------
// Windowing and User Interface


static void on_main_window_load(Window *window) {
  // Bounding Dimensions
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Back Background for Main Window
  window_set_background_color(window, GColorBlack);

  // Create the Time Layer with specific bounds
  s_time_layer = text_layer_create( GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // End Time Layer Setup
  
  // Add all components.
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Regsiter click handlers
  window_set_click_config_provider( s_main_window, main_click_config_provider);

  // Ensure time is latest.
  update_time();
}


static void on_main_window_unload(Window *window) {
  // De Time Layer
  text_layer_destroy(s_time_layer);
  // Do not need to explicitly destroy our window, avoids Double Free Error - window_destroy(s_main_window);
}



/**
 * Menu Window creation
 */

static void on_menu_window_load(Window *window) {
  // Bounding Dimensions
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the Menu Layer 
  s_menu_layer = menu_layer_create(bounds);
  menu_config(s_menu_layer);

  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
      .get_num_sections = on_menu_get_num_sections,
	.get_num_rows = on_menu_get_num_rows,
	.get_header_height = on_menu_get_header_height,
	.draw_header = on_menu_draw_header,
	.draw_row = on_menu_draw_row,
	.select_click = on_menu_select,
	.get_cell_height = NULL
	});

  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

}

static void on_menu_window_unload(Window *window) {
  // De Menu Layer
  menu_layer_destroy(s_menu_layer);

  // DO Not do this - window_destroy(s_menu_window);
}


//----------------------------------------------------------------
// Startup and Shutdown

/**
 * on_init
 * Handles Callback Registration and initial window setup
 */
static void on_init() { 
  app_worker_launch();
  app_worker_message_subscribe(on_app_worker_message);

  // Main Window Setup
  s_main_window = window_create();
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = on_main_window_load,
    .unload = on_main_window_unload
        });

  // MENU Window Setup
  s_menu_window = window_create();
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_menu_window, (WindowHandlers) {
      .load = on_menu_window_load,
    .unload = on_menu_window_unload
        });

  // We are a go! Show the window to the user.
  window_stack_push(s_main_window, true);

  // ---------- CALL BACK REGISTRATIONS -----------------------
  // Register Tick Timer Callbacks
  tick_timer_service_subscribe(MINUTE_UNIT, on_tick);

  // Register App Messsge Callbacks
  app_message_register_inbox_received(on_inbox_received);
  app_message_register_inbox_dropped(on_inbox_dropped);
  app_message_register_outbox_failed(on_outbox_failed);
  app_message_register_outbox_sent(on_outbox_sent);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

}

static void on_deinit() { 

  // De-AppMesage
  app_worker_message_unsubscribe();
  connection_service_unsubscribe();

  // De-Windowing
  window_destroy(s_main_window);
}


int main(void) {
  on_init();
  app_event_loop();
  on_deinit();
}

