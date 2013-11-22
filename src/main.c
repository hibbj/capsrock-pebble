#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x08, 0xB8, 0xED, 0x0C, 0xD3, 0xC1, 0x49, 0xC6, 0x88, 0xBA, 0xAD, 0x34, 0xF0, 0x73, 0x2E, 0x94 }
PBL_APP_INFO(MY_UUID,
             "Hello World!!", "CAPSROCK",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

enum {
  CMD_KEY_RCV = 0x0, // TUPLE_INTEGER
  CMD_KEY_SEND = 0x1, 
};

char* CMD_SELECT = "Select";
char* CMD_UP = "Up";
char* CMD_DOWN = "Down";

Window window;
TextLayer hello_layer;
char* global_msg;

//----------------------- Text Message Generator Function --------------------------
void disp_msg(char* msg){	
  text_layer_set_text(&hello_layer, msg);
  layer_mark_dirty((Layer*)&hello_layer);
}

//----------------------- Initializing text layer ----------------------------------
void init_layer(){
  text_layer_init(&hello_layer, GRect(0, 0, 144, 144));
  text_layer_set_text_alignment(&hello_layer, GTextAlignmentCenter);
  text_layer_set_font(&hello_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(&window.layer, &hello_layer.layer);
  disp_msg("Hello World!");
}

//---------------------- Sending String Command to Phone ----------------------------
static void send_char_cmd(char* cmd) {
  Tuplet value = TupletCString(CMD_KEY_SEND, cmd);
  
  DictionaryIterator *iter;
  app_message_out_get(&iter);
  
  if (iter == NULL)
    return;
  
  dict_write_tuplet(iter, &value);
  dict_write_end(iter);
  
  app_message_out_send();
  app_message_out_release();
}


//--------------------------- Click Handlers -------------------------------------
void select_single_click_handler(ClickRecognizerRef recognizer, Window *window) {
  //Single Click recognized
  send_char_cmd(global_msg);
  vibes_short_pulse();
}
void select_multi_click_handler(ClickRecognizerRef recognizer, Window *window) {
  //count keeps track of number of clicks
  const uint16_t count = click_number_of_clicks_counted(recognizer);
  //Multiple clicks recognized
}
void select_long_click_handler(ClickRecognizerRef recognizer, Window *window) {
  //Long 'click' recognized
}
void select_long_click_release_handler(ClickRecognizerRef recognizer, Window *window) {
  //Long click 'released' recognized
}

//---------------------- AppMessage Handlers --------------------------------------
void my_out_sent_handler(DictionaryIterator *sent, void *context) {
  // outgoing message was delivered
	disp_msg("Message Sent!");
}
void my_out_fail_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  // outgoing message failed
	disp_msg("Couldn't send message...");
}
void my_in_rcv_handler(DictionaryIterator *received, void *context) {
	Tuple *text_tuple = dict_find(received, CMD_KEY_RCV);
	global_msg = text_tuple->value->cstring;
	disp_msg(global_msg);
	vibes_short_pulse();
}
void my_in_drp_handler(void *context, AppMessageResult reason) {
    disp_msg("Didn't get the message...");
    vibes_short_pulse();
}

//------------------ Button Config Provider Function ---------------------------------
void config_provider(ClickConfig **config, Window *window) {
  // single click / repeat-on-hold config:
  config[BUTTON_ID_SELECT]->click.handler = (ClickHandler) select_single_click_handler;
  config[BUTTON_ID_SELECT]->click.repeat_interval_ms = 300; // "hold-to-repeat" gets overridden if there's a long click handler configured!
  // multi click config:
  config[BUTTON_ID_SELECT]->multi_click.handler = (ClickHandler) select_multi_click_handler;
  config[BUTTON_ID_SELECT]->multi_click.min = 2;
  config[BUTTON_ID_SELECT]->multi_click.max = 10;
  config[BUTTON_ID_UP]->multi_click.last_click_only = true;
  // long click config:
  config[BUTTON_ID_SELECT]->long_click.handler = (ClickHandler) select_long_click_handler;
  config[BUTTON_ID_SELECT]->long_click.release_handler = (ClickHandler) select_long_click_release_handler;
  config[BUTTON_ID_SELECT]->long_click.delay_ms = 700;
}

//------------------------- Initializing App -------------------------------------
void handle_init(AppContextRef ctx) {
  (void)ctx;
  
  window_init(&window, "Text Box");
  window_stack_push(&window, true /* Animated */);
  window_set_click_config_provider(&window, (ClickConfigProvider) config_provider);
  init_layer();
}

//-------------------------- Main Function ----------------------------------------
void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	.messaging_info = {
      .buffer_sizes = {
        .inbound = 64, // inbound buffer size in bytes
        .outbound = 64, // outbound buffer size in bytes
      },
	  .default_callbacks.callbacks = {
        .out_sent = my_out_sent_handler,
        .out_failed = my_out_fail_handler,
        .in_received = my_in_rcv_handler,
        .in_dropped = my_in_drp_handler,
      },
    },
  };
  app_event_loop(params, &handlers);
}
