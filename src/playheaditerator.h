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
#endif
