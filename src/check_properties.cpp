#include "property.h"
#include "properties.h"
#include <string>
#include <map>
#include <iostream>

void cb(std::string &name, int value)
{
    std::cout << name << " changed to " << value << std::endl;
}

void f_cb(std::string &name, float value)
{
    std::cout << name << " changed to " << value << std::endl;
}

int main(int /* argc */, char ** /*argv */)
{
    std::string name("hello");
    Property<int> p(name, 0);
    p.value_changed_signal_.connect(&cb);
    p.set_value(8);

    Properties<int> holder = Properties<int>();
    holder.add_property("bar", 9);
    Property<int> *x = holder.get_property("bar");
    x->value_changed_signal_.connect(&cb);
    //x->register_on_changed_slot(&cb);
    x->set_value(6);
    holder.remove_property("bar");
    Property<int> *val = holder.get_property("bar");
    if (val != 0)
        std::cout << "Should not get a valid pointer " << val << std::endl;
    else
        std::cout << "Good! Did not get a valid pointer " << val << std::endl;

    Property<float> f("float property", 0.0);
    f.value_changed_signal_.connect(&f_cb);
    f.set_value(3.14159);

    return 0;
}
