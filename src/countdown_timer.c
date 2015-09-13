// @file countdown_timer.c
// @brief Logic and data for a timer object
//
// Contains all the code to create and maintain a CountdownTimer linked list. Also includes code
// to modify and format timers into human readable strings.
//
// @author Eric D. Phillips
// @date September 11, 2015
// @bugs No known bugs

#include "countdown_timer.h"
#include "utility.h"


// Main singleton data structure for all CountdownTimers
struct {
  CountdownTimer  *head;    //< Pointer to first CountdownTimer in linked list
} countdown_timer_data;

// Structure for CountdownTimer
typedef struct CountdownTimer {
  uint32_t        id;       //< A unique number identifier for the timer
  int64_t         epoch;    //< The epoch when the timer was started in milliseconds
  int64_t         length;   //< The total length of the timer in milliseconds

  CountdownTimer  *next;    //< Pointer to next item in CountdownTimer linked list
} CountdownTimer;


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions
//

// Find a CountdownTimer by its id
// Optionally override the id and find the last CountdownTimer
static CountdownTimer *prv_countdown_timer_find_by_id(uint32_t id, bool find_last) {
  // if nodes exist
  if (countdown_timer_data.head) {
    // find node in linked list
    CountdownTimer *last_timer = countdown_timer_data.head;
    while (last_timer) {
      // check id or last timer
      if (last_timer->id == id && !find_last) {
        return last_timer;
      } else if (!last_timer->next && find_last) {
        return last_timer;
      }
      // index to next node
      last_timer = last_timer->next;
    }
  }

  // no timers were found
  return NULL;
}

// Recursively find and remove a node from the list
static CountdownTimer *prv_countdown_timer_recursive_removal(CountdownTimer *current_node,
                                                      uint32_t target_id) {
  // return if current node is null
  if (!current_node) {
    return NULL;
  }
  // check if target node
  if (current_node->id == target_id) {
    CountdownTimer *next_node = current_node->next;
    free(current_node);
    return next_node;
  }
  // call self
  CountdownTimer *node = prv_countdown_timer_recursive_removal(current_node->next, target_id);
  current_node->next = node;
  return current_node;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// API Interface
//

// Format the CountdownTimer as a human readable string
void countdown_timer_format_as_string(uint32_t id, char *buff) {
  // if nodes exist
  if (countdown_timer_data.head) {
    // find node in linked list
    CountdownTimer *last_timer = countdown_timer_data.head;
    while (last_timer) {
      // print to logs
      printf("Timer: %d  %d  %d", (int)last_timer->id, (int)last_timer->epoch, (int)
        last_timer->length);
      // index to next node
      last_timer = last_timer->next;
    }
  }
}

// Create a new CountdownTimer
uint32_t countdown_timer_create(int64_t length) {
  // allocate memory with success check
  CountdownTimer *countdown_timer = MALLOC(sizeof(CountdownTimer));

  // set timer properties
  countdown_timer->id = (uint32_t)time(NULL);
  countdown_timer->epoch = epoch();
  countdown_timer->length = length;
  countdown_timer->next = NULL;

  // get last timer
  CountdownTimer *last_timer = prv_countdown_timer_find_by_id(0, true);
  if (last_timer) {
    last_timer->next = countdown_timer;
  } else {
    countdown_timer_data.head = countdown_timer;
  }

  // return the id
  return countdown_timer->id;
}

// Destroy a CountdownTimer
void countdown_timer_destroy(uint32_t id) {
  // begin recursive search and removal of that CountdownTimer
  countdown_timer_data.head = prv_countdown_timer_recursive_removal(countdown_timer_data.head, id);
}

// Initialize CountdownTimer singleton
void countdown_timer_initialize(void){
  countdown_timer_data.head = NULL;
}
