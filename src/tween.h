#ifndef __TWEEN_H__
#define __TWEEN_H__

// TODO: enclose in a namespace
enum tween_type 
{
    TWEEN_LINEAR,
    TWEEN_EASEINOUTCUBIC
};

// TODO add more tweentypes

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

