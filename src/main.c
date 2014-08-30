#include <pebble.h>
#include <ctype.h>

static Window    *s_main_window;

static TextLayer *s_dow_layer;
static TextLayer *s_batt_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;

static bool      has_small_font;
static GFont     s_small_font;
static bool      has_large_font;
static GFont     s_large_font;

static bool      enabled;

static int minute_when_last_updated;

#define PERSIST_BLACK_ON_WHITE 0
#define PERSIST_SHOW_DATE 1
#define PERSIST_SHOW_BATTERY 2

static GColor fg;
static GColor bg;

static bool black_on_white;
static bool show_date;
static bool show_battery;

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
    strftime(time_buffer, sizeof(time_buffer), "%I:%M:%S %p", tick_time);
    time_buffer[9]  = (char)toupper((unsigned int)time_buffer[9]);
    time_buffer[10] = (char)toupper((unsigned int)time_buffer[11]);
    time_buffer[11] = '\0';
  }
  
  if (minute_when_last_updated != tick_time->tm_min) {
    if (show_date) {
      text_layer_set_text(s_dow_layer,  dow_buffer);
      text_layer_set_text(s_date_layer, date_buffer);
    }
  }
  text_layer_set_text(s_time_layer, time_buffer);
  
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
  text_layer_set_text(s_batt_layer, buffer);
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
    vpos += 20;
  }
  vpos_time = vpos; vpos += 40;	
  
  vmove = vcenter - ((vtop + vpos) / 2); 
  
  if (vpos_dow  > -1) { vpos_dow  += vmove; }
  if (vpos_date > -1) { vpos_date += vmove; }
  if (vpos_time > -1) { vpos_time += vmove; }
  
  fg = black_on_white ? GColorBlack : GColorWhite;
  bg = black_on_white ? GColorWhite : GColorBlack;
  window_set_background_color(s_main_window, bg);
  
  s_batt_layer = NULL;
  s_dow_layer  = NULL;
  s_date_layer = NULL;
  
  if (show_battery) {
    s_batt_layer = text_layer_create(GRect( 2, vpos_batt, 140, 20));
  }
  if (show_date) {
    s_dow_layer  = text_layer_create(GRect( 2, vpos_dow , 140, 20));
    s_date_layer = text_layer_create(GRect( 2, vpos_date, 140, 20));
  }
  
  if (clock_is_24h_style() == true) {
    s_time_layer = text_layer_create(GRect(0, vpos_time, 144, 40));
  } else {
    s_time_layer = text_layer_create(GRect(0, vpos_time, 144, 40));
  }
  
  if (show_battery) {
    text_layer_set_background_color(s_batt_layer, bg);
    text_layer_set_text_color(s_batt_layer, fg);
  }
  
  if (show_date) {
    text_layer_set_background_color(s_dow_layer, bg);
    text_layer_set_text_color(s_dow_layer, fg);
    text_layer_set_background_color(s_date_layer, bg);
    text_layer_set_text_color(s_date_layer, fg);
  }
  
  text_layer_set_background_color(s_time_layer, bg);
  text_layer_set_text_color(s_time_layer, fg);
  
  if (show_battery || show_date) {
    has_small_font = 1;
    s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_16));
  } else {
    has_small_font = 0;
  }
  
  has_large_font = 1;
  s_large_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_CONDENSED_32));
  
  if (show_battery) {
    text_layer_set_font(s_batt_layer, s_small_font);
    text_layer_set_text_alignment(s_batt_layer, GTextAlignmentRight);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_batt_layer));
  }
  
  if (show_date) {
    text_layer_set_font(s_dow_layer,  s_small_font);
    text_layer_set_font(s_date_layer, s_small_font);
    text_layer_set_text_alignment(s_dow_layer, GTextAlignmentLeft);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_dow_layer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  }
  
  text_layer_set_font(s_time_layer, s_large_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  update_time();
  
  if (show_battery) {
    battery_state = battery_state_service_peek();
    on_battery_state_change(battery_state);
    battery_state_service_subscribe(on_battery_state_change);
  }
  
  enabled = 1;
}

static void main_window_unload(Window *window) {
  enabled = 0;
  
  battery_state_service_unsubscribe();
  
  if (s_dow_layer) {
    text_layer_destroy(s_dow_layer);
    s_dow_layer = NULL;
  }
  if (s_batt_layer) {
    text_layer_destroy(s_batt_layer);
    s_batt_layer = NULL;
  }
  if (s_date_layer) {
    text_layer_destroy(s_date_layer);
    s_date_layer = NULL;
  }
  if (s_time_layer) {
    text_layer_destroy(s_time_layer);
    s_time_layer = NULL;
  }
  if (has_small_font) {
    fonts_unload_custom_font(s_small_font);
    has_small_font = 0;
  }
  if (has_large_font) {
    fonts_unload_custom_font(s_large_font);
    has_large_font = 0;
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  if (enabled) {
    update_time();
  }
}

#define KEY_CONFIG_BLACK_ON_WHITE 0
#define KEY_CONFIG_SHOW_DATE 1
#define KEY_CONFIG_SHOW_BATTERY 2

static void message_handler(DictionaryIterator *received, void *context) {

  bool bool_value;
  bool refresh_window = 0;

  app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, "this is message_handler");

  Tuple *tuple_black_on_white = dict_find(received, KEY_CONFIG_BLACK_ON_WHITE);
  Tuple *tuple_show_date      = dict_find(received, KEY_CONFIG_SHOW_DATE);
  Tuple *tuple_show_battery   = dict_find(received, KEY_CONFIG_SHOW_BATTERY);

  if (tuple_black_on_white) {
    app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__,
	    "tuple_black_on_white type %d length %d key %d",
	    (int)tuple_black_on_white->type,
	    (int)tuple_black_on_white->length,
	    (int)tuple_black_on_white->key);
    refresh_window = 1;
    black_on_white = (bool)tuple_black_on_white->value->int32;
  }
  if (tuple_show_date) {
    app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__,
	    "tuple_show_date type %d length %d key %d",
	    (int)tuple_show_date->type,
	    (int)tuple_show_date->length,
	    (int)tuple_show_date->key);
    refresh_window = 1;
    show_date = (bool)tuple_show_date->value->int32;
  }
  if (tuple_show_battery) {
    app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__,
	    "tuple_show_battery type %d length %d key %d",
	    (int)tuple_show_battery->type,
	    (int)tuple_show_battery->length,
	    (int)tuple_show_battery->key);
    refresh_window = 1;
    show_battery = (bool)tuple_show_battery->value->int32;
  }

  if (refresh_window) {
    main_window_unload(s_main_window);
    main_window_load(s_main_window);
  }
}

static void init() {

  black_on_white = 1;
  show_date      = 1;
  show_battery   = 1;

  if (persist_exists(PERSIST_BLACK_ON_WHITE)) {
    black_on_white = persist_read_bool(PERSIST_BLACK_ON_WHITE);
  }
  if (persist_exists(PERSIST_SHOW_DATE)) {
    show_date = persist_read_bool(PERSIST_SHOW_DATE);
  }
  if (persist_exists(PERSIST_SHOW_BATTERY)) {
    show_battery = persist_read_bool(PERSIST_SHOW_BATTERY);
  }

  s_main_window = window_create();
  window_set_background_color(s_main_window, bg);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load   = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  app_message_open(app_message_inbox_size_maximum(),
		   app_message_outbox_size_maximum());
  app_message_register_inbox_received(message_handler);
}

static void deinit() {
  app_message_deregister_callbacks();
  window_destroy(s_main_window);
}

int main(void) {
  enabled = 0;
  init();
  app_event_loop();
  deinit();
}

