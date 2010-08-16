/* 
 * Timer class
 */
#include "timing.h"
#include "timer.h"

/** 
 * Float timer
 */
Timer::Timer()
{
    reset();
}
/**
 * Reset the start time of the timer to now.
 */
void Timer::reset()
{
    start_time_ = timing::get_timestamp_now();
    now_ = start_time_;
}
/**
 * Updates the current time.
 * Returns the current time.
 */
float Timer::tick()
{
    now_ = timing::get_timestamp_now();
    return now_;
}
/**
 * Returns how many seconds - with float precision - has elapsed since the creation of this object.
 * Note that you must call tick() prior to call this method if you want the current time.
 */
float Timer::get_elapsed()
{
    return (now_ - start_time_) * 0.000001;
}

