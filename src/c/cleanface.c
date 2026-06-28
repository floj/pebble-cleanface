#include <pebble.h>
#include <locale.h>

static Window *s_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;
static GFont s_font_notosans_bold_49;
static GFont s_font_notosans_regular_21;

static char s_time_buf[6];
static char s_date_buf[32];
static int s_battery_percent;

static void prv_update_time(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  strftime(s_time_buf, sizeof(s_time_buf), "%H:%M", t);
  text_layer_set_text(s_time_layer, s_time_buf);
  char day_name[12];
  char day_month[16];
  strftime(day_name, sizeof(day_name), "%A", t);
  strftime(day_month, sizeof(day_month), "%d %B", t);
  snprintf(s_date_buf, sizeof(s_date_buf), "%s\n%s", day_name, day_month);
  text_layer_set_text(s_date_layer, s_date_buf);
}

static void prv_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  prv_update_time();
}

static void prv_battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  int bar_width = (s_battery_percent * (bounds.size.w - 2)) / 100;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(1, 1, bar_width, bounds.size.h - 2), 0, GCornerNone);
}

static void prv_battery_handler(BatteryChargeState charge) {
  s_battery_percent = charge.charge_percent;
  layer_mark_dirty(s_battery_layer);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  window_set_background_color(window, GColorWhite);

  // Load custom fonts
  s_font_notosans_bold_49 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NOTOSANS_BOLD_49));
  s_font_notosans_regular_21 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_NOTOSANS_REGULAR_21));

  // Time layer — large, centered
  int time_h = 56;
  int time_y = (bounds.size.h - time_h) / 2 - 10;
  s_time_layer = text_layer_create(GRect(0, time_y, bounds.size.w, time_h));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_font_notosans_bold_49);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Date layer — bottom of screen
  s_date_layer = text_layer_create(GRect(0, bounds.size.h - 52, bounds.size.w, 52));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, s_font_notosans_regular_21);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // Battery layer — bar across top
  s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, 10));
  layer_set_update_proc(s_battery_layer, prv_battery_update_proc);
  layer_add_child(window_layer, s_battery_layer);

  // Show initial values
  prv_update_time();
  prv_battery_handler(battery_state_service_peek());
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  fonts_unload_custom_font(s_font_notosans_bold_49);
  fonts_unload_custom_font(s_font_notosans_regular_21);
}

static void prv_init(void) {
  setlocale(LC_ALL, "");
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
