#include <pebble.h>
#include "seatop_start.h"
//----------------------------------------------------------------
// Global Variables

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_text_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_second_layer;

// Implemented the Menu as a separate window to gain use of the back key automatically.
static Window *s_menu_window;
static MenuLayer *s_menu_layer;

#define INITIAL_SECONDS 300
// #define INITIAL_SECONDS 65 // Quick Testing 

static int s_remaining_seconds = INITIAL_SECONDS;
static int s_in_sequence = 0;



//----------------------------------------------------------------
// Prototypes

static void complete_sequence();

//----------------------------------------------------------------
// Time Handlers (This IS a watch after all)

static void vibe_if_needed(int minutes, int seconds ) {

  if ( seconds == 30 || seconds == 45 || seconds == 15) { 

    uint32_t segments[] = { 100 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };

    vibes_enqueue_custom_pattern(pattern);
  
  } else if ( minutes == 4 && seconds == 59 ) { 
    
    uint32_t segments[] = { 500 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };
    vibes_enqueue_custom_pattern(pattern);

  } else if ( minutes == 4 && seconds == 0 ) { 
    
    //                       1         2         3         4
    uint32_t segments[] = { 250, 150, 250, 150, 250, 150, 250, 150 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };
    vibes_enqueue_custom_pattern(pattern);
    
  } else if ( minutes == 3 && seconds == 0 ) { 

    //                       1         2         3      
    uint32_t segments[] = { 250, 150, 250, 150, 250, 150 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };
    vibes_enqueue_custom_pattern(pattern);

  } else if ( minutes == 2 && seconds == 0 ) { 

    //                       1         2              
    uint32_t segments[] = { 250, 150, 250, 150 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };
    vibes_enqueue_custom_pattern(pattern);

  } else if ( minutes == 1 && seconds == 0 ) { 

    uint32_t segments[] = { 500 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };
    vibes_enqueue_custom_pattern(pattern);

    // final count down // 10 SECONDS
  } else if ( minutes == 0 && seconds == 10 ) { 

    uint32_t segments[] = { 500 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };

    vibes_enqueue_custom_pattern(pattern);
  
  } else if ( minutes == 0 && seconds != 0 && seconds <= 5 ) { 

    uint32_t segments[] = { 50 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };

    vibes_enqueue_custom_pattern(pattern);
  

  } else if ( minutes == 0 && seconds == 0 ) { 

    uint32_t segments[] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
    VibePattern pattern = { 
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments)
    };
    vibes_enqueue_custom_pattern(pattern);

  } 

}

static void update_time(struct tm *on_tick_time) {

  time_t temp = time(NULL);
  struct tm * tick_time = localtime(&temp);

  // struct tm * tick_time = &temp;
  
   
  // Write the current hours and minutes into a buffer
  static char s_buffer[12];
  strftime(s_buffer, sizeof(s_buffer), "%T", tick_time);


  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);


  // Get time since launch
  int seconds = s_remaining_seconds % 60;
  int minutes = (s_remaining_seconds % 3600) / 60;


  if ( s_in_sequence ) { 
    s_remaining_seconds--;

    // Vibe handles completeion vibe as well
    vibe_if_needed(minutes, seconds);
    if ( s_remaining_seconds < 0 ) { complete_sequence(); }

  }



  static char s_buffer_minutes[3];
  snprintf( s_buffer_minutes, 3, "%d", minutes );

  static char s_buffer_seconds[3];
  snprintf( s_buffer_seconds, 3, "%02d", seconds );
  
  text_layer_set_text(s_minute_layer, s_buffer_minutes);
  text_layer_set_text(s_second_layer, s_buffer_seconds);

}


static void resume_sequence() { 
  // Turn colors active
  text_layer_set_background_color(s_minute_layer, GColorWhite);
  text_layer_set_text_color(s_minute_layer, GColorBlack);
  text_layer_set_background_color(s_second_layer, GColorBlack);
  text_layer_set_text_color(s_second_layer, GColorWhite);

  // Handle the first click
  if ( s_remaining_seconds == INITIAL_SECONDS ) {
    s_remaining_seconds--;
  }


  s_in_sequence = 1;
  // update_time();  Doing update time here causes to fast of a second loss
} 

static void pause_sequence() { 
  // Turn colors INActive
  text_layer_set_background_color(s_minute_layer, GColorBlack);
  text_layer_set_text_color(s_minute_layer, GColorLightGray);
  text_layer_set_background_color(s_second_layer, GColorBlack);
  text_layer_set_text_color(s_second_layer, GColorLightGray);

  s_remaining_seconds++; // ad a second back to clock
  s_in_sequence = 0;
  update_time(0);
}

static void reset_sequence() { 
  // Turn colors INActive
  text_layer_set_background_color(s_minute_layer, GColorBlack);
  text_layer_set_text_color(s_minute_layer, GColorLightGray);
  text_layer_set_background_color(s_second_layer, GColorBlack);
  text_layer_set_text_color(s_second_layer, GColorLightGray);

  s_remaining_seconds = INITIAL_SECONDS;
  s_in_sequence = 0;
  update_time(0);
} 

static void complete_sequence() { 
  // Turn colors INActive
  text_layer_set_background_color(s_minute_layer, GColorWhite);
  text_layer_set_text_color(s_minute_layer, GColorBlack);
  text_layer_set_background_color(s_second_layer, GColorWhite);
  text_layer_set_text_color(s_second_layer, GColorBlack);

  s_in_sequence = 0;
  s_remaining_seconds = 0;
} 


//----------------------------------------------------------------
// Click Handlers

static void on_select_click(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "on_select_click");

  // resume or pause countdown
  if ( s_in_sequence ) { 
    pause_sequence();
  } else { 
    // Begin 
    resume_sequence();
  } 

}

static void on_select_long_click(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "on_select_long_click");

  reset_sequence();
}

static void on_up_click(ClickRecognizerRef recognizer, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "on_up_click");

  // go to the nearest minute outside of 30 sec
  if ( s_remaining_seconds > 30 ) { 

    if ( s_remaining_seconds % 60 >= 30 ) { 
      s_remaining_seconds = ( (int)( s_remaining_seconds / 60.0 ) + 1 ) * 60;
    } else if ( s_remaining_seconds % 60 <= 30 ) { 
      s_remaining_seconds = ( (int)( s_remaining_seconds / 60.0 )) * 60;
    }
  }
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



static void on_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
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
  s_time_layer = text_layer_create( GRect(0, 2, bounds.size.w, 50));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_font(s_time_layer, fonts_get_system_font( FONT_KEY_ROBOTO_CONDENSED_21 ));


  // Create the Time Layer with specific bounds
  s_text_layer = text_layer_create( GRect(0, bounds.size.h - 28, bounds.size.w, 28));

  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_color(s_text_layer, GColorLightGray);
  text_layer_set_text(s_text_layer, "SeaTop Start");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_text_layer, fonts_get_system_font( FONT_KEY_ROBOTO_CONDENSED_21 ));


  //-----------------------------------
  // Create the Minute Layer with specific bounds

  int padX = 5;
  int secPadX = 5;
  int padY = 60;
  int height = 60;

  s_minute_layer = text_layer_create( GRect(padX, padY, bounds.size.w / 2 - padX, height ));
  text_layer_set_text(s_minute_layer, "-");
  text_layer_set_text_alignment(s_minute_layer, GTextAlignmentCenter);
  text_layer_set_font(s_minute_layer, fonts_get_system_font( FONT_KEY_LECO_42_NUMBERS ));

  s_second_layer = text_layer_create( GRect(bounds.size.w / 2 , padY, bounds.size.w / 2 - secPadX, height ));
  text_layer_set_text(s_second_layer, "--");
  text_layer_set_text_alignment(s_second_layer, GTextAlignmentCenter);
  text_layer_set_font(s_second_layer, fonts_get_system_font( FONT_KEY_LECO_42_NUMBERS ));

  // Ensure time is latest.
  reset_sequence();
  update_time(0);


  // End Time Layer Setup
  
  // Add all components.
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_second_layer));

  // Regsiter click handlers
  window_set_click_config_provider( s_main_window, main_click_config_provider);

}


static void on_main_window_unload(Window *window) {
  // De Time Layer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_text_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(s_second_layer);
  // Do not need to explicitly destroy our window, avoids Double Free Error - window_destroy(s_main_window);
}



//----------------------------------------------------------------
// Startup and Shutdown

/**
 * on_init
 * Handles Callback Registration and initial window setup
 */
static void on_init() { 

  // Main Window Setup
  s_main_window = window_create();
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = on_main_window_load,
    .unload = on_main_window_unload
        });

  // We are a go! Show the window to the user.
  window_stack_push(s_main_window, true);

  // ---------- CALL BACK REGISTRATIONS -----------------------
  // Register Tick Timer Callbacks
  tick_timer_service_subscribe(SECOND_UNIT, on_tick);

}

static void on_deinit() { 

  // De-Windowing
  window_destroy(s_main_window);
}


int main(void) {
  on_init();
  app_event_loop();
  on_deinit();
}

