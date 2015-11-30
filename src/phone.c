/*
 * NOTE: This is an older version of some code to send pins to the Timeline.
 * I am currently writing a Timeline pin library, and so I am not updating
 * this, as it will be replaced.
 */


#include <pebble.h>
#include "phone.h"
#include "countdown_timer.h"

// AppMessage Keys
#define KEY_DURATION 5
#define KEY_UNIQUEID 10
#define KEY_TOTAL_TIME 15



// ********** Utilities ********** //
// send a pin for the timer
void phone_send_pin(CountdownTimer *countdown_timer) {
  // begin iterator
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  // write data
  dict_write_int32(iter, KEY_UNIQUEID, countdown_timer_get_id(countdown_timer));
  dict_write_int32(iter, KEY_DURATION, countdown_timer_get_current_time(countdown_timer) / 1000);
  dict_write_int32(iter, KEY_TOTAL_TIME, countdown_timer_get_duration(countdown_timer) / 1000);
  dict_write_end(iter);
  // send
  app_message_outbox_send();
}
// delete a pin
void phone_delete_pin(CountdownTimer *countdown_timer) {
  // begin iterator
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  // write data (duration of 0 means delete pin)
  dict_write_int32(iter, KEY_UNIQUEID, countdown_timer_get_id(countdown_timer));
  dict_write_int32(iter, KEY_DURATION, 0);
  dict_write_int32(iter, KEY_TOTAL_TIME, 0);
  dict_write_end(iter);
  // send
  app_message_outbox_send();
}



// ********** Callbacks ********** //
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
// error callbacks
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason,
                                   void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed: %d", (APP_MSG_NOT_CONNECTED == (int)reason));
}



// ********** Initialize/De-initialize ********** //
// start phone connection
void phone_connect(void) {
  // register callbacks
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  // Open AppMessage
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
}

// stop phone connection
void phone_disconnect(void) {
  // unregister callbacks
  app_message_deregister_callbacks();
}
