#include <pebble.h>

static Window *s_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_battery_layer;
static GFont s_font_spacemono_bold_42;
static GFont s_font_spacemono_regular_21;
static GFont s_font_spacemono_regular_14;

static char s_time_buf[6];
static char s_date_buf[20];
static char s_battery_buf[8];

static void prv_update_time(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(s_time_buf, sizeof(s_time_buf), "%H:%M", t);
  text_layer_set_text(s_time_layer, s_time_buf);
  strftime(s_date_buf, sizeof(s_date_buf), "%a %d %b", t);
  text_layer_set_text(s_date_layer, s_date_buf);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_update_time();
}

static void prv_battery_handler(BatteryChargeState charge) {
  snprintf(s_battery_buf, sizeof(s_battery_buf), "%d%%", charge.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buf);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorWhite);

  // Load custom fonts
  s_font_spacemono_bold_42 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SPACEMONO_BOLD_42));
  s_font_spacemono_regular_21 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SPACEMONO_REGULAR_21));
  s_font_spacemono_regular_14 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SPACEMONO_REGULAR_14));

  // Time layer — large, centered
  int time_h = 50;
  int time_y = (bounds.size.h - time_h) / 2 - 10;
  s_time_layer = text_layer_create(GRect(0, time_y, bounds.size.w, time_h));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_font_spacemono_bold_42);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Date layer — below time
  int date_y = time_y + time_h + 2;
  s_date_layer = text_layer_create(GRect(0, date_y, bounds.size.w, 28));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, s_font_spacemono_regular_21);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Battery layer — top right
  s_battery_layer = text_layer_create(GRect(bounds.size.w - 50, 2, 48, 20));
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_font(s_battery_layer, s_font_spacemono_regular_14);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));

  // Show initial values
  prv_update_time();
  prv_battery_handler(battery_state_service_peek());
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_battery_layer);
  fonts_unload_custom_font(s_font_spacemono_bold_42);
  fonts_unload_custom_font(s_font_spacemono_regular_21);
  fonts_unload_custom_font(s_font_spacemono_regular_14);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, prv_tick_handler);
  battery_state_service_subscribe(prv_battery_handler);
}

static void prv_deinit(void) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
