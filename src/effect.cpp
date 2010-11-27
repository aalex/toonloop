
#include <clutter/clutter.h>
#include "effect.h"

void Effect::add_actor(ClutterActor *actor)
{
    actors_ = g_list_append(actors_, actor);        
    setup_actor(actor);
}

void Effect::update_all_actors()
{
    GList *iter = NULL;
    for (iter = g_list_first(actors_); iter; iter = g_list_next(iter))
    {
        update_actor(CLUTTER_ACTOR(iter->data));
    }
}
