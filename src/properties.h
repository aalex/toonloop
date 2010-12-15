#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__
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

#include <map>
// TODO: use tr1:unordered_map, since lookup is faster with it
#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "property.h"

/**
 * Holds many properties identified by their name.
 */
template <typename T> class Properties
{
    public:
        typedef boost::shared_ptr< Property<T> > PropertyPtr;

        Property<T> *add_property(const std::string &name, T value)
        {
            // TODO: check if we have it.
            if (has_property(name))
            {
                std::cout << "Warning: property \"" << name << 
                    "\" already exists" << std::endl;
                return 0;
            }
            properties_[name] = PropertyPtr(new Property<T>(name, value));
            return get_property(name);
        }
        
        void remove_property(const std::string &name)
        {
            properties_.erase(name);
        }

        Property<T> *get_property(const std::string &name) const
        {
            if (properties_.find(name) == properties_.end())
                return 0;
            return properties_.find(name)->second.get();
        }

        bool set_property_value(const std::string &name, T value)
        {
            if (not has_property(name))
            {
                std::cout << "No such property \"" << name << "\"" << std::endl;
                return false;
            }
            else
                properties_[name]->set_value(value);
            return true;
        }

        T get_property_value(const std::string &name)
        {
            if (not has_property(name))
            {
                std::cout << "No such property \"" << name << "\"" << std::endl;
                return 0;
            }
            else
                return properties_[name]->get_value();
        }

        bool has_property(const std::string &name) const
        {
            return properties_.find(name) != properties_.end();
        }

    private:
        std::map<std::string, PropertyPtr> properties_;
};
#endif
