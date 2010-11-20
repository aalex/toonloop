#include <clutter/clutter.h>
#include "saturationeffect.h"
#include "glslang.h"
#include "controller.h"
#include "unused.h"

void SaturationEffect::update_actor(ClutterActor *actor)
{
    if (! loaded_)
        return;
    clutter_actor_set_shader_param_float(actor, "saturation", 0.0);
    clutter_actor_set_shader_param_float(actor, "contrast", 1.0);
    clutter_actor_set_shader_param_float(actor, "brightness", 1.0);
    clutter_actor_set_shader_param_float(actor, "alpha", 1.0);
    clutter_actor_set_shader_param_float(actor, "opacity", 0.0);
    clutter_actor_set_shader_param_int(actor, "image", 0);

    GValue value;
    g_value_init(&value, CLUTTER_TYPE_SHADER_FLOAT);
    gfloat avgluma[] = {1.0, 1.0, 1.0};
    clutter_value_set_shader_float(&value, 3, avgluma);
    clutter_actor_set_shader_param(actor, "avgluma", &value);
    g_value_unset(&value);
}

void SaturationEffect::init_properties()
{
    ClutterShader *shader = NULL;
    shader = clutter_shader_new();
    gchar *file_name = toon_find_shader_file("frag.brcosa.glsl");
    toon_load_fragment_source_file(shader, file_name);
    g_free(file_name);
    GError *error = NULL;
    clutter_shader_compile(shader, &error);
    if (error)
    {
        g_warning("Unable to init shader: %s", error->message);
        g_error_free(error);
    }
    else
        loaded_ = true;
}
