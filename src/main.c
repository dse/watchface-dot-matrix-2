#include <pebble.h>
#include <ctype.h>

#include "main.h"

#define app__log(a) app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, a);

static Window    *s_main_window;

static TextLayer *s_dow_text_layer;
static TextLayer *s_batt_text_layer;
static TextLayer *s_date_text_layer;
static TextLayer *s_time_text_layer;

static bool      has_small_font;
static GFont     s_small_font;
static bool      has_large_font;
static GFont     s_large_font;

static bool      enabled;

static int       minute_when_last_updated;

/* options --- these macros are used as parameters for both persist
   keys and config keys. */
#define OPTION_BLACK_ON_WHITE    0
#define OPTION_SHOW_DATE         1
#define OPTION_SHOW_BATTERY      2
#define OPTION_LARGER_CLOCK_FONT 3

static GColor fg;
static GColor bg;

/* options */
static bool black_on_white;
static bool show_date;
static bool show_battery;
static bool larger_clock_font;

static void update_time() {
  unsigned int i;
  unsigned int j;
  
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  static char dow_buffer[]  = "WEDNESDAY";
  static char date_buffer[] = "2014-01-01";
  static char time_buffer[] = "23:25:32 p.m."; /* room for strftime() */
  /*                          [          1 1] */
  /*                          [0    5    0 2] */

  if (minute_when_last_updated != tick_time->tm_min) {
    strftime(dow_buffer,  sizeof(dow_buffer),  "%A", tick_time);
    for (j = strlen(dow_buffer), i = 0; i < j; i += 1) {
      dow_buffer[i] = (char)toupper((unsigned int)dow_buffer[i]);
    }
    strftime(date_buffer, sizeof(date_buffer), "%Y-%m-%d", tick_time);
  }
  
  if (clock_is_24h_style() == true) {
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", tick_time);
  } else {
    if (larger_clock_font) {
      strftime(time_buffer, sizeof(time_buffer), "%I:%M:%S", tick_time);
    } else {
      strftime(time_buffer, sizeof(time_buffer), "%I:%M:%S %p", tick_time);
      time_buffer[9]  = (char)toupper((unsigned int)time_buffer[9]);
      time_buffer[10] = (char)toupper((unsigned int)time_buffer[11]);
      time_buffer[11] = '\0';
    }
  }
  
  if (minute_when_last_updated != tick_time->tm_min) {
    if (show_date) {
      text_layer_set_text(s_dow_text_layer,  dow_buffer);
      text_layer_set_text(s_date_text_layer, date_buffer);
    }
  }

  text_layer_set_text(s_time_text_layer, time_buffer);
  
  minute_when_last_updated = tick_time->tm_min;
}

static void on_battery_state_change(BatteryChargeState charge) {
  static char buffer[] = "++++++++++";
  int l;
  
  snprintf(buffer, sizeof(buffer), "%d%%", charge.charge_percent);
  if (charge.is_charging) {
    l = strlen(buffer);
    strncpy(buffer + l, " CH", sizeof(buffer) - l);
  }
  if (charge.is_plugged) {
    l = strlen(buffer);
    strncpy(buffer + l, " PL", sizeof(buffer) - l);
  }
  text_layer_set_text(s_batt_text_layer, buffer);
}

static void main_window_load(Window *window) {
  static BatteryChargeState battery_state;
  
  static int vpos_batt;
  static int vpos_dow;
  static int vpos_date;
  static int vpos_time;
  static int vtop;
  static int vcenter;
  static int vpos;
  static int vmove;
  
  minute_when_last_updated = -1;
  
  vpos_batt = -1;
  vpos_dow  = -1;
  vpos_date = -1;
  vpos_time = -1;
  
  vpos = 0;
  if (show_battery) {
    vpos_batt = vpos; vpos += 20;
  }
  vtop = vpos;			
  
  vcenter = (vtop + 168) / 2;	
  
  if (show_date) {
    vpos_dow  = vpos; vpos += 20;
    vpos_date = vpos; vpos += 20;
    vpos += 20;			/* for space between date and time */
  }

  vpos_time = vpos; vpos += 32;	/* larger clock font is just as tall */
  
  vmove = vcenter - ((vtop + vpos) / 2); 
  
  if (vpos_dow  > -1) { vpos_dow  += vmove; }
  if (vpos_date > -1) { vpos_date += vmove; }
  if (vpos_time > -1) { vpos_time += vmove; }
  
  fg = black_on_white ? GColorBlack : GColorWhite;
  bg = black_on_white ? GColorWhite : GColorBlack;
  window_set_background_color(s_main_window, bg);
  
  s_batt_text_layer = NULL;
  s_dow_text_layer  = NULL;
  s_date_text_layer = NULL;
  
  if (show_battery) {
    s_batt_text_layer = text_layer_create(GRect( 2, vpos_batt, 140, 20));
  }
  if (show_date) {
    s_dow_text_layer  = text_layer_create(GRect( 2, vpos_dow , 140, 20));
    s_date_text_layer = text_layer_create(GRect( 2, vpos_date, 140, 20));
  }
  
  s_time_text_layer = text_layer_create(GRect(0, vpos_time, 144, 32)); /* larger clock font is just as tall */
  
  if (show_battery) {
    text_layer_set_background_color(s_batt_text_layer, bg);
    text_layer_set_text_color(s_batt_text_layer, fg);
  }
  
  if (show_date) {
    text_layer_set_background_color(s_dow_text_layer, bg);
    text_layer_set_text_color(s_dow_text_layer, fg);
    text_layer_set_background_color(s_date_text_layer, bg);
    text_layer_set_text_color(s_date_text_layer, fg);
  }

  text_layer_set_background_color(s_time_text_layer, bg);
  text_layer_set_text_color(s_time_text_layer, fg);
  
  if (show_battery || show_date) {
    has_small_font = 1;
    s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_16));
  } else {
    has_small_font = 0;
  }
  
  has_large_font = 1;
  if (larger_clock_font) {
    s_large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_DOTTED_SEMICONDENSED_32));
  } else {
    s_large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_CONDENSED_32));
  }
  
  if (show_battery) {
    text_layer_set_font(s_batt_text_layer, s_small_font);
    text_layer_set_text_alignment(s_batt_text_layer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_batt_text_layer));
  }
  
  if (show_date) {
    text_layer_set_font(s_dow_text_layer,  s_small_font);
    text_layer_set_font(s_date_text_layer, s_small_font);
    text_layer_set_text_alignment(s_dow_text_layer, GTextAlignmentLeft);
    text_layer_set_text_alignment(s_date_text_layer, GTextAlignmentLeft);
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_dow_text_layer));
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_date_text_layer));
  }

  text_layer_set_font(s_time_text_layer, s_large_font);
  text_layer_set_text_alignment(s_time_text_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_time_text_layer));
  
  update_time();
  
  if (show_battery) {
    battery_state = battery_state_service_peek();
    on_battery_state_change(battery_state);
    battery_state_service_subscribe(on_battery_state_change);
  }

  enabled = 1;
}

static void main_window_unload(Window *window) {
  int i;

  enabled = 0;
  
  battery_state_service_unsubscribe();
  
  if (s_dow_text_layer) {
    text_layer_destroy(s_dow_text_layer);
    s_dow_text_layer = NULL;
  }
  if (s_batt_text_layer) {
    text_layer_destroy(s_batt_text_layer);
    s_batt_text_layer = NULL;
  }
  if (s_date_text_layer) {
    text_layer_destroy(s_date_text_layer);
    s_date_text_layer = NULL;
  }
  if (s_time_text_layer) {
    text_layer_destroy(s_time_text_layer);
    s_time_text_layer = NULL;
  }
  if (has_small_font) {
    fonts_unload_custom_font(s_small_font);
    has_small_font = 0;
    s_small_font = NULL;
  }
  if (has_large_font) {
    fonts_unload_custom_font(s_large_font);
    has_large_font = 0;
    s_large_font = NULL;
  }

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (enabled) {
    update_time();
  }
}

static void message_handler(DictionaryIterator *received, void *context) {

  bool refresh_window = 0;

  app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, "this is message_handler");

  /* options */
  Tuple *tuple_black_on_white    = dict_find(received, OPTION_BLACK_ON_WHITE);
  Tuple *tuple_show_date         = dict_find(received, OPTION_SHOW_DATE);
  Tuple *tuple_show_battery      = dict_find(received, OPTION_SHOW_BATTERY);
  Tuple *tuple_larger_clock_font = dict_find(received, OPTION_LARGER_CLOCK_FONT);

  /* options */
  if (tuple_black_on_white) {
    app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, "found tuple_black_on_white");
    refresh_window = 1;
    black_on_white = (bool)tuple_black_on_white->value->int32;
  }
  if (tuple_show_date) {
    app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, "found tuple_show_data");
    refresh_window = 1;
    show_date = (bool)tuple_show_date->value->int32;
  }
  if (tuple_show_battery) {
    app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, "found tuple_show_battery");
    refresh_window = 1;
    show_battery = (bool)tuple_show_battery->value->int32;
  }
  if (tuple_larger_clock_font) {
    app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, "found tuple_larger_clock_font");
    refresh_window = 1;
    larger_clock_font = (bool)tuple_larger_clock_font->value->int32;
  }

  /* options */
  persist_write_bool(OPTION_BLACK_ON_WHITE, black_on_white);
  persist_write_bool(OPTION_SHOW_DATE,      show_date);
  persist_write_bool(OPTION_SHOW_BATTERY,   show_battery);
  persist_write_bool(OPTION_LARGER_CLOCK_FONT, larger_clock_font);

  if (refresh_window) {
    main_window_unload(s_main_window);
    main_window_destroy();
    main_window_create();
    main_window_load(s_main_window);
  }
}

static void main_window_create() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, bg);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load   = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
}

static void main_window_destroy() {
  window_stack_pop(true);
  window_destroy(s_main_window);
}

static void init() {

  /* options */
  black_on_white    = 0;
  show_date         = 1;
  show_battery      = 1;
  larger_clock_font = 0;

  /* options */
  if (persist_exists(OPTION_BLACK_ON_WHITE)) {
    black_on_white = persist_read_bool(OPTION_BLACK_ON_WHITE);
  }
  if (persist_exists(OPTION_SHOW_DATE)) {
    show_date = persist_read_bool(OPTION_SHOW_DATE);
  }
  if (persist_exists(OPTION_SHOW_BATTERY)) {
    show_battery = persist_read_bool(OPTION_SHOW_BATTERY);
  }
  if (persist_exists(OPTION_LARGER_CLOCK_FONT)) {
    larger_clock_font = persist_read_bool(OPTION_LARGER_CLOCK_FONT);
  }

  main_window_create();
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  app_message_open(app_message_inbox_size_maximum(),
		   app_message_outbox_size_maximum());
  app_message_register_inbox_received(message_handler);
}

static void deinit() {
  app_message_deregister_callbacks();
  tick_timer_service_unsubscribe();
  main_window_destroy();
}

int main(void) {
  enabled = 0;
  init();
  app_event_loop();
  deinit();
}

