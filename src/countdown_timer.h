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

//! Format the CountdownTimer as a human readable string
//! @param id The unique number identifier for the CountdownTimer
//! @param buff The char buff into which to write the formatted string
void countdown_timer_format_as_string(uint32_t id, char *buff);

//! Create a new CountdownTimer
//! @param length The initial value at which to create the CountdownTimer in milliseconds
//! @return A unique number identifier for the CountdownTimer
uint32_t countdown_timer_create(int64_t length);

//! Destroy a CountdownTimer
//! @param id The unique number identifier for the CountdownTimer
void countdown_timer_destroy(uint32_t id);

//! Initialize CountdownTimer singleton
void countdown_timer_initialize(void);
