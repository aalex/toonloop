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

#ifndef __BRCOSA_EFFECT_H__
#define __BRCOSA_EFFECT_H__
#include "effect.h"
#include <string>

/**
 * Brightness, contrast and saturation effect.
 */
class BrCoSaEffect: public Effect
{
    public:
        BrCoSaEffect(Controller *controller);
    private:
        virtual void init_properties();
        virtual void update_actor(ClutterActor *actor);
        virtual void setup_actor(ClutterActor *actor);
        void on_saturation_changed(std::string name, float value);
        void on_contrast_changed(std::string name, float value);
        float contrast_;
        float saturation_;
        ClutterShader *shader_;
};
#endif

