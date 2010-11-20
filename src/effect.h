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

#ifndef __EFFECT_H__
#define __EFFECT_H__

#include <clutter/clutter.h>

class Controller;

// typedefs for some GLSL base types.
//typedef float[3] vec3;
//typedef float[4] vec4;

/**
 * Toonloop base class for effects
 * 
 * This is only a prototype - for now.
 */
class Effect
{
    public:
        /**
         * Registers properties to the Controller.
         * 
         * One should change the properties for this effect using the controller's interface.
         * Each property name must be unique.
         */
        Effect(Controller *controller) :
            loaded_(false),
            controller_(controller)
        {
            actors_ = NULL;
        }
        void add_actor(ClutterActor *actor);
        void update_all_actors();
    protected:
        bool loaded_;
    private:
        Controller *controller_;
        GList *actors_;
        virtual void update_actor(ClutterActor *actor) = 0;
        virtual void init_properties() = 0;
};

#endif

