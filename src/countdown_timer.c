// @file countdown_timer.c
// @brief Logic and data for a timer object
//
// Contains all the code to create and maintain a CountdownTimer linked list. Also includes code
// to modify and format timers into human readable strings.
//
// @author Eric D. Phillips
// @date September 11, 2015
// @bugs No known bugs


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
// API Interface
//

// Format the CountdownTimer as a human readable string
void countdown_timer_format_as_string(uint32_t id, char *buff);

// Create a new CountdownTimer
uint32_t countdown_timer_create(int64_t value);

// Destroy a CountdownTimer
void countdown_timer_destroy(uint32_t id);
