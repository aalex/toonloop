/*
 * Toonloop
 *
 * Copyright (c) 2010 Alexandre Quessy <alexandre@quessy.net>
 * Copyright (c) 2010 Tristan Matthews <le.businessman@gmail.com>
 *
 * Toonloop is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Toonloop is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the gnu general public license
 * along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "timing.h"
#include "timer.h"

/** 
 * Timer using floats
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

