/*
 * Toonloop
 *
 * Copyright 2010 Alexandre Quessy
 * <alexandre@quessy.net>
 * http://www.toonloop.com
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
#ifndef __TWEEN_H__
#define __TWEEN_H__

// TODO: enclose in a namespace
enum tween_type 
{
    TWEEN_LINEAR,
    TWEEN_EASEINOUTCUBIC
};

// TODO add more tweentypes
/** Motion tweening to create motion interpolation.
 */
//TODO: Clutter probably offers us better stuff.
 
class Tween
{
	// FIXME TODO add start time / current time
    public:
        Tween();
        // Tween_tick should be renamed (since it doesnt manage time)
        float tick(float t); // calls the proper tween func
        float linearTween(float t);
        void  line(float val, float duration);
        void  setType(tween_type newtype);
        float easeInOutCubic(float t);

    private:
        // t: current time, b: beginning value, c: change in value, d: duration
        float b;
        float c;
        float d;
        float v; // current value
        tween_type tweentype_;
};

#endif

