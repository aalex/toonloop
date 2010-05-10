/*
 * The Tween Class for AVR and Arduino
 * 
 * Part of LibInteract for AVR
 * Alexandre Quessy 2008 ~ BSD License (libre software)
 * http://wiki.dataflow.ws/LibInteract
 * 
 * This is a second draft of a C class for motion tweening. It can be used 
 * to create chases with lights, or fades for audio. 
 * 
 * It works on Arduino. Just drop the (used to be) whole "Tween" directory 
 * in your "<arduino directory>/hardware/librairies/" directory. 
 * 
 * The crucial maths are taken from the excellent easing equations from Robert Penner. 
 * Under a BSD-like License:
 * Easing Equations v1.5
 *   (c) 2003 Robert Penner, all rights reserved. May 1, 2003
 *   This work is subject to the terms in http://www.robertpenner.com/easing_terms_of_use.html.  
 */

#include "tween.h"

Tween::Tween()
{
	v = 0;
	c = 0;
	b = 0;
	d = 0;
}

// time_now
float Tween::linearTween(float t) 
{
	// t: current time, b: beginning value, c: change in value, d: duration
	v = ((c * t) / d) + b;
	//c*t/d + b;
	return v;
}
// val, time
void Tween::line(float val, float duration) 
{
	b = v; // beginning = current
	c = val - v; // change in value = target - current
	d = duration;
}

float Tween::tick(float t) 
{
	float ret = 0;
	// if done ( >  duration)
	if (t > d)
	{
		ret = b + c; // arrived
	} 
	else
	{
		switch (tweentype_)
		{
			// call appropriate tween function
			case TWEEN_EASEINOUTCUBIC:
				ret = easeInOutCubic(t);
				break;
			// default
			case TWEEN_LINEAR:
			default:
				ret = linearTween(t); 
				break;
		}
	}
	v = ret; // XXX IMPORTANT
	return ret;
}

void Tween::setType(tween_type newtype) 
{
	switch (newtype)
	{
		// those are ok
		case TWEEN_EASEINOUTCUBIC:
		case TWEEN_LINEAR:
			tweentype_ = newtype;
			break;
		default:
			// call linear tween
			tweentype_ = TWEEN_LINEAR;
			break;
	}
}

// cubic easing in/out - acceleration until halfway, then deceleration
float Tween::easeInOutCubic(float t) 
{
	if ((t /= d/2) < 1) 
	{ 
		return c/2*t*t*t + b;
	}
	return c / 2* ((t-=2)*t*t + 2) + b;
}

