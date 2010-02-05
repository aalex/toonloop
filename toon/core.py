#!/usr/bin/env python
#
# Toonloop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# Toonloop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Toonloop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
#
"""
Toonloop is a live stop motion performance tool. 

The idea is to spread its use for teaching new medias to children and to 
give a professional tool for movie creators.

In the left window, you can see what is seen by the live camera.
In the right window, it is the result of the stop motion loop.

INSTALLATION NOTES : 
The camera module for pygame is available from pygame's svn revision 
1744 or greater
svn co svn://seul.org/svn/pygame/trunk

The startup file to execute is toonloop
"""
# keep this in sync with man_toonloop.txt
INTERACTIVE_HELP = """Toonloop interactive keyboard controls :
 - Press the SPACE bar to grab a frame.
 - Press DELETE or BACKSPACE to delete the last frame.
 - Press 'r' to reset and start the current sequence. 
   (and remove all its frames)
 - Press 's' to save the current sequence as a Motion-JPEG movie.
 - Press 'i' to print current loop frame number, number of frames in loop 
   and global framerate. It toggles on/off a head-up display of text.
 - Press 'h' to print a help message.
 - Press UP to increase frame rate.
 - Press DOWN to decrease frame rate.
 - Press 'a' to toggle on/off the auto recording.
   (it records one frame on every frame) It is an intervalometer.
   Best used to create timelapse clips automatically.
 - Press 'k' or 'j' to increase or decrease the auto recording rate.
 - Press 'f' or ESCAPE to toggle fullscreen mode.
 - Press CTRL-'q' to quit.
 - Press '.' to change the graphical theme.
 - Press TAB to change the playback direction.
 - Press a number from 0 to 9 to switch to a different clip number.
 - Press 'p' to pause playback.
 - Press 'o' to toggle the onion skinning on/off.
 - Press 'b' to take a snapshot as the new background image.
 - Press 'n' to select the next effect available."""
import gc 
import sys
from time import strftime
import os
import glob
import pprint

# ignore (mostly nevow) warnings
import warnings
from exceptions import DeprecationWarning
warnings.simplefilter("ignore", DeprecationWarning)

from twisted.internet import reactor
from twisted.internet import defer

from rats import render
from rats import sig
try:
    from toon import opensoundcontrol
    OSC_LOADED = True
except ImportError, e:
    print("For OSC support, please install pyliblo.")
    OSC_LOADED = False
from toon import mencoder
from toon import draw
from toon import puredata
from toon import midi
from toon import save
from toon import sampler
from toon import data
from toon import fx
try:
    from rats import statesaving
    STATESAVING_LOADED = True
except ImportError, e:
    print("For state saving support, please install the python-simplejson package.")
    print(e.message)
    STATESAVING_LOADED = False

import pygame
import pygame.camera
import pygame.locals as PYGM # no namespace contamination
from OpenGL import GL # no namespace contamination

PACKAGE_DATA_PATH = os.path.dirname(data.__file__)
DIRECTION_FORWARD = "forward"
DIRECTION_BACKWARD = "backward"
DIRECTION_YOYO = "yoyo" # back and forth
THEME_SPLIT_SCREEN = "split_screen"
THEME_PICTURE_IN_PICTURE = "picture_in_picture"
THEME_PLAYBACK_PICTURE_IN_PICTURE = "playback_picture_in_picture"

class ToonloopError(Exception):
    """
    Any error Toonloop might encounter
    """
    pass

class Configuration(object): #Serializable):
    """
    Configuration options.
    
    Default values are defined here. 
    Overriden as **argd arguments to Toonloop(**argd)
    
    Python allows commas at the end of lists and tuples
    """
    def __init__(self, **argd): 
        # basics
        self.verbose = False
        self.video_device = 0 
        
        # project
        self.toonloop_home = os.path.expanduser("~/Documents/toonloop")
        self.config_file = os.path.expanduser("~/.toonloop.json")
        self.project_name = "new_project" # name of the folder
        #self.project_file = 'project.txt'
        self.max_num_clips = 10
        self.delete_jpeg = False
        
        # framerate
        self.framerate_min = 1
        self.framerate_max = 30
        # TODO: framerate value

        # image size
        self.image_width = 320 # 640
        self.image_height = 240 # 480
        self.image_flip_horizontal = False
        #self.playback_opacity = 0.3
        #self.max_num_frames = 1000
        
        # window
        self.display_width = self.image_width * 2 # 640 , was 1024
        self.display_height = self.image_height * 2 # 480 , was 768
        self.display_fullscreen = False
        self.display_theme = THEME_SPLIT_SCREEN
        
        # fudi puredata interface
        self.fudi_enabled = False
        self.fudi_receive_port = 15555
        self.fudi_send_port = 17777
        self.fudi_send_host = 'localhost'
        
        # osc
        self.osc_enabled = False
        self.osc_send_port = 7770
        self.osc_send_host = 'localhost'
        self.osc_listen_port = 7772
        self.osc_verbose = False
        self.osc_sampler_enabled = False
        self.osc_sampler_num_sounds = 500
        #self.osc_receive_hosts = ''
        
        # hud infos (text over images)
        self.hud_enabled = False
        self.hud_enabled = False

        # onionskin
        self.onionskin_enabled = True
        self.onionskin_on = False
        self.onionskin_opacity = 0.3
        
        # background
        self.bgimage_enabled = False
        
        # effects
        self.effect_name = "None"
        self.effects_enabled = False
        
        # intervalometer
        self.intervalometer_on = False 
        self.intervalometer_enabled = True
        self.intervalometer_rate_seconds = 10.0 # in seconds
        # TODO: clean intervalometer on/enabled stuff
        
        # autosave
        self.autosave_on = False 
        self.autosave_enabled = True
        self.autosave_rate_seconds = 600.0 # in seconds

        # midi
        self.midi_enabled = False
        self.midi_verbose = True
        self.midi_input_id = -1 # means default
        self.midi_pedal_control_id = 64
        self.midi_note_record = 41 
        self.midi_note_clear = 43

        # overrides some attributes whose defaults and names are below.
        self.__dict__.update(**argd) 
        # this one is special : it needs other values to set itself. Let's find a way to prevent this

    def save(self):
        """
        Save to JSON config file.

        Uses self.config_file as a file to save to.
        """
        exclude_list = ["toonloop_home", "config_file"]
        if STATESAVING_LOADED:
            data = {}
            for key in sorted(self.__dict__): # FIXME: does not work!
                value = self.__dict__[key]
                if key in exclude_list:
                    if self.verbose:
                        print("Excluding %s since it is in the exclude list." % (key))
                else:
                    data[key] = value
            if self.verbose:
                print("Saving config to %s" % (self.config_file))
            try:
                statesaving.save(self.config_file, data)
            except statesaving.StateSavingError, e:
                if self.verbose:
                    print(e.message)
            else:
                print("Saved config file to %s." % (self.config_file))
        else:
            if self.verbose:
                print("Could not save config. Json is not loaded.")
    
    def load(self):
        """
        Load from JSON config file.

        Uses self.config_file as a file to load from.
        """
        # TODO: fix the bugs it can create. (by saving/loading only vars that are safe)
        # FIXME: not used now.
        if STATESAVING_LOADED: # if found json module
            try:
                data = statesaving.load(self.config_file)
            except statesaving.StateSavingError, e:
                if self.verbose:
                    print("Could not load configuration file \"%s\". %s" % (self.config_file, e.message))
            else:
                print("Loading configuration values from \"%s\"." % (self.config_file))
                self.__dict__.update(data)
        else:
            if self.verbose:
                print("Could not load config. Json is not loaded.")
        
    def print_values(self):
        for k in sorted(self.__dict__):
            v = self.__dict__[k]
            print("    -o %s %s" % (k, v))

    def set(self, name, value):
        """
        Casts to its type and sets the value.

        Intended to be used even from ASCII string values. (FUDI, etc.)

        A bool value can be set using 'True' or 1 and 'False' or 0
        """
        # try:
        kind = type(self.__dict__[name])
        if kind is bool:
            if value == 'True':
                casted_value = True
            elif value == 'False':
                casted_value = False
            else:
                casted_value = bool(int(value))
        else:
            casted_value = kind(value)
        self.__dict__[name] = casted_value
        if self.verbose:
            print('Setting config option \"%s\" to \"%s\" (%s)' % (name, self.__dict__[name], kind.__name__))
        return kind
        # except Exception, e:
        #    print e.message

class ToonClip(object): #Serializable):
    """
    Toonloop clip.
    A clip is a serie of frames. 
    """
    def __init__(self, id, **argd):
        """
        :param id: int 
        """
        self.id = id
        self.playhead = 0 # between 0 and n - 1
        self.playhead_previous = 0 # previous position of the playhead
        self.playhead_iterate_every = 1
        self.direction = DIRECTION_FORWARD
        # Ratio that decides of the framerate
        # self.framerate = 12
        self.__dict__.update(argd)
        self.images = []
        self.writehead = len(self.images) # index of the next image to be filled up. Between 0 and n.

class SplitScreenTheme(object):
    """
    Theme allow to customize the graphical attributes of the rendering.
    This is the base theme with a split screen.
    """
    def __init__(self):
        self.name = THEME_SPLIT_SCREEN
        self.render_play_first = True
        self.play_pos = (2.0, 0.0, 0.0)
        self.play_scale = (2.0, 1.5, 1.0)
        self.edit_pos = (-2.0, 0.0, 0.0)
        self.edit_scale = (2.0, 1.5, 1.0)
        self.background_color = (0.0, 0.0, 0.0)
        # saving progress bar
        self.progress_foreground_color = (1.0, 1.0, 1.0, 0.5) 
        self.progress_background_color = (0.7, 0.7, 0.7, 0.5) 
        self.progress_line_color = (1.0, 1.0, 1.0, 0.6)
        self.progress_pos = (0.0, -2.0, 0.0) 
        self.progress_scale = (3.0, 0.05, 1.0) 
        self.flash_color = (1.0, 1.0, 1.0, 0.5)
        self.white_flash_enabled = True
        #TODO: add a theme not displaying the edit "viewport".

class PictureInPictureTheme(SplitScreenTheme):
    """
    This theme is a picture in picture theme.
    """
    def __init__(self):
        SplitScreenTheme.__init__(self) # inherit some attributes from the base theme.
        self.render_play_first = False
        self.name = THEME_PICTURE_IN_PICTURE
        self.play_pos = (0.0, 0.0, 0.0)
        self.play_scale = (4.0, 3.0, 1.0)
        self.edit_pos = (2.0, 1.5, 0.0)
        self.edit_scale = (1.0, 0.75, 1.0)

class PlaybackPictureInPictureTheme(SplitScreenTheme):
    """
    This theme is a picture in picture theme as well, but the edit is big.
    """
    def __init__(self):
        SplitScreenTheme.__init__(self) # inherit some attributes from the base theme.
        self.render_play_first = True
        self.name = THEME_PLAYBACK_PICTURE_IN_PICTURE
        self.edit_pos = (0.0, 0.0, 0.0)
        self.edit_scale = (4.0, 3.0, 1.0)
        self.play_pos = (2.0, 1.5, 0.0)
        self.play_scale = (1.0, 0.75, 1.0)
    
class Toonloop(render.Game):
    """
    Toonloop is a realtime stop motion tool.

    For 1 GB of RAM, one can expect to stock about 3000 images at 640x480, if having much swap memory.

    Private methods starts with _.
    The draw method is a special one inherited from rats.render.Game. The process_events one as well.

    Onion skin effect and chroma key shader are mutually exclusives.

    There are a lot of services/features that are disabled by default. (fudi, osc, midi, etc.)
    """
    # OpenGL textures indices in the list
    TEXTURE_MOST_RECENT = 0
    TEXTURE_PLAYBACK = 1
    TEXTURE_ONION = 2
    TEXTURE_BACKGROUND = 3

    def __init__(self, config):
        """
        Startup poutine.

        Reads config.
        Starts pygame, the camera, the clips list.
        Creates the window. Hides the mouse.
        Start services (OSC and FUDI)
        Enables the effects or the onion peal.
        """
        self.config = config
        # size of the rendering window
        self._display_size = (self.config.display_width, self.config.display_height)
        self.running = True
        self.pd = None # fudi send and receive
        self.midi_manager = None
        self.paused = False
        self.image_size = (self.config.image_width, self.config.image_height)
        self.clock = pygame.time.Clock()
        self.fps = 0 # for statistics
        self.clip_id = 1 # Currently selected clip
        self.clip = None # current ToonClip instance
        self.clips = [] # ToonClip instances
        self._saver_progress = None # ClipSaver progress bar ratio. float from 0 to 1
        self._init_clips()
        self.renderer = None # Renderer instance that owns it.
        self.themes = {
            THEME_SPLIT_SCREEN: SplitScreenTheme(),
            THEME_PICTURE_IN_PICTURE: PictureInPictureTheme(),
            THEME_PLAYBACK_PICTURE_IN_PICTURE: PlaybackPictureInPictureTheme(),
            }
        if self.config.display_theme not in self.themes.keys():
            print("Error: not such theme: %s. using default")
            self.config.display_theme = THEME_SPLIT_SCREEN
        self.theme = self.themes[self.config.display_theme]
            
        # the icon
        try:
            icon = pygame.image.load(os.path.join(PACKAGE_DATA_PATH, "icon.png"))
            pygame.display.set_icon(icon) # a 32 x 32 surface
        except pygame.error, e:
            print("ERROR : Could not load icon : %s" % (e.message))
        # the pygame window
        window_flags = PYGM.OPENGL | PYGM.DOUBLEBUF | PYGM.HWSURFACE
        if self.config.display_fullscreen:
            window_flags |= PYGM.FULLSCREEN
            self._display_size = (0, 0) # Automatically detected !
            self.display = pygame.display.set_mode(self._display_size, window_flags)
        else:
            window_flags |= PYGM.RESIZABLE
            self.display = pygame.display.set_mode(self._display_size, window_flags)
        pygame.display.set_caption("Toonloop")
        pygame.mouse.set_visible(False)
        # the images
        
        self.osc = None # sender and receiver.
        self.camera = None # pygame camera
        self.is_mac = False # is on Mac OS X or not. (linux) For the camera.
        self.textures = [0, 0, 0, 0] # list of OpenGL texture objects id
        self._setup_window()
        # updates the size of the image according to the actual image size grabbed from the camera.
        self._setup_camera()
        self.most_recent_image = pygame.surface.Surface(self.image_size)
        self._camera_grab_frame()
        _w, _h = self.most_recent_image.get_size()
        print _w, _h
        if _w != self.config.image_width or _h != self.config.image_height:
            print("Expected image dimensions are %sx%s, but we got %sx%s." % (self.config.image_width, self.config.image_height, _w, _h))
            self.config.image_width = _w
            self.config.image_height = _h
        self._blank_surface = pygame.Surface((self.config.image_width, self.config.image_height))
        self.effects = {}
        self.optgroups = {}
        self._setup_effects()
        self.background_image = None
        self._setup_background()
        self._clear_playback_view()
        self._clear_onion_peal()
        self._playhead_iterator = 0
        # intervalometer
        self._intervalometer_delayed_id = None
        self.intervalometer_on = self.config.intervalometer_on
        if config.intervalometer_on:
            self.intervalometer_toggle(True)
        # autosave
        self._autosave_delayed_id = None
        if config.autosave_on:
            self.autosave_toggle(True)
        # copy conf elements
        if self.config.verbose:
            pprint.pprint(self.config.__dict__)
        self._has_just_added_frame = False
        self.clip_saver = None
        self.sampler = None # toon.sampler.Sampler
        # signal / slots. Each is documented with its arg, or no arg.
        self.signal_playhead = sig.Signal() # int index
        self.signal_writehead = sig.Signal() # int index
        self.signal_framerate = sig.Signal() # int fps
        self.signal_clip = sig.Signal() # int clip id
        self.signal_frame_add = sig.Signal() # no arg
        self.signal_frame_remove = sig.Signal() # no arg
        self.signal_sampler_record = sig.Signal() # bool start/stop
        self.signal_sampler_clear = sig.Signal() # bool start/stop
        
        # start services
        reactor.callLater(0, self._start_services)

    def _setup_effects(self):
        """
        Set all effects up.
        """
        #TODO: use glew or so to check if needed OpenGL extensions are there.
        if self.config.effects_enabled:
            self.effects = fx.load_effects()
            # current effect name is "None"
            for effect in self.effects.itervalues():
                self.optgroups[effect.name] = effect.options
        else:
            self.effects = {}

    def effect_next(self):
        """
        Selects the next effect.
        """
        previous = self.config.effect_name 
        all = self.effects.keys()
        if len(all) == 0:
            print("No effect loaded.")
        else:
            index_of_previous = all.index(previous)
            if index_of_previous >= len(all) - 1:
                new = all[0]
            else:
                new = all[index_of_previous + 1]
            if self.config.verbose:
                print("Using effect %s" % (new))
            self.config.effect_name = new

    def _get_current_effect(self):
        return self.effects[self.config.effect_name]

    def _start_services(self):
        """
        Starts the Toonloop network services.

        Called once the Twisted reactor has been started. 

        Implemented services : 
         * FUDI protocol with PureData
         * MIDI input (not quite a service, but we start it here)
        """
        # OSC
        if self.config.osc_enabled and OSC_LOADED:
            self.osc = opensoundcontrol.ToonOsc(
                self, 
                listen_port=self.config.osc_listen_port, 
                send_port=self.config.osc_send_port, 
                send_host=self.config.osc_send_host, 
                )
            if self.config.osc_sampler_enabled:
                self.sampler = sampler.Sampler(self, self.osc)

        # FUDI
        if self.config.fudi_enabled:
            try:
                # TODO: subscription push mecanism
                app = self
                fudi_recv = self.config.fudi_receive_port
                fudi_send = self.config.fudi_send_port
                fudi_send_host = self.config.fudi_send_host

                self.pd = puredata.start(app=app, receive_port=fudi_recv, send_port=fudi_send, send_host=fudi_send_host)
            except:
                print("Error loading puredata: %s" % (sys.exc_info()))
                #raise
        # MIDI
        if self.config.midi_enabled:
            self.midi_manager = midi.SimpleMidiInput(self.config.midi_input_id, self.config.midi_verbose)
            self.midi_manager.register_callback(self._cb_midi_event)
            try:
                self.midi_manager.start()
            except midi.NotConnectedError, e:
                print("Could not setup MIDI device %d" % (self.config.midi_input_id))
        self.signal_clip(0) # default clip id
        self.signal_writehead(0)

    def theme_change(self):
        """
        Pick next theme
        """
        all = self.themes.keys()
        current_index = all.index(self.theme.name)
        current_index = (current_index + 1) % len(all)
        self.theme = self.themes[all[current_index]]
        if self.config.verbose:
            print("Switching to the %s theme." % (self.theme.name))
        self.config.display_theme = self.theme.name

    def sampler_record(self, start=True):
        """
        Starts of stops recording a sampler.
        The sounds sampler handles this.
        """
        self.signal_sampler_record(start)
    
    def sampler_clear(self):
        """
        Clear the sound in current frame
        The sounds sampler handles this.
        """
        self.signal_sampler_clear()
        
    def _cb_midi_event(self, event):
        """
        Called when a MIDI event happens.
        If MIDI is enabled, of course.
        """
        MIDI_NOTE = 144
        MIDI_CTRL = 176
        if event.status == MIDI_NOTE: # MIDI note
            note = event.data1
            velocity = event.data2
            on = event.data2 >= 1 # bool
            if self.config.midi_verbose:
                print("MIDI note: Pitch: %s Velocity: %s" % (note, velocity))
            if note == self.config.midi_note_record:
                if on:
                    print("start recording sample")
                    self.sampler_record(True)
                else:
                    print("stop recording sample")
                    self.sampler_record(False)
            if note == self.config.midi_note_clear:
                if on:
                    print("clear a sample")
                    self.sampler_clear()
        elif event.status == MIDI_CTRL: # MIDI control
            ctrl_id = event.data1
            val = event.data2
            if self.config.midi_verbose:
                print("MIDI control: ID: %s Value: %s" % (ctrl_id, val))
            if ctrl_id == self.config.midi_pedal_control_id and val >= 1:
                self.frame_add()

    def _setup_background(self):
        """
        Loads initial background image.
        """
        draw.texture_from_image(self.textures[self.TEXTURE_BACKGROUND], self._blank_surface)

    def bgimage_snap(self):
        """
        Makes the current image from the camera to be the background image.
        """
        if self.config.verbose:
            print("Taking a snapshot for background image.")
        try:
            if self.is_mac:
                self.background_image = self.most_recent_image.copy()
            else:
                self.background_image = self.most_recent_image
        except MemoryError, e:
            print("CRITICAL ERROR : No more memory !!! %s" % (e.message))
        else:
            self.config.bgimage_enabled = True
            # Create an OpenGL texture
            draw.texture_from_image(self.textures[self.TEXTURE_BACKGROUND], self.background_image)

    def print_stats(self):
        """
        Print statistics
        """
        print(">>>>>>> Toonloop Statistics >>>>>>>>")
        try:
            self.config.print_values()
            print('pygame.display.Info(): %s' % (pygame.display.Info()))
            total_imgs = 0
            for clip_num in range(len(self.clips)):
                num_images = len(self.clips[clip_num].images)
                print(' * Clip #%d has %d images.' % (clip_num, num_images))
                total_imgs += num_images
            print('TOTAL: %d images.' % (total_imgs))
            print("Current playhead: " + str(self.clip.playhead))
            print("Num images: " + str(len(self.clip.images)))
            print("FPS: %d" % (self.fps))
            print("Playhead frequency ratio: 30 / %d" % (self.clip.playhead_iterate_every))
            self.print_optgroups()
            print("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<")
        except AttributeError, e:
            print sys.exc_info()
    
    def set_option_in_group(self, group, key, value):
        """
        Set an option that is in a OptionsGroup.
        See toon.optgroup.OptionsGroup.

        Might raise ToonloopError of OptionGroupError
        :param group: Name of the group.
        :param key: Name of the option.
        :param value: str with the value.
        """
        if group not in self.optgroups.keys():
            raise ToonloopError("No option group named %s." % (group))
        else:
            obj = self.optgroups[group]
            obj.set_value(key, value)
            if self.config.verbose:
                print("Set %s in group %s to %s" % (key, group, value))

    def print_optgroups(self):
        for name, group in self.optgroups.iteritems():
            print("Options in group %s:" % (name))
            for key, value in group.__dict__.iteritems():
                print("    -x %s %s %s" % (name, key, value))

    def print_help(self):
        """
        Prints help for live keyboard controls.
        """
        print(INTERACTIVE_HELP)

    def hud_toggle(self):
        """
        Toggles on/off the head-up display of statistics.
        """
        self.config.hud_enabled = not self.config.hud_enabled

    def _draw_hud(self):
        """
        Draws the head-up-display. Numbers, text, etc. overlayed on top of images. 
        """
        #TODO
        if self.config.hud_enabled:
            # style
            hud_base_position = (-3.8, 2.8, 0.0)
            hud_text_color = (1.0, 1.0, 1.0, 1.0)
            _font_height = 16
            # got the value 16 pixels of height with:
            #GLUT.glutBitmapHeight(GLUT.GLUT_BITMAP_HELVETICA_12) 
            hud_line_height = 6.0 / (float(self.config.display_height) / float(_font_height))
            hud_line_height *= -1
            # holds state between each text drawing
            _current_pos = [
                    hud_base_position[0], 
                    hud_base_position[1], 
                    hud_base_position[2] 
                ]
            def _write(txt, _current_pos):
                draw.draw_text(txt, position=_current_pos, color=hud_text_color)
                _current_pos[1] += hud_line_height

            _write("Press \"i\" to make this text disappear.", _current_pos)
            _write("Toonloop statistics and help.", _current_pos)
            # draw number of images in each clip
            total_imgs = 0
            txt_num = "Number of images: "
            num_clips = len(self.clips)
            for clip_num in range(num_clips):
                num_images = len(self.clips[clip_num].images)
                txt_num += "%3d" % (num_images)
                if clip_num != num_clips - 1:
                    txt_num += ", "
                total_imgs += num_images
            # and total number of images
            txt_num += ". Total: %d" % (total_imgs)
            _write(txt_num, _current_pos)
            # draw playhead position
            _write("Current playhead: " + str(self.clip.playhead), _current_pos)
            _write("Num images: " + str(len(self.clip.images)), _current_pos)
            _write("Rendering FPS: %d" % (self.fps), _current_pos)

            actual_playhead_fps = self.fps / float(self.clip.playhead_iterate_every)
            _write("Playhead frequency ratio: 30 / %d. Actual playhead FPS: %2.2f" % (self.clip.playhead_iterate_every, actual_playhead_fps), _current_pos)
            
            if self.config.autosave_enabled:
                _write("Auto saving every %s seconds." % (self.config.autosave_rate_seconds), _current_pos)
            _write("Project name: %s" % (self.config.project_name), _current_pos)
            _write("Video device: %s" % (self.config.video_device), _current_pos)
            _write("Toonloop directory: %s" % (self.config.toonloop_home), _current_pos)
            _write("Captured image dimensions: %sx%s" % (self.config.image_width, self.config.image_height), _current_pos)
            _write("Window dimensions: %sx%s" % (self.config.display_width, self.config.display_height), _current_pos)
            _write("Theme: %s" % (self.config.display_theme), _current_pos)
            if self.config.effects_enabled:
                _write("Effect: %s" % (self.config.effect_name), _current_pos)
            if self.config.fudi_enabled:
                _write("FUDI sending on %s:%s, receiving on %s." % (self.config.fudi_send_host, self.config.fudi_send_port, self.config.fudi_receive_port), _current_pos)
            if self.config.image_flip_horizontal:
                _write("The image is flipped horizontally", _current_pos)
            if self.config.intervalometer_enabled and self.config.intervalometer_on:
                _write("Intervalometer is on. Grabbing an image every %s seconds." % (self.config.intervalometer_rate_seconds), _current_pos)
            if self.config.midi_enabled:
                _write("Using MIDI input %s" % (self.config.midi_input_id), _current_pos)
            if self.config.onionskin_enabled:
                _write("Onion skinning is on.", _current_pos)
            if self.config.osc_enabled:
                _write("OSC sending on %s:%s, receiving on %s." % (self.config.osc_send_host, self.config.osc_send_port, self.config.osc_listen_port), _current_pos)

            _write("Configuration file: %s. (press x to save)" % (self.config.config_file), _current_pos)
            _current_pos[1] += hud_line_height # spacer
            _write("~~~ HELP ~~~", _current_pos)
            for line in INTERACTIVE_HELP.splitlines():
                if _current_pos > -4.0:
                    _write(line, _current_pos)

    def _init_clips(self):
        """
        Creates a new project.
        """
        for i in range(self.config.max_num_clips):
            self.clips.append(ToonClip(i))
        try:
            self.clip = self.clips[self.clip_id]
        except IndexError:
            if self.config.max_num_clips > 1:
                self.clip_id = 1 # default clip is 1, for a better usability
            else:
                self.clip_id = 0
            self.clip = self.clips[self.clip_id]

    def clip_select(self, index=0):
        """
        Selects an other clip
        """
        self.clip_id = index
        self.clip = self.clips[index]
        if self.config.verbose:
            print("Clip #%s" % (self.clip_id))
        if len(self.clip.images) == 0:
            self._clear_playback_view()
        self.signal_clip(self.clip_id)
        
    def _setup_window(self):
        """
        OpenGL setup.
        """
        # create OpenGL texture objects 
        # window is 1280 x 960
        self._resize_window()#self._display_size) # arg totally useless
        GL.glEnable(GL.GL_TEXTURE_RECTANGLE_ARB) # 2D)
        GL.glEnable(GL.GL_BLEND)
        GL.glShadeModel(GL.GL_SMOOTH)
        GL.glClearColor(0.0, 0.0, 0.0, 0.0) # black background
        GL.glColor4f(1.0, 1.0, 1.0, 1.0) # self.config.playback_opacity) # for now we use it for all
        GL.glBlendFunc(GL.GL_SRC_ALPHA, GL.GL_ONE_MINUS_SRC_ALPHA)
        for i in range(len(self.textures)):
            self.textures[i] = GL.glGenTextures(1)
        # print "texture names : ", self.textures


    def _resize_window(self): 
        #, (width, height)):
        """
        Called when we resize the window.
        (fullscreen on/off)

        The OpenGL coordinates go from -4 to 4 horizontally
        and -3 to 3 vertically.
        (ratio is 4:3)
        """
        #print("resize", width, height)
        #if height == 0:
        #    height = 1
        GL.glMatrixMode(GL.GL_PROJECTION)
        GL.glLoadIdentity()
        GL.glOrtho(-4.0, 4.0, -3.0, 3.0, -1.0, 1.0)
        GL.glMatrixMode(GL.GL_MODELVIEW)
        GL.glLoadIdentity()

    def _setup_camera(self):
        """
        Starts using the video camera.
        """
        if os.uname()[0] == 'Darwin':
            self.is_mac = True
        else:
            self.is_mac = False
        size = (self.config.image_width, self.config.image_height)
        try:
            pygame.camera.init() 
        # except AttributeError, e:
        #     print "Sometimes the camera module need it's init()",
        #     print " to be called and sometimes not."
        except AttributeError, e:
            print('ERROR: pygame.camera has no method init() ! %s' % (e.message))
        except Exception, e:
            print("error calling pygame.camera.init(): %s" % (e.message))
            print(sys.exc_info())
            raise ToonloopError("Error initializing the video camera. %s" % (e.message))
        try:
            all_cameras = pygame.camera.list_cameras()
            print("cameras : %s" % (all_cameras))
            if self.is_mac:
                print("Using camera %s" % (self.config.video_device))
                self.camera = pygame.camera.Camera(str(self.config.video_device), size)
            else:
                camera_file_name = "/dev/video%d" % (self.config.video_device)
                if camera_file_name not in all_cameras:
                    print("ERROR : Camera %s does not exist !" % (camera_file_name))
                    if len(all_cameras) == 0:
                        raise ToonloopError("Could not find a valid V4L2 video camera.")
                    else:
                        # using the first camera in list.
                        camera_file_name = all_cameras[0]
                        self.config.video_device = int(camera_file_name[-1]) # last char of str
                print("Using camera %s" % (camera_file_name))
                self.camera = pygame.camera.Camera(camera_file_name, size)
            self.camera.start()
        except SystemError, e:
            print(sys.exc_info())
            raise ToonloopError("Invalid camera. %s" % (str(e.message)))
        except Exception, e:
            print(sys.exc_info())
            raise ToonloopError("Invalid camera. %s" % (str(e.message)))
    
    def frame_add(self):
        """
        Copies the last grabbed frame to the list of images.
        """
        index = self.clip.writehead
        try:
            if self.is_mac:
                #self.clip.images.append(self.most_recent_image.copy())
                self.clip.images.insert(index, self.most_recent_image.copy())
            else:
                #self.clip.images.append(self.most_recent_image)
                self.clip.images.insert(index, self.most_recent_image)
        except MemoryError, e:
            print("CRITICAL ERROR : No more memory !!! %s" % (e.message))
        else:
            self.clip.writehead += 1
            # Creates an OpenGL texture
            draw.texture_from_image(self.textures[self.TEXTURE_ONION], self.most_recent_image)
            self._has_just_added_frame = True
            if self.config.verbose:
                pass
                #print("Added frame at index %d" % (index))
                #print('num frames: %s' % (len(self.clip.images)))
            self.signal_writehead(len(self.clip.images))
            self.signal_frame_add()
    
    def writehead_move(self, steps):
        """
        Moves the writehead of the current clip.
        :param steps: How many frames in which direction. 
        To move to the left, give a steps value of 1. 
        To move to the right, give a steps value of -1. 
        """
        index = self.clip.writehead + steps
        if index == -1:
            if self.config.verbose:
                print("Already at the first frame. Type RETURN to go to last.")
        else:
            self.writehead_goto(index)
    
    def writehead_goto(self, index):
        """
        Moves the writehead of the current clip to the given index.
        :param index: int
        -1 for the end.
        0 for the beginning
        """
        last = len(self.clip.images)
        if index == -1:
            index = last
        elif index < 0: # if negative
            index = last - index
            if index < -last: # if more negative than it can be...
                index = 0
        elif index > last:
            print("writehead_goto: Frame index %d is too big. Using %d instead." % (index, last))
            index = last
        if self.config.verbose:
            print("writehead_goto %d" % (index))
        if len(self.clip.images) == 0:
            index = 0
        else:
            # copying the onion skinning texture
            try:
                draw.texture_from_image(self.textures[self.TEXTURE_ONION], self.clip.images[index])
            except IndexError, e:
                index = len(self.clip.images) - 1
                draw.texture_from_image(self.textures[self.TEXTURE_ONION], self.clip.images[index])
        self.clip.writehead = index
        self.signal_writehead(index) # FIXME : is this ok to call it ?
    
    def draw(self):
        """
        Renders one frame.
        Called from the event loop. (twisted)
        """
        GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)
        if not self.paused:
            self._playhead_iterator = (self._playhead_iterator + 1) % self.clip.playhead_iterate_every
            if self._playhead_iterator ==  0:
                self._playhead_iterate()
                if len(self.clip.images) > 0:
                    # send osc message /toon/playhead
                    self.signal_playhead(self.clip.playhead)
                # 30/3 = 10 FPS
        self._camera_grab_frame() # grab a frame
        self._draw_window_background()
        # now, let's draw something
        GL.glEnable(GL.GL_TEXTURE_RECTANGLE_ARB)
        
        if self.theme.render_play_first:
            # edit
            self._draw_edit_background()
            self._draw_edit_view()
            self._draw_onion_skin()
            self._draw_white_flash()
            # playback
            self._draw_play_background()
            self._draw_playback_view()
        else:
            # playback
            self._draw_play_background()
            self._draw_playback_view()
            self._draw_edit_background()
            self._draw_edit_view()
            # edit
            self._draw_onion_skin()
            self._draw_white_flash()
        GL.glDisable(GL.GL_TEXTURE_RECTANGLE_ARB) # important not to draw a big pixel !
        # ----------- saving progress bar
        self._draw_saving_progress_bar()
        self._draw_hud()
        # ----------- done drawing.
        self.clock.tick()
        self.fps = self.clock.get_fps()
        pygame.display.flip()
        # old : pygame.display.update()
        gc.collect() # force garbage collection on every frame
        # TODO: on every frame is a bit much. Maybe every second ?
        # otherwise, python slows down when it is time to collect garbage.

    def _draw_saving_progress_bar(self):
        """
        Draws the progress bar of saving of the clip, if in progress.
        """
        if self._saver_progress is not None:
            GL.glPushMatrix()
            GL.glTranslatef(*self.theme.progress_pos)
            GL.glScalef(*self.theme.progress_scale)
            draw.draw_horizontal_progress_bar(
                background_color=self.theme.progress_background_color, 
                foreground_color=self.theme.progress_foreground_color, 
                line_color=self.theme.progress_line_color, 
                progress=self._saver_progress)
            GL.glPopMatrix()

    def _draw_white_flash(self):
        if self._has_just_added_frame:
            self._has_just_added_frame = False
            if self.theme.white_flash_enabled:
                # TODO: use time.time() to create tween.
                # left view
                GL.glColor4f(*self.theme.flash_color)
                GL.glPushMatrix()
                GL.glTranslatef(*self.theme.edit_pos)
                GL.glScalef(*self.theme.edit_scale)
                draw.draw_square()
                GL.glPopMatrix()
    def _draw_window_background(self):
        GL.glColor3f(*self.theme.background_color)
        GL.glPushMatrix()
        GL.glScalef(4.0, 3, 1.0)
        draw.draw_square()
        GL.glPopMatrix()
        GL.glColor4f(1.0, 1.0, 1.0, 1.0)

    def _draw_edit_background(self):
        """
        Renders the background 
        """
        if self.config.bgimage_enabled:
            GL.glColor4f(1.0, 1.0, 1.0, 1.0)
            # playback view
            GL.glPushMatrix()
            GL.glTranslatef(*self.theme.play_pos)
            GL.glScalef(*self.theme.play_scale)
            GL.glBindTexture(GL.GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_BACKGROUND])
            if self.config.image_flip_horizontal: # FIXME?
                GL.glRotatef(180., 0., 1., 0.)
            draw.draw_textured_square(self.config.image_width, self.config.image_height)
            GL.glPopMatrix()

    def _draw_play_background(self):
        if self.config.bgimage_enabled:
            GL.glColor4f(1.0, 1.0, 1.0, 1.0)
            # edit view
            GL.glPushMatrix()
            GL.glTranslatef(*self.theme.edit_pos)
            GL.glScalef(*self.theme.edit_scale)
            GL.glBindTexture(GL.GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_BACKGROUND])
            if self.config.image_flip_horizontal: # FIXME?
                GL.glRotatef(180., 0., 1., 0.)
            draw.draw_textured_square(self.config.image_width, self.config.image_height)
            GL.glPopMatrix()

    def _camera_grab_frame(self):
        """
        Image capture from the video camera.
        """
        if self.is_mac:
            self.camera.get_image(self.most_recent_image)
        else:
            self.most_recent_image = self.camera.get_image()
        # Create an OpenGL texture
        draw.texture_from_image(self.textures[self.TEXTURE_MOST_RECENT], self.most_recent_image)
        
    def _draw_edit_view(self):
        """
        Renders edit view (the live camera + onion peal)
        """
        if self.config.effects_enabled:
            self._get_current_effect().pre_draw()
        GL.glColor4f(1.0, 1.0, 1.0, 1.0)
        GL.glPushMatrix()
        GL.glTranslatef(*self.theme.edit_pos) 
        GL.glScalef(*self.theme.edit_scale) 
        # most recent grabbed :
        GL.glBindTexture(GL.GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_MOST_RECENT])
        if self.config.image_flip_horizontal: # FIXME?
            GL.glRotatef(180., 0., 1., 0.)
        draw.draw_textured_square(self.config.image_width, self.config.image_height)
        # self.display_width = 1024
        GL.glPopMatrix()
        # old: self.display.blit(self.most_recent_image, (0, 0))
        if self.config.effects_enabled:
            self._get_current_effect().post_draw()

    def _draw_onion_skin(self):
        # FIXME: onion skin does not work with every effect. 
        # glsl effects should take alpha into account.
        if self.config.onionskin_enabled and self.config.onionskin_on:
            if len(self.clip.images) > 0:
                # Onion skin over dit view:
                
                texture_id = self.textures[self.TEXTURE_ONION]
                # FIXME: the shader actually want the texture unit !
                #effect.options.texture_id = texture_id
                if self.config.effects_enabled:
                    effect = self._get_current_effect()
                    effect.pre_draw()
                GL.glPushMatrix()
                GL.glTranslatef(*self.theme.edit_pos)
                GL.glScalef(*self.theme.edit_scale)
                GL.glColor4f(1.0, 1.0, 1.0, self.config.onionskin_opacity)
                GL.glBindTexture(GL.GL_TEXTURE_RECTANGLE_ARB, texture_id)
                #if self.config.image_flip_horizontal: # FIXME?
                #    GL.glRotatef(180., 0., 1., 0.)
                draw.draw_textured_square(self.config.image_width, self.config.image_height)
                GL.glColor4f(1.0, 1.0, 1.0, 1.0) 
                GL.glPopMatrix()
                if self.config.effects_enabled:
                    effect.post_draw()

    def _draw_playback_view(self):
        """
        Renders the playback view. 
        """
        if self.config.effects_enabled:
            effect = self._get_current_effect()
            effect.pre_draw()
        GL.glColor4f(1.0, 1.0, 1.0, 1.0)
        GL.glPushMatrix()
        GL.glTranslatef(*self.theme.play_pos)#2.0, 0.0, 0.0)
        GL.glScalef(*self.theme.play_scale)#2.0, 1.5, 1.0)
        GL.glBindTexture(GL.GL_TEXTURE_RECTANGLE_ARB, self.textures[self.TEXTURE_PLAYBACK])
        if self.config.image_flip_horizontal: # FIXME?
            GL.glRotatef(180., 0., 1., 0.)
        draw.draw_textured_square(self.config.image_width, self.config.image_height)
        GL.glPopMatrix()
        if self.config.effects_enabled:
            effect.post_draw()
    
    def _playhead_iterate(self):
        """ 
        Increments the playhead position of one frame
        """
        if len(self.clip.images) == 0:
            self.clip.playhead_previous = 0
            # nothing to copy to VRAM
        else:
            # check things up
            if self.clip.direction == DIRECTION_FORWARD:
                direction = DIRECTION_FORWARD
            elif self.clip.direction == DIRECTION_BACKWARD:
                direction = DIRECTION_BACKWARD
            elif self.clip.direction == DIRECTION_YOYO:
                # FIXME: this should be simpler. Maybe use a table lookup?
                if self.clip.playhead > self.clip.playhead_previous:
                    # if was going forward
                    if self.clip.playhead >= len(self.clip.images) - 1:
                        direction = DIRECTION_BACKWARD
                    else:
                        direction = DIRECTION_FORWARD
                else:
                    # if was going backwards
                    if self.clip.playhead == 0:
                        direction = DIRECTION_FORWARD
                    else:
                        direction = DIRECTION_BACKWARD
            self.clip.playhead_previous = self.clip.playhead
            # actually do the incrementation
            if direction == DIRECTION_FORWARD:
                if self.clip.playhead < len(self.clip.images) - 1:
                    self.clip.playhead += 1
                else:
                    self.clip.playhead = 0
            elif direction == DIRECTION_BACKWARD:
                if self.clip.playhead > 0:
                    self.clip.playhead -= 1
                else:
                    self.clip.playhead = len(self.clip.images) - 1
            # now, let's copy it to VRAM
            try:
                image = self.clip.images[self.clip.playhead]
            except IndexError, e:
                print("ERROR: No frame %s in clip %s." % (self.clip.playhead, self.clip))
                image = self.clip.images[0]
                self.clip.playhead = 0
            draw.texture_from_image(self.textures[self.TEXTURE_PLAYBACK], image)

    def _clear_playback_view(self):
        """
        Sets all pixels in the playback view as black.
        """
        #blank_surface = pygame.Surface((self.config.image_width, self.config.image_height))
        draw.texture_from_image(self.textures[self.TEXTURE_PLAYBACK], self._blank_surface)
        
    def _clear_onion_peal(self):
        """
        Sets all pixels in the onion peal as black.
        """
        #blank_surface = pygame.Surface((self.config.image_width, self.config.image_height))
        draw.texture_from_image(self.textures[self.TEXTURE_ONION], self._blank_surface)

    def pause(self, val=None):
        """
        Toggles on/off the pause
        """
        if val is not None:
            self.paused = val
        else:
            self.paused = not self.paused

    def clip_reset(self):
        """
        Deletes all frames from the current animation
        """
        self.clip.images = []
        self._clear_playback_view()

    def clip_save(self):
        """
        Saves all images as jpeg and encodes them to a MJPEG movie.
        
        See _write_next_image
        """
        if self.config.verbose:
            print("clip_save")
        # TODO : in a thread
        if len(self.clip.images) > 1:
            if self.clip_saver is not None:
                if self.clip_saver.is_busy:
                    print("There is already one clip being saved. Try again later.")
                    return # TODO: return failed deferred
                else:
                    self.clip_saver = None
            dir_path = os.path.join(self.config.toonloop_home, self.config.project_name)
            file_prefix = strftime("%Y-%m-%d_%Hh%Mm%S") # without an extension.
            file_prefix += '_%s' % (self.clip_id)
            if self.config.verbose:
                print("Will save images %s %s" % (dir_path, file_prefix))
            core = self
            self.clip_saver = save.ClipSaver(core, dir_path, file_prefix, self.clip_id)
            self.clip_saver.signal_progress.connect(self._slot_saver_progress)
            self.clip_saver.signal_done.connect(self._slot_saver_done)
            return self.clip_saver.save() # returns a deferred

    def _slot_saver_progress(self, progress_ratio):
        """
        :param progress_ratio: float from 0 to 1
        """
        self._saver_progress = progress_ratio

    def _slot_saver_done(self, success):
        """
        :param success: boolean
        """
        self._saver_progress = None
        
    def frame_remove(self):
        """
        Deletes the last frame from the current list of images.
        """
        if self.clip.images != []:
            index = self.clip.writehead - 1
            try:
                self.clip.images.pop(index)
            except IndexError, e:
                print("ERROR: Could not remove frame '%s' at writehead - 1 in clip %s." % (index, self.clip))
                index = len(self.clip.images) - 1
                self.clip.images.pop(index)
                self.clip.writehead = max(0, index - 1)
            if self.clip.writehead != 0:
                self.clip.writehead -= 1 # FIXME Is this ok?
            # would it be better to also delete it ? calling del
            if self.clip.images == []:
                self._clear_playback_view()
            else:
                pass 
            if self.clip.writehead > 0:
                try:
                    draw.texture_from_image(self.textures[self.TEXTURE_ONION], self.clip.images[self.clip.writehead - 1])
                except IndexError, e:
                    print("frame_remove:onion skin texture : %s" % (e.message))
            self.signal_writehead(self.clip.writehead)
            self.signal_frame_remove()
    
    def effect_select(self, name_or_index=None):
        """
        :param name_or_index: Either a int from 0 to n-1 or a string
        """
        if self.config.verbose:
            print("effect_select(%s)" % (name_or_index))
        all = self.effects.keys()
        curr = self.config.effect_name
        
        if name_or_index is None:
            try:
                noeffect = all.index("None")
            except ValueError, e:
                print(e.message)
            else:
                self.config.effect_name = "None"
        elif type(name_or_index) == int:
            try:
                name = all[name_or_index]
            except IndexError, e:
                print(e.message)
            else:
                self.config.effect_name = name
        else:
            try:
                _tmp = self.effects[name_or_index]
            except IndexError, e:
                print(e.message)
            else:
                self.config.effect_name = name_or_index

    def onionskin_toggle(self, val=None):
        """
        Toggles on/off the onion skin. 
        (see most recent frame grabbed in transparency)
        """
        if val is not None:
            self.config.onionskin_on = val is True # set to val
        else:
            self.config.onionskin_on = not self.config.onionskin_on # toggle
        print('config.onionskin_on = %s' % (self.config.onionskin_on))
            
    def framerate_increase(self, dir=1):
        """
        Increase or decreases the FPS
        :param dir: by how much increment it.
        """
        # TODO: for the user, it should be in FPS, not some weird ratio
        # does not alter the FPS of the renderer, but rather only the playback rate.
        # if self.renderer is not None:
        #     # accesses the Renderer instance that owns this.
        #     will_be = self.renderer.desired_fps + dir
        #     if will_be > 0 and will_be <= 60:
        #         self.renderer.desired_fps = will_be
        #         print "FPS:", will_be
        will_be = self.clip.playhead_iterate_every - dir
        if will_be > self.config.framerate_min and will_be <= self.config.framerate_max:
            self.clip.playhead_iterate_every = will_be
            if self.config.verbose:
                print("Playhead frequency ratio: 30 / %d" % (will_be))
        self.signal_framerate(will_be)

    def toggle_fullscreen(self):
        """
        Toggles from window to fullscreen view.
        """
        self.config.display_fullscreen != self.config.display_fullscreen
        pygame.display.toggle_fullscreen()
    
    def process_events(self, events):
        """
        Processes pygame events.
        :param events: got them using pygame.event.get()
        """
        for e in events:
            if e.type == PYGM.QUIT:
                self.running = False
            # TODO : catch window new size when resized.
            elif e.type == pygame.MOUSEBUTTONDOWN:
                if e.button == 1:
                    self.frame_add() # left mouse button
                elif e.button == 3: 
                    self.frame_remove() # right mouse button
            elif e.type == pygame.VIDEORESIZE:
                print("VIDEORESIZE %s" % (e))
            elif e.type == PYGM.KEYDOWN:
                modifiers = pygame.key.get_mods()
                if modifiers & PYGM.KMOD_LSHIFT != 0:
                    if self.config.verbose:
                        #print("Left shit is being pressed.")
                        pass
                try:
                    if self.config.verbose:
                        if e.key < 255 and not e.key == PYGM.K_ESCAPE:
                            c = chr(e.key)
                            #print("key down: %s (\"%s\")" % (e.key, c))
                    if e.key == PYGM.K_k: # K
                        self.intervalometer_rate_increase(1)
                    elif e.key == PYGM.K_j: # J
                        self.intervalometer_rate_increase(-1)
                    elif e.key == PYGM.K_f: # F Fullscreen
                        self.toggle_fullscreen()
                    elif e.key == PYGM.K_i: # I Info
                        if self.config.verbose: 
                            self.print_stats()
                        self.hud_toggle()
                    elif e.key == PYGM.K_p: # P Pause
                        self.pause()
                    elif e.key == PYGM.K_r: # R Reset
                        self.clip_reset()
                    elif e.key == PYGM.K_h or e.key == PYGM.K_F1: # H Help
                        self.print_help()
                    elif e.key == PYGM.K_s: # S Save
                        self.clip_save()
                    elif e.key == PYGM.K_x: # X : config save
                        self.config_save()
                    elif e.key == PYGM.K_n: # N : next effect
                        self.effect_next()
                    elif e.key == PYGM.K_o: # O Onion
                        self.onionskin_toggle()
                    elif e.key == PYGM.K_b: # B Background image snapshot
                        self.bgimage_snap()
                    elif e.key == PYGM.K_a: # A Auto
                        print("toggle intervalometer")
                        self.intervalometer_toggle()
                    elif e.key == PYGM.K_q: # q Start recording sample
                        if modifiers & PYGM.KMOD_CTRL != 0: # if control is being pressed
                            self.quit()
                        else:
                            self.sampler_record(True)
                    elif e.key == PYGM.K_w: # Clear the sound in current frame 
                        self.sampler_clear()
                    elif e.key == PYGM.K_0: # [0, 9] Clip selection
                        self.clip_select(0)
                    elif e.key == PYGM.K_1:
                        self.clip_select(1)
                    elif e.key == PYGM.K_2:
                        self.clip_select(2)
                    elif e.key == PYGM.K_3:
                        self.clip_select(3)
                    elif e.key == PYGM.K_4:
                        self.clip_select(4)
                    elif e.key == PYGM.K_5:
                        self.clip_select(5)
                    elif e.key == PYGM.K_6:
                        self.clip_select(6)
                    elif e.key == PYGM.K_7:
                        self.clip_select(7)
                    elif e.key == PYGM.K_8:
                        self.clip_select(8)
                    elif e.key == PYGM.K_9:
                        self.clip_select(9)
                    elif e.key == PYGM.K_UP: # UP Speed Increase
                        self.framerate_increase(1)
                    elif e.key == PYGM.K_DOWN: # DOWN Speed Decrease
                        self.framerate_increase(-1)
                    elif e.key == PYGM.K_RIGHT: # previous frame
                        self.writehead_move(1)
                    elif e.key == PYGM.K_LEFT: # next frame
                        self.writehead_move(-1)
                    elif e.key == PYGM.K_RETURN: # RETURN: goes to last frame
                        self.writehead_goto(-1)
                    elif e.key == PYGM.K_SPACE: # SPACE Add frame
                        self.frame_add()
                    elif e.key == PYGM.K_BACKSPACE: # BACKSPACE Remove frame
                        self.frame_remove()
                    elif e.key == PYGM.K_TAB: # TAB changes direction
                        self.direction_change()
                    elif e.key == PYGM.K_PERIOD: # PERIOD changes theme
                        self.theme_change()
                    elif e.key == PYGM.K_MINUS:
                        pass # TODO
                    elif e.key == PYGM.K_PLUS:
                        pass # TODO
                    elif e.key == PYGM.K_ESCAPE:
                        self.toggle_fullscreen()
                except ValueError, e :
                    if self.config.verbose:
                        print("Key event error : %s" % (e.message))

    def quit(self):
        """
        Quits the application in a short while.
        """
        reactor.callLater(0.1, self._quit)

    def _quit(self):
        self.running = False
        # self.cleanup will be called.

    def direction_change(self, direction=None):
        """
        Changes the direction of the current clip's playback.

        If direction param is None, will cycle through the available directions.
        :param direction: int with the value of the constant DIRECTION_FORWARD or DIRECTION_BACKWARD
        """
        if direction is None:
            if self.clip.direction == DIRECTION_BACKWARD:
                direction = DIRECTION_FORWARD
            elif self.clip.direction == DIRECTION_FORWARD:
                direction = DIRECTION_YOYO
            else:
                direction = DIRECTION_BACKWARD
        #if direction == DIRECTION_YOYO:
        #    print("Playhead direction YOYO is not yet implemented.") # TODO
        #else:
        if self.config.verbose:
            print("Changing playback direction to %s" % (direction))
        self.clip.direction = direction

    def autosave_toggle(self, val=None):
        """
        Toggles on/off the autosave 
        """
        if self.config.autosave_enabled:
            if val is not None:
                self.config.autosave_on = val
            else:
                self.config.autosave_on = not self.config.autosave_on
            if self.config.autosave_on:
                self._autosave_delayed_id = reactor.callLater(0, self._autosave)
                if self.config.verbose:
                    print("autosave ON")
            else:
                if self._autosave_delayed_id is not None:
                    if self._autosave_delayed_id.active():
                        self._autosave_delayed_id.cancel()
                        if self.config.verbose:
                            print("autosave OFF")
                
    def intervalometer_toggle(self, val=None):
        """
        Toggles on/off the intervalometer / timelapse / auto mode.
        """
        if self.config.intervalometer_enabled:
            if val is not None:
                self.intervalometer_on = val
            else:
                self.intervalometer_on = not self.intervalometer_on
            if self.intervalometer_on:
                self._intervalometer_delayed_id = reactor.callLater(0, self._intervalometer_frame_add)
                if self.config.verbose:
                    print("intervalometer ON")
            else:
                if self._intervalometer_delayed_id is not None:
                    if self._intervalometer_delayed_id.active():
                        self._intervalometer_delayed_id.cancel()
                        if self.config.verbose:
                            print("intervalometer OFF")
    
    def intervalometer_rate_increase(self, dir=1):
        """
        Increase or decreases the intervalometer rate. (in seconds)
        :param dir: by how much increment it.
        """
        if self.config.intervalometer_enabled:
            will_be = self.config.intervalometer_rate_seconds + dir
            if will_be > 0 and will_be <= 60:
                self.config.intervalometer_rate_seconds = will_be
                print("auto rate: %s" %  (will_be))
        if self.config.verbose:
            print("Cleaning up before exiting.")

    def _intervalometer_frame_add(self):
        """
        Called when it is time to automatically grab an image.

        The auto mode is like an intervalometer to create timelapse animations.
        """
        self.frame_add()
        if self.config.verbose:
            print("intervalometer auto grab %d" % (len(self.clip.images)))
            sys.stdout.flush()
        if self.intervalometer_on:
            self._intervalometer_delayed_id = reactor.callLater(self.config.intervalometer_rate_seconds, self._intervalometer_frame_add)

    def _autosave(self):
        """
        Called when it is time to automatically save a movie
        """
        # self.frame_add()
        if self.config.verbose:
            print("autosave")
        self.clip_save()
        if self.config.autosave_on:
            self._autosave_delayed_id = reactor.callLater(self.config.autosave_rate_seconds, self._autosave)

    def cleanup(self):
        """
        Called by the rats.render.Renderer before quitting the application.
        """
        if self.config.verbose:
            print("Cleaning up.")
        if self.config.osc_enabled:
            if self.config.verbose:
                print("Deleting OSC sender/receiver.")
            del self.osc
        # glDeleteTextures(3, self.textures)

    def config_set(self, name, value):
        """
        Changes a configuration option.
        """
        self.config.set(name, value)

    def config_save(self):
        """
        Saves the current config to a file.
        """
        if self.config.verbose:
            print("Save config to %s" % (self.config.config_file))
        self.config.save()
