/**
 * Depends on the "shaders/brcosa.glsl" fragment shader file.
 */
#include <boost/bind.hpp>
#include <clutter/clutter.h>
#include "saturationeffect.h"
#include "glslang.h"
#include "controller.h"
#include "unused.h"

//TODO: check if verbose

void BrCoSaEffect::update_actor(ClutterActor *actor)
{
    if (! loaded_)
        return;
    //g_print("%s: set actor %s shader params with saturation=%f contrast=%f\n", __FUNCTION__, clutter_actor_get_name(actor), saturation_, contrast_);
    clutter_actor_set_shader_param_float(actor, "saturation", saturation_);
    clutter_actor_set_shader_param_float(actor, "contrast", contrast_);
    clutter_actor_set_shader_param_float(actor, "brightness", 1.0);
    clutter_actor_set_shader_param_float(actor, "alpha", 1.0);
    clutter_actor_set_shader_param_float(actor, "opacity", 0.0);
    clutter_actor_set_shader_param_int(actor, "image", 0);

    //GValue value;
    //g_value_init(&value, CLUTTER_TYPE_SHADER_FLOAT);
    //gfloat avgluma[] = {1.0, 1.0, 1.0};
    //clutter_value_set_shader_float(&value, 3, avgluma);
    //clutter_actor_set_shader_param(actor, "avgluma", &value);
    //g_value_unset(&value);
}

void BrCoSaEffect::init_properties()
{
    shader_ = NULL;
    shader_ = clutter_shader_new();
    gchar *file_name = toon_find_shader_file("brcosa.glsl");
    if (! file_name)
        g_critical("Could not fine shader file %s.", "brcosa.glsl");
    toon_load_fragment_source_file(shader_, file_name);
    g_free(file_name);
    GError *error = NULL;
    clutter_shader_compile(shader_, &error);
    if (error)
    {
        g_warning("Unable to init shader: %s", error->message);
        g_error_free(error);
    }
    else
        loaded_ = true;

    // TODO:2010-12-11:aalex:init_preperties should be done anyways, but loading the shader might be facultative. (in case shaders are not supported or desired)
    // Those two actions should be done in two separate virtual methods. (wrapped in a non-virtual interface)
    if (loaded_)
    {
        //g_print("Creating float property %s\n", "saturation");
        controller_->add_float_property("fx.brcosa.saturation", saturation_)->value_changed_signal_.connect(boost::bind(&BrCoSaEffect::on_saturation_changed, this, _1, _2));
        //g_print("Creating float property %s\n", "contrast");
        controller_->add_float_property("fx.brcosa.contrast", contrast_)->value_changed_signal_.connect(boost::bind(&BrCoSaEffect::on_contrast_changed, this, _1, _2));
    }
}

void BrCoSaEffect::setup_actor(ClutterActor *actor)
{
    if (! loaded_)
    {
        g_critical("BrCoSaEffect not loaded yet");
        return;
    }
    //g_print("%s: add shader %s to actor %s\n", __FUNCTION__, "BrCoSaEffect", clutter_actor_get_name(actor));
    clutter_actor_set_shader(actor, shader_);
}

void BrCoSaEffect::on_contrast_changed(std::string &name, float value)
{
    UNUSED(name);
    contrast_ = value;
    update_all_actors();
}
void BrCoSaEffect::on_saturation_changed(std::string &name, float value)
{
    UNUSED(name);
    saturation_ = value;
    update_all_actors();
}

