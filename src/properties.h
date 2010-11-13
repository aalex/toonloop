#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__
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

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "property.h"

/**
 * Holds many properties identified by their name.
 */
template <typename T> class Properties
{
    public:
        typedef boost::shared_ptr< Property<T> > PropertyPtr;

        void add_property(const std::string &name, T value)
        {
            properties_[name] = PropertyPtr(new Property<T>(name, value));
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

        bool has_property(const std::string &name) const
        {
            return properties_.find(name) != properties_.end();
        }

    private:
        std::map<std::string, PropertyPtr> properties_;
};
#endif
