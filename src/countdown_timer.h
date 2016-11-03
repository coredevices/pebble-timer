/*******************************************************************************
 * FILENAME :        countdown_timer.h
 *
 * DESCRIPTION :
 *      Timer creation, destruction, modification, etc.
 *
 * PUBLIC FUNCTIONS :
 *      CountdownTimer  *countdown_timer_create(int64_t duration);
 *      void            countdown_timer_destroy(CountdownTimer
 *                          *countdown_timer);
 *      void            countdown_timer_start(CountdownTimer *countdown_timer);
 *      void            countdown_timer_stop(CountdownTimer *countdown_timer);
 *      void            countdown_timer_update(CountdownTimer *countdown_timer,
 *                          int64_t duration, bool update_duration);
 *      CountdownTimer  *countdown_timer_check_ended(CountdownTimer
 *                          **timer_array, uint8_t timer_array_count);
 *      void            countdown_timer_list_add(CountdownTimer **timer_array,
 *                          uint8_t timer_array_max, uint8_t *timer_array_count,
 *                          CountdownTimer *countdown_timer);
 *      void            countdown_timer_list_remove(CountdownTimer
 *                          **timer_array, uint8_t *timer_array_count,
 *                          uint8_t timer_index);
 *      int16_t         countdown_timer_list_get_timer_index(CountdownTimer
 *                          **timer_array, uint8_t timer_array_count,
 *                          CountdownTimer *countdown_timer);
 *      CountdownTimer  *countdown_timer_list_get_closest_timer(CountdownTimer
 *                          **timer_array, uint8_t timer_array_count);
 *      CountdownTimer  *countdown_timer_list_get_timer_by_id(
 *                          CountdownTimer **timer_array,
 *                          uint8_t timer_array_count, int32_t id);
 *      void            countdown_timer_list_destroy_all(CountdownTimer
 *                          **timer_array, uint8_t *timer_array_count);
 *      void            countdown_timer_list_save(CountdownTimer **timer_array,
 *                          uint8_t timer_array_count, uint32_t key);
 *      void            countdown_timer_list_load(CountdownTimer **timer_array,
 *                          uint8_t *timer_array_count, uint32_t key);
 *      bool            countdown_timer_get_paused(CountdownTimer
 *                          *countdown_timer);
 *      int64_t         countdown_timer_get_start(CountdownTimer
 *                          *countdown_timer);
 *      int64_t         countdown_timer_get_current_time(CountdownTimer
 *                          *countdown_timer);
 *      void            countdown_timer_rand_id(CountdownTimer
 *                          *countdown_timer);
 *      int32_t         countdown_timer_get_id(CountdownTimer *countdown_timer);
 *      int64_t         countdown_timer_get_duration(CountdownTimer
 *                          *countdown_timer);
 *      void            countdown_timer_format_text(int64_t value,
 *                          char *buff, uint8_t size);
 *      char            *countdown_timer_format_own_buff(CountdownTimer
 *                          *countdown_timer);
 *
 * NOTES :      The actual timer structure definition is not exposed to
 *              prevent direct modification of the structure.
 *
 * AUTHOR :     Eric Phillips        START DATE :    07/09/15
 *
 */

#pragma once

#include <pebble.h>



/*
 * Function:    countdown_timer_get_epoch_ms
 * -----------------------------------------
 * gets the current epoch time in milliseconds
 *
 *  returns: the epoch in ms
 */

int64_t countdown_timer_get_epoch_ms(void);



/*
 * Structure:   CountdownTimer
 * ---------------------------
 * a structure defining the main "timer" type
 */

typedef struct CountdownTimer CountdownTimer;



/*
 * Function:    countdown_timer_create
 * -----------------------------------
 * creates a new CountdownTimer
 *
 *  duration: the length of the new timer in milliseconds
 *  current_id_max: a pointer to the current max id
 *
 *  returns: a pointer to the new timer in memory
 */

CountdownTimer *countdown_timer_create(int64_t duration, int32_t *current_id_max);



/*
 * Function:    countdown_timer_destroy
 * ------------------------------------
 * destroys an existing CountdownTimer
 *
 *  countdown_timer: a pointer to the CountdownTimer being destroyed
 */

void countdown_timer_destroy(CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_start
 * ----------------------------------
 * starts or resumes a CountdownTimer
 *
 *  countdown_timer: a pointer to the CountdownTimer being started
 */

void countdown_timer_start(CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_stop
 * ---------------------------------
 * stops or pauses a CountdownTimer
 *
 *  countdown_timer: a pointer to the CountdownTimer being stopped
 *  current_id_max: a pointer to the current max id
 */

void countdown_timer_stop(CountdownTimer *countdown_timer, int32_t *current_id_max);



/*
 * Function:    countdown_timer_update
 * -----------------------------------
 * updates the duration of a CountdownTimer
 * without changing it's state
 *
 *  countdown_timer: the CountdownTimer to update
 *  duration: the time that the timer should have left
 *  update_duration: set the original duration to the new duration
 */

void countdown_timer_update(CountdownTimer *countdown_timer, int64_t duration,
                            bool update_duration);



/*
 * Function:    countdown_timer_check_ended
 * ----------------------------------------
 * checks through the timers and resets any that have finished
 * also returns a pointer to the first one found overdue
 *
 *  timer_array: array of timers to search through (double pointer)
 *  timer_array_count: the total number of timers in the array
 *
 *  returns: a pointer to the first timer reset
 */

CountdownTimer *countdown_timer_check_ended(CountdownTimer **timer_array,
                                            uint8_t timer_array_count);



/*
 * Function:    countdown_timer_list_add
 * -------------------------------------
 * adds an existing CountdownTimer to an array of timers
 * and allocates more space if necessary
 *
 *  timer_array: the array of timers to be added to (double pointer)
 *  timer_array_size: the size of the array
 *  timer_array_count: pointer to total number of timers in the array
 *  countdown_timer: a pointer to the timer being added
 */

void countdown_timer_list_add(CountdownTimer **timer_array, uint8_t timer_array_max,
                              uint8_t *timer_array_count, CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_list_remove
 * ----------------------------------------
 * removes a CountdownTimer pointer from an array of timers
 * NOTE: this does not destroy the timer, it merely removes its pointer
 *
 *  timer_array: the array of timers to be added to (double pointer)
 *  timer_array_count: pointer to total number of timers in the array
 *  timer_index: the index of the timer pointer to be removed
 */

void countdown_timer_list_remove(CountdownTimer **timer_array, uint8_t *timer_array_count,
                                 uint8_t timer_index);



/*
 * Function:    countdown_timer_list_get_timer_index
 * -------------------------------------------------
 * gets the index of a CountdownTimer pointer in an array of pointers
 *
 *  timer_array: the array of CountdownTimer pointers to search
 *  timer_array_count: the total number of CountdownTimers in the array
 *  countdown_timer: the CountdownTimer pointer to be searching for
 *
 *  returns: the index within the array of the CountdownTimer or -1 if not found
 */

int16_t countdown_timer_list_get_timer_index(CountdownTimer **timer_array,
    uint8_t timer_array_count, CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_list_get_closest_timer
 * ---------------------------------------------------
 * gets the timer with the least time left that is running
 *
 *  timer_array: the array of CountdownTimers to check through (double pointer)
 *  timer_array_count: the number of CountdownTimer in the array
 *
 *  returns: a pointer to the closest to ended CountdownTimer
 */

CountdownTimer *countdown_timer_list_get_closest_timer(CountdownTimer **timer_array,
                                                       uint8_t timer_array_count);



/*
 * Function:    countdown_timer_list_get_timer_by_id
 * -------------------------------------------------
 * gets a CountdownTimer by it's ID
 *
 *  timer_array: a pointer to an array of CountdownTimers (double pointer)
 *  timer_array_count: the number of timers in the array
 *  id: the CountdownTimer ID to search for
 *
 *  returns: a pointer to a CountdownTimer or NULL if none were found to match
 */

CountdownTimer *countdown_timer_list_get_timer_by_id(CountdownTimer **timer_array,
                                                     uint8_t timer_array_count, int32_t id);



/*
 * Function:    countdown_timer_list_destroy_all
 * ---------------------------------------------
 * destroys all CountdownTimers in a list of them
 *
 *  timer_array: the array of timers to destroy
 *  timer_array_count: the number of timers in the array
 */

void countdown_timer_list_destroy_all(CountdownTimer **timer_array, uint8_t *timer_array_count);



/*
 * Function:    countdown_timer_list_save
 * --------------------------------------
 * saves a list of timers to persistent storage
 *
 *  timer_array: the array of CountdownTimers to save (double pointer)
 *  timer_array_count: the number of CountdownTimers in the array
 *  key: the key to save data with, will be indexed each save
 */

void countdown_timer_list_save(CountdownTimer **timer_array, uint8_t timer_array_count,
                               uint32_t key);



/*
 * Function:    countdown_timer_list_load
 * --------------------------------------
 * loads a list of CountdownTimers
 * this also creates memory for the timers
 */

void countdown_timer_list_load(CountdownTimer **timer_array, uint8_t *timer_array_count,
                               uint32_t key);



/*
 * Function:    countdown_timer_get_paused
 * ---------------------------------------
 * gets a value indicating whether the timer is paused or not
 *
 *  countdown_timer: the timer to get the state of
 */

bool countdown_timer_get_paused(CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_get_start
 * -----------------------------------------
 * gets the start time of a CountdownTimer
 *
 *  countdown_timer: the timer to get the start of
 */

int64_t countdown_timer_get_start(CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_get_current_time
 * ---------------------------------------------
 * gets the current remaining CountdownTimer time in milliseconds
 *
 *  countdown_timer: the CountdownTimer to get the time of
 */

int64_t countdown_timer_get_current_time(CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_rand_id
 * ------------------------------------
 * creates a random ID for the CountdownTimer
 *
 *  countdown_timer: the CountdownTimer to give a random ID to
 *  current_id_max: pointer to the current maximum increment of the id
 */

void countdown_timer_rand_id(CountdownTimer *countdown_timer, int32_t *current_id_max);



/*
 * Function:    countdown_timer_get_id
 * -----------------------------------
 * gets the ID of the current CountdownTimer
 *
 *  countdown_timer: the CountdownTimer to get the ID of
 */

int32_t countdown_timer_get_id(CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_get_duration
 * -----------------------------------------
 * get the total duration of the current CountdownTimer in milliseconds
 *
 *  countdown_timer: the CountdownTimer to get the duration of
 */

int64_t countdown_timer_get_duration(CountdownTimer *countdown_timer);



/*
 * Function:    countdown_timer_format_text
 * ----------------------------------------
 * formats the timer's value as text and prints it into the provided buffer
 *
 *  value: an int64_t containing the time in milliseconds to be formated
 *  buff: the buffer to print the text onto
 */

void countdown_timer_format_text(int64_t value, char *buff, uint8_t size);



/*
 * Function:    countdown_timer_format_own_buff
 * --------------------------------------------
 * formats the timer's own current time and prints it on its own buffer
 * also returns a pointer to that buffer
 *
 *  countdown_timer: the timer to modify
 *
 *  returns: a pointer to the formated text
 */

char *countdown_timer_format_own_buff(CountdownTimer *countdown_timer);


CountdownTimer *countdown_timer_list_get_last_updated_timer(CountdownTimer **timer_array,
                                                       uint8_t timer_array_count);

CountdownTimer *countdown_timer_list_get_closest_timer_after(CountdownTimer **timer_array,
                                                       uint8_t timer_array_count,
                                                       CountdownTimer *after_countdown_timer);

time_t countdown_timer_get_last_update(CountdownTimer *countdown_timer);