//! @file countdown_timer.h
//! @brief Logic and data for a timer object
//!
//! Contains all the code to create and maintain a CountdownTimer linked list. Also includes code
//! to modify and format timers into human readable strings.
//!
//! @author Eric D. Phillips
//! @date September 11, 2015
//! @bugs No known bugs

#include <pebble.h>


// Structure for CountdownTimer
typedef struct CountdownTimer CountdownTimer;


////////////////////////////////////////////////////////////////////////////////////////////////////
// API Interface
//

//! Get the current value of a CountdownTimer
//! @param id The unique number identifier for the CountdownTimer
//! @return The current time left on the timer in milliseconds
int64_t countdown_timer_get_current_value(uint32_t id);

//! Get the total length of a CountdownTimer
//! @param id The unique number identifier for the CountdownTimer
//! @return The total duration of a timer in milliseconds
int64_t countdown_timer_get_total_length(uint32_t id);

//! Get the paused state of the CountdownTimer
//! @param id The unique number identifier for the CountdownTimer
bool countdown_timer_get_paused(uint32_t id);

//! Format the CountdownTimer as a human readable string
//! @param id The unique number identifier for the CountdownTimer
//! @param buff The char buff into which to write the formatted string
//! @param size The length of the buff array
void countdown_timer_format_as_string(uint32_t id, char *buff, uint8_t size);

//! Get the current number of CountdownTimers
//! @return An integer value for the current number of CountdownTimers
uint16_t countdown_timer_get_timer_count(void);

//! Get a CountdownTimer's id from its index in the list
//! @param index The index of the CountdownTimer to access
//! @return The id of the CountdownTimer at that index
uint32_t countdown_timer_get_id_from_index(uint32_t index);

//! Create a new CountdownTimer
//! @param length The initial value at which to create the CountdownTimer in milliseconds
//! @return A unique number identifier for the CountdownTimer
uint32_t countdown_timer_create(int64_t length);

//! Destroy a CountdownTimer
//! @param id The unique number identifier for the CountdownTimer
void countdown_timer_destroy(uint32_t id);

//! Initialize CountdownTimer singleton
void countdown_timer_initialize(void);
