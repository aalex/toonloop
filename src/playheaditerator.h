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
#ifndef __PLAYHEADITERATOR_H__
#define __PLAYHEADITERATOR_H__

#include <string>
/**
 * Moves the playhead position.
 * 
 * Abstract base class for playhead iterators.
 */
class PlayheadIterator
{
    public:
        PlayheadIterator() {}
        const std::string &get_name() const;
        unsigned int iterate(unsigned int current, unsigned int length);
    private:
        virtual const std::string &do_get_name() const = 0;
        virtual unsigned int do_iterate(unsigned int current, unsigned int length) = 0;
};

/**
 * Moves the playhead forward.
 */
class ForwardIterator : public PlayheadIterator
{
    public:
        ForwardIterator() : 
            PlayheadIterator()
        {}
    private:
        // declaration
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual unsigned int do_iterate(unsigned int current, unsigned int length);
};

/**
 * Moves the playhead backward.
 */
class BackwardIterator : public PlayheadIterator
{
    public:
        BackwardIterator() : 
            PlayheadIterator()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual unsigned int do_iterate(unsigned int current, unsigned int length);
};

/**
 * Moves the playhead back and forth.
 */
class YoyoIterator : public PlayheadIterator
{
    public:
        YoyoIterator() :
            PlayheadIterator(),
            yoyo_direction_(1)
        {}
    private:
        /** 
         * Either 1 or -1
         */
        int yoyo_direction_;
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual unsigned int do_iterate(unsigned int current, unsigned int length);
};

/**
 * Moves the playhead randomly.
 */
class RandomIterator : public PlayheadIterator
{
    public:
        RandomIterator() :
            PlayheadIterator()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual unsigned int do_iterate(unsigned int current, unsigned int length);
};

/**
 * Moves the playhead with random steps.
 */
class DrunkIterator : public PlayheadIterator
{
    public:
        DrunkIterator() :
            PlayheadIterator()
        {}
    private:
        static const std::string name_;
        virtual const std::string &do_get_name() const { return name_; }
        virtual unsigned int do_iterate(unsigned int current, unsigned int length);
};
#endif

