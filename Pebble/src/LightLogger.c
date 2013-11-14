#include "pebble.h"

#define TAG__TAP  0xCAFE
#define TAG__USER 0xBABE


Window *window;
TextLayer *text_date_layer;
TextLayer *text_time_layer;
TextLayer *text_day_layer;
Layer *triangle_layer;

void triangle_layer_update_callback(Layer *layer, GContext* ctx) {
  time_t ct;
  time(&ct);

  static char time_text[] = "0";
  strftime(time_text, sizeof(time_text), "%w", localtime(&ct));

  uint8_t weekday = ((time_text[0] - '0') + 5) % 6;

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(weekday * 18, 0, 8, 3), 0, GCornerNone);
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;


  // TODO: Only update the date when it's changed.
  strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  text_layer_set_text(text_date_layer, date_text);

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);
}

void click_handler(ClickRecognizerRef recognizer, void *context) {
  uint8_t button = click_recognizer_get_button_id(recognizer);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Button pressed: %i", button);
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet tag_value = TupletInteger(0x00, TAG__USER);
  dict_write_tuplet(iter, &tag_value);
  Tuplet time_value = TupletInteger(0x01, time(NULL));
  dict_write_tuplet(iter, &time_value);
  Tuplet button_value = TupletInteger(0x02, button);
  dict_write_tuplet(iter, &button_value);
  dict_write_end(iter);
  uint8_t result = app_message_outbox_send();

  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message not sent");
  }
}
void click_config_provider() {
  window_raw_click_subscribe(BUTTON_ID_UP, click_handler, NULL, NULL);
  window_raw_click_subscribe(BUTTON_ID_DOWN, click_handler, NULL, NULL);
  window_long_click_subscribe(BUTTON_ID_BACK, 500, click_handler, NULL);
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  Tuplet tag_value = TupletInteger(0x00, TAG__TAP);
  dict_write_tuplet(iter, &tag_value);
  Tuplet time_value = TupletInteger(0x01, time(NULL));
  dict_write_tuplet(iter, &time_value);
  Tuplet button_value = TupletInteger(0x02, 0x00);
  button_value.type = TUPLE_UINT;
  dict_write_tuplet(iter, &button_value);
  dict_write_end(iter);
  uint8_t result = app_message_outbox_send();

  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message not sent");
  }
}
void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  accel_tap_service_subscribe(&accel_tap_handler);
}

void handle_init(void) {
  window = window_create();
  window_set_fullscreen(window, true);
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);
  window_set_click_config_provider(window, click_config_provider);

  Layer *window_layer = window_get_root_layer(window);

  text_date_layer = text_layer_create(GRect(8, 32, 144-16, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(text_date_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_time_layer = text_layer_create(GRect(8, 46, 144-16, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(text_time_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  text_day_layer = text_layer_create(GRect(12, 100, 144-12, 168-68));
  text_layer_set_text_color(text_day_layer, GColorWhite);
  text_layer_set_background_color(text_day_layer, GColorClear);
  text_layer_set_font(text_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(text_day_layer));
  text_layer_set_text(text_day_layer, "M  T  W  T  F  S  S");

  GRect triangle_frame = GRect(12, 121, 144-12, 8);
  triangle_layer = layer_create(triangle_frame);
  layer_set_update_proc(triangle_layer, triangle_layer_update_callback);
  layer_add_child(window_layer, triangle_layer);

  TextLayer* correct_activation = text_layer_create(GRect(0, 1, 144 - 5, 20));
  text_layer_set_text_color(correct_activation, GColorWhite);
  text_layer_set_background_color(correct_activation, GColorClear);
  text_layer_set_font(correct_activation, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(correct_activation, GTextAlignmentRight);
  text_layer_set_text(correct_activation, "activation CORRECT");
  layer_add_child(window_layer, text_layer_get_layer(correct_activation));

  TextLayer* missed_activation = text_layer_create(GRect(0, 168 - 20, 144 - 5, 20));
  text_layer_set_text_color(missed_activation, GColorWhite);
  text_layer_set_background_color(missed_activation, GColorClear);
  text_layer_set_font(missed_activation, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(missed_activation, GTextAlignmentRight);
  text_layer_set_text(missed_activation, "activation MISSED");
  layer_add_child(window_layer, text_layer_get_layer(missed_activation));

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  accel_tap_service_subscribe(&accel_tap_handler);

   const uint32_t inbound_size = 1;
   const uint32_t outbound_size = 64;
   app_message_open(inbound_size, outbound_size);
}


int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
