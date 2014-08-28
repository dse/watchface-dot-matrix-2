#include <pebble.h>
#include <ctype.h>

static Window    *s_main_window;

static TextLayer *s_dow_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_layer;

static GFont     s_dow_font;
static GFont     s_date_font;
static GFont     s_time_font;

static int minute_when_last_updated;

static void update_time() {
  unsigned int i;
  unsigned int j;

  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  static char dow_buffer[]  = "WEDNESDAY";
  static char date_buffer[] = "2014-01-01";
  static char time_buffer[] = "23:25:32 p.m.";
  /*                                     1 1 */
  /*                           0    5    0 2 */
  
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
    text_layer_set_text(s_dow_layer,  dow_buffer);
    text_layer_set_text(s_date_layer, date_buffer);
  }
  text_layer_set_text(s_time_layer, time_buffer);

  minute_when_last_updated = tick_time->tm_min;
}

static void main_window_load(Window *window) {

  minute_when_last_updated = -1;
  
  s_dow_layer  = text_layer_create(GRect(2, 35, 140, 20));
  s_date_layer = text_layer_create(GRect(2, 55, 140, 20));
  if (clock_is_24h_style() == true) {
    s_time_layer = text_layer_create(GRect(2, 85, 140, 40));
  } else {
    s_time_layer = text_layer_create(GRect(0, 85, 144, 40));
  }

  text_layer_set_background_color(s_dow_layer, GColorBlack);
  text_layer_set_text_color(s_dow_layer, GColorClear);
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorClear);
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorClear);

  //text_layer_set_text(s_time_layer, "00:00");
  //text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  s_dow_font  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_16));
  s_date_font = s_dow_font;
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DOT_MATRIX_NUMBER_ONE_CONDENSED_32));

  text_layer_set_font(s_dow_layer,  s_dow_font);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_font(s_time_layer, s_time_font);
  
  text_layer_set_text_alignment(s_dow_layer, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_dow_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  update_time();
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_dow_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_dow_font);
  //fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_time_font);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load   = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

