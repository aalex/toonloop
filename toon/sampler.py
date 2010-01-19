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
Controls the Toonloop audio sampler
"""
from rats import sig

# uses toon/opensoundcontrol, rats/sig and toon/core
class AllocationError(Exception):
    """
    Any error thrown by the Allocator or Mapper class.
    """
    pass

class Allocator(object):
    """
    Allocates sound indices.
    """
    def __init__(self, max_index=500):
        self.max_index = max_index
        self.pool = set()
    
    def allocate(self):
        ok = False
        for value in xrange(self.max_index):
            if value not in self.pool:
                ok = True
                break
        if not ok:
            raise AllocationError("No more numbers available. Pool: %s" % (self.pool))
        self.pool.add(value)
        return value

    def free(self, value):
        try:
            self.pool.remove(value)
        except KeyError, e:
            raise AllocationError("Value %d not in pool." % (value))

class Mapper(object):
    """
    Maps clip numbers, frame numbers to sound index.
    """
    def __init__(self, num_clips=10, max_num_frames=4000, num_sounds=500):
        self.num_sounds = num_sounds
        self.max_num_frames = max_num_frames
        self.num_clips = num_clips
        self.allocator = Allocator(self.num_sounds)
        self.clips = {} # dict of dicts. Indices are clip_id, frame_id
        #self.signal_clear = sig.Signal() # int buffer_id
        #self.signal_record = sig.Signal() # int bufer_id
        
    def add(self, clip_id, frame_id):
        """
        Adds a sounds in the given clip, frame slot.
        """
        if not self.clips.has_key(clip_id):
            if clip_id >= self.num_clips:
                raise AllocationError("Clip number too big: %s." % (clip_id))
            self.clips[clip_id] = {}
        if self.clips[clip_id].has_key(frame_id):
            buffer_id = self.clips[clip_id][frame_id]
            # self.signal_clear(buffer_id)
            # TODO: the actual sampler is responsible for clearer its buffer
            # if it has a sound in it when recording. 
            # (since UDP packet order's is not guaranteed)
        else:
            buffer_id = self.allocator.allocate()
            self.clips[clip_id][frame_id] = buffer_id
        #self.signal_record(buffer_id)
        return buffer_id

    def get(self, clip_id, frame_id):
        try:
            return self.clips[clip_id][frame_id]
        except KeyError:
            return None
    
    def get_data(self):
        # for serialize
        return self.clips
    
    def set_data(self, data):
        # for unserialize
        self.clips = data
    
    def remove(self, clip_id, frame_id):
        """
        Clears a sound from a given clip, frame slot.
        :return: buffer id
        """
        if self.clips.has_key(clip_id):
            if self.clips[clip_id].has_key(frame_id):
                buffer_id = self.clips[clip_id].pop(frame_id)
                self.allocator.free(buffer_id)
                #self.signal_clear(buffer_id)
                return buffer_id
            else:
                raise AllocationError("Clip %d has no frame %s." % (clip_id, frame_id))
        else:
            raise AllocationError("No clip %d." % (clip_id))

class Sampler(object):
    """
    == Sends OSC messages: ==
     * /sampler/play/start <i>
     * /sampler/play/stop <i>
     * /sampler/record/start <i>
     * /sampler/record/stop <i>
    """
    def __init__(self, toonloop, osc_manager):
        self.toonloop = toonloop
        NUM_SOUNDS = toonloop.config.osc_sampler_num_sounds # 500 ? 
        self.mapper = Mapper(num_sounds=NUM_SOUNDS)
        self.osc = osc_manager
        self.verbose = self.toonloop.config.osc_verbose
        self.player_id = 0
        self.NUM_PLAYERS = 24 # FIXME
        self._setup()

    def _setup(self):
        self.toonloop.signal_sampler_record.connect(self._slot_sampler_record)# bool start/stop
        self.toonloop.signal_sampler_clear.connect(self._slot_sampler_clear)
        self.toonloop.signal_playhead.connect(self._slot_playhead)

    def _slot_playhead(self, frame_id):
        clip_id = self.toonloop.clip.id
        buffer_id = self.mapper.get(clip_id, frame_id)
        if buffer_id is not None:
            player_id = (self.player_id + 1) % self.NUM_PLAYERS
            if self.verbose:
                print("send /sampler/play/start %d %s" % (player_id, buffer_id))
            self.osc.send_sampler_play_start(player_id, buffer_id)
        
    def _slot_sampler_record(self, starting): 
        """
        Slot which listens for a Toonloop's signal
        :param starting: bool start/stop
        """
        if starting:
            #TODO: send_record_start
            #TODO: send_record_stop
            clip_id = self.toonloop.clip.id
            frame_id = self.toonloop.clip.playhead
            try:
                buffer_id = self.mapper.add(clip_id, frame_id)
            except AllocationError, e:
                print(e.message)
            else:
                self.osc.send_sampler_record_start(buffer_id)
                if self.verbose:
                    print("send /sampler/record/start %d" % (buffer_id))
        else:
            if self.verbose:
                print("warning: record/stop not implemented.")
    
    def _slot_sampler_clear(self):
        """
        Slot which listens for a Toonloop's signal
        """
        # TODO
        clip_id = self.toonloop.clip.id
        frame_id = self.toonloop.clip.playhead
        try:
            buffer_id = self.mapper.remove(clip_id, frame_id)
        except AllocationError, e:
            print(e.message)
        else:
            self.osc.send_sampler_clear(buffer_id)
            if self.verbose:
                print("send /sampler/clear %d" % (buffer_id))
