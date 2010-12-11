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

#ifndef __SATURATION_EFFECT_H__
#define __SATURATION_EFFECT_H__
#include "effect.h"
#include <string>

class SaturationEffect: public Effect
{
    public:
        SaturationEffect(Controller *controller) : 
            Effect(controller), 
            contrast_(1.0),
            saturation_(1.0)
        {
            init_properties();
        }
            
    private:
        virtual void init_properties();
        virtual void update_actor(ClutterActor *actor);
        virtual void setup_actor(ClutterActor *actor);
        void on_saturation_changed(std::string &name, float value);
        void on_contrast_changed(std::string &name, float value);
        float contrast_;
        float saturation_;
        ClutterShader *shader_;
};
#endif

