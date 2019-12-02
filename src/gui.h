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

#ifndef __GUI_H__
#define __GUI_H__

#include <GL/glx.h>
#include <clutter/clutter.h>
#include <tr1/memory> // for shared_ptr
#include <vector>
#include "infowindow.h"
#include "timer.h"

class Application;
class Effect;

// TODO:2010-10-05:aalex:Should we put that in a namespace or in the Gui class?
const unsigned int NUM_LAYOUTS = 4;

/** This graphical user interface uses GTK and Clutter-GST.
 */
class Gui
{
    public:
        enum layout_number {
            LAYOUT_SPLITSCREEN,
            LAYOUT_OVERLAY,
            LAYOUT_PLAYBACK_ONLY,
            LAYOUT_PORTRAIT,
            LAYOUT_LIVEFEED_ONLY,
        };
        enum BlendingMode {
            BLENDING_MODE_NORMAL = 0,
            BLENDING_MODE_ADDITIVE
        };
        ClutterActor* get_live_input_texture() const;
        Gui(Application* owner); 
        ~Gui();
        void toggleFullscreen();
        void resize_actors();
        void makeFullscreen();
        void makeUnfullscreen();
        layout_number get_layout() const { return current_layout_; }
        void set_layout(layout_number layout);
        void toggle_layout();
        void set_blending_mode(BlendingMode mode);
        BlendingMode get_blending_mode() const { return blending_mode_; }
        static std::string get_blending_mode_name(BlendingMode mode);
        static std::string get_layout_name(layout_number layout);
        void show();
        
    private:
        void crossfade_increment(float value);
        static void on_live_input_texture_size_changed(ClutterTexture *texture, gint width, gint height, gpointer user_data);
        void on_next_image_to_play(unsigned int clip_number, unsigned int image_number, std::string file_name);
        void on_frame_added(unsigned int clip_number, unsigned int image_number);
        static void on_delete_event(ClutterStage* stage, ClutterEvent* event, gpointer user_data);
        static gboolean key_press_event(ClutterActor *stage, ClutterEvent *event, gpointer user_data);
        static gboolean on_mouse_button_event(ClutterActor *actor, ClutterEvent *event, gpointer user_data);
        static void on_fullscreen(ClutterStage *stage, gpointer user_data);
        static void on_unfullscreen(ClutterStage *stage, gpointer user_data);
        static void on_render_frame(ClutterTimeline * timeline, gint msecs, gpointer user_data);
        void on_blending_mode_int_property_changed(std::string name, int value);
        void on_crossfade_ratio_changed(std::string name, float value);
        void on_livefeed_opacity_changed(std::string name, int value);
        void on_black_out_changed(std::string name, int value);
        void on_black_out_opacity_changed(std::string name, int value);
        void on_playback_opacity_changed(std::string name, int value);
        void on_save_clip(unsigned int clip_number, std::string file_name);
        void on_save_project(std::string file_name);
        void enable_onionskin(bool value);
        void set_onionskin_opacity(int value);
        void hideCursor();
        void showCursor();
        void update_info_text();
        void toggle_info();
        void toggle_help();
        void animate_progress_bar();
        void reset_progress_bar();
        void animate_flash();
        /**
         * Loads all the effects and shaders.
         */
        void load_effects();
        /**
         * Inits all the effects and shaders once the ClutterActors are created.
         */
        void apply_effects();
        /** 
         * Sets the window icon.
         * (not used right now, since we use GTK)
         */
        void set_window_icon(const std::string &path);
        
        float video_input_width_;
        float video_input_height_;
        Application* owner_;
        bool isFullscreen_;
        ClutterActor *stage_;
        ClutterActor *playback_group_;
        ClutterActor *onionskin_group_;
        ClutterActor *live_input_texture_;
        ClutterActor *flash_actor_;
        std::vector< ClutterActor* > playback_textures_;
        std::vector< ClutterActor* > onionskin_textures_;
        ClutterActor *info_text_actor_;
        ClutterActor *help_text_actor_;
        ClutterActor *status_text_actor_;
        ClutterActor *progress_bar_actor_;
        ClutterActor *status_group_;
        ClutterActor *black_out_rectangle_;
        ClutterTimeline *timeline_;
        layout_number current_layout_;
        int onionskin_opacity_;
        bool onionskin_enabled_;
        bool enable_info_;
        bool enable_help_;
        /**
         * How long the fade between each frame lasts.
         * 1.0 means that it's going to last one (playback) frame.
         * Can be up to 10.0 or so.
         */
        float crossfade_ratio_;
        Timer fps_calculation_timer_;
        int number_of_frames_in_last_second_; // counting FPS
        int rendering_fps_;
        static const int WINWIDTH = 640;
        static const int WINHEIGHT = 480;
        BlendingMode blending_mode_;
        std::tr1::shared_ptr<Effect> saturation_effect_; // TODO: have a vector of effects
        InfoWindow info_window_;
        bool is_shown_;
};

#endif // __GUI_H__
