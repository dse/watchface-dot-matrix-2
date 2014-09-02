#include <pebble.h>
#include <ctype.h>

#define LARGER_CLOCK_BITMAP 0

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

#define LARGER_CLOCK_FONT_ICONS 14

static GBitmap     *larger_clock_font_icons;
static GBitmap     *larger_clock_font_icon[LARGER_CLOCK_FONT_ICONS];
static BitmapLayer *s_larger_clock_bitmap_layer[8];
static InverterLayer *s_larger_clock_inverter_layer[8];

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

  if (larger_clock_font) {
    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[0], larger_clock_font_icon[time_buffer[0] - '0']);
    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[1], larger_clock_font_icon[time_buffer[1] - '0']);
    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[3], larger_clock_font_icon[time_buffer[3] - '0']);
    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[4], larger_clock_font_icon[time_buffer[4] - '0']);
    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[6], larger_clock_font_icon[time_buffer[6] - '0']);
    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[7], larger_clock_font_icon[time_buffer[7] - '0']);
  } else {
    text_layer_set_text(s_time_text_layer, time_buffer);
  }
  
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

  if (larger_clock_font) {
    vpos_time = vpos; vpos += 54;
  } else {
    vpos_time = vpos; vpos += 40;
  }
  
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
  
  if (larger_clock_font) {
    s_time_text_layer = NULL;
  } else {
    s_time_text_layer = text_layer_create(GRect(0, vpos_time, 148, 40));
  }
  
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

  if (!larger_clock_font) {
    text_layer_set_background_color(s_time_text_layer, bg);
    text_layer_set_text_color(s_time_text_layer, fg);
  } else {
  }
  
  if (show_battery || show_date) {
    has_small_font = 1;
    s_small_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_16));
  } else {
    has_small_font = 0;
  }
  
  has_large_font = 1;
  if (larger_clock_font) {
    has_large_font = 0;		/* not using a font */
    s_large_font = NULL;
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

  if (!larger_clock_font) {
    text_layer_set_font(s_time_text_layer, s_large_font);
    text_layer_set_text_alignment(s_time_text_layer, GTextAlignmentCenter);
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(s_time_text_layer));
  } else {
  }
  
  if (larger_clock_font) {

    larger_clock_font_icons = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LARGER_CLOCK_FONT);

    /*                                                                                                     1         1         1         1         1    */
    /*           1         2         3         4         5         6         7         8         9         0         1         2         3         4    */
    /* 012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123 */
    /*    --------------       --------------    --------    --------------       --------------    --------    --------------       --------------     */
    /*    ## ## ## ## ## -- -- ## ## ## ## ## -- -- ## -- -- ## ## ## ## ## -- -- ## ## ## ## ## -- -- ## -- -- ## ## ## ## ## -- -- ## ## ## ## ##     */

    larger_clock_font_icon[0]  /* '0' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(  3, 0, 14, 27));
    larger_clock_font_icon[1]  /* '1' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect( 24, 0, 14, 27));
    larger_clock_font_icon[2]  /* '2' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect( 45, 0, 14, 27));
    larger_clock_font_icon[3]  /* '3' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect( 66, 0, 14, 27));
    larger_clock_font_icon[4]  /* '4' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect( 87, 0, 14, 27));
    larger_clock_font_icon[5]  /* '5' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(108, 0, 14, 27));
    larger_clock_font_icon[6]  /* '6' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(129, 0, 14, 27));
    larger_clock_font_icon[7]  /* '7' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(150, 0, 14, 27));
    larger_clock_font_icon[8]  /* '8' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(171, 0, 14, 27));
    larger_clock_font_icon[9]  /* '9' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(192, 0, 14, 27));
    larger_clock_font_icon[10] /* ':' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(210, 0,  8, 27));
    larger_clock_font_icon[11] /* 'A' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(222, 0, 14, 27));
    larger_clock_font_icon[12] /* 'P' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(243, 0, 14, 27));
    larger_clock_font_icon[13] /* 'M' */ = gbitmap_create_as_sub_bitmap(larger_clock_font_icons, GRect(264, 0, 14, 27));
    
    s_larger_clock_bitmap_layer[0] = bitmap_layer_create(GRect(  3, vpos_time, 14, 27)); /* H */
    s_larger_clock_bitmap_layer[1] = bitmap_layer_create(GRect( 24, vpos_time, 14, 27)); /* H */
    s_larger_clock_bitmap_layer[2] = bitmap_layer_create(GRect( 42, vpos_time,  8, 27)); /* : */
    s_larger_clock_bitmap_layer[3] = bitmap_layer_create(GRect( 54, vpos_time, 14, 27)); /* M */
    s_larger_clock_bitmap_layer[4] = bitmap_layer_create(GRect( 75, vpos_time, 14, 27)); /* M */
    s_larger_clock_bitmap_layer[5] = bitmap_layer_create(GRect( 93, vpos_time,  8, 27)); /* : */
    s_larger_clock_bitmap_layer[6] = bitmap_layer_create(GRect(105, vpos_time, 14, 27)); /* S */
    s_larger_clock_bitmap_layer[7] = bitmap_layer_create(GRect(126, vpos_time, 14, 27)); /* S */

    if (!black_on_white) {
      s_larger_clock_inverter_layer[0] = inverter_layer_create(GRect(  3, vpos_time, 14, 27)); /* H */
      s_larger_clock_inverter_layer[1] = inverter_layer_create(GRect( 24, vpos_time, 14, 27)); /* H */
      s_larger_clock_inverter_layer[2] = inverter_layer_create(GRect( 42, vpos_time,  8, 27)); /* : */
      s_larger_clock_inverter_layer[3] = inverter_layer_create(GRect( 54, vpos_time, 14, 27)); /* M */
      s_larger_clock_inverter_layer[4] = inverter_layer_create(GRect( 75, vpos_time, 14, 27)); /* M */
      s_larger_clock_inverter_layer[5] = inverter_layer_create(GRect( 93, vpos_time,  8, 27)); /* : */
      s_larger_clock_inverter_layer[6] = inverter_layer_create(GRect(105, vpos_time, 14, 27)); /* S */
      s_larger_clock_inverter_layer[7] = inverter_layer_create(GRect(126, vpos_time, 14, 27)); /* S */
    }

    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[2], larger_clock_font_icon[10]); /* ':' */
    bitmap_layer_set_bitmap(s_larger_clock_bitmap_layer[5], larger_clock_font_icon[10]); /* ':' */

    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[0]));
    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[1]));
    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[2]));
    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[3]));
    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[4]));
    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[5]));
    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[6]));
    layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_larger_clock_bitmap_layer[7]));

    if (!black_on_white) {
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[0]));
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[1]));
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[2]));
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[3]));
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[4]));
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[5]));
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[6]));
      layer_add_child(window_get_root_layer(s_main_window), inverter_layer_get_layer(s_larger_clock_inverter_layer[7]));
    }
    
  } else {
    larger_clock_font_icons = NULL;
  }
  
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
  }
  if (has_large_font) {
    fonts_unload_custom_font(s_large_font);
    has_large_font = 0;
  }
  if (larger_clock_font_icons) {

    gbitmap_destroy(larger_clock_font_icons);
    larger_clock_font_icons = NULL;

    for (i = 0; i < LARGER_CLOCK_FONT_ICONS; i += 1) {
      if (larger_clock_font_icon[i]) {
    	gbitmap_destroy(larger_clock_font_icon[i]);
    	larger_clock_font_icon[i] = NULL;
      }
    }

    for (i = 0; i < 8; i += 1) {
      if (s_larger_clock_bitmap_layer[i]) {
    	bitmap_layer_destroy(s_larger_clock_bitmap_layer[i]);
      }
      s_larger_clock_bitmap_layer[i] = NULL;
    }

    for (i = 0; i < 8; i += 1) {
      if (s_larger_clock_inverter_layer[i]) {
    	inverter_layer_destroy(s_larger_clock_inverter_layer[i]);
      }
      s_larger_clock_inverter_layer[i] = NULL;
    }

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
    main_window_load(s_main_window);
  }
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
  int i;

  larger_clock_font_icons = NULL;
  for (i = 0; i < LARGER_CLOCK_FONT_ICONS; i += 1) {
    larger_clock_font_icon[i] = NULL;
  }
  for (i = 0; i < 8; i += 1) {
    s_larger_clock_bitmap_layer[i] = NULL;
  }
  for (i = 0; i < 8; i += 1) {
    s_larger_clock_inverter_layer[i] = NULL;
  }

  enabled = 0;

  init();
  app_event_loop();
  deinit();
}

