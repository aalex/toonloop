#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# ToonLoop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ToonLoop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with ToonLoop.  If not, see <http://www.gnu.org/licenses/>.
#
"""
Simple example of a patch dynamically created using rats.purity.
"""

from rats.purity import client
from rats.purity import obj
from twisted.internet import reactor

class SamplerSubpatch(object):
    TABLE_PREFIX = "table_"
    def __init__(self, id, main_patch):
        self.id = id
        self.patch = main_patch.subpatch("sampler_" + str(self.id))
        self.r_rec = self.patch.receive("rec_" + str(self.id))
        self.r_play = self.patch.receive("play_" + str(self.id))
        table_name = self.TABLE_PREFIX + str(0)
        self.tabwrite = self.patch.obj("tabwrite~", table_name)
        self.tabread = self.patch.obj("tabread~", table_name)

class FxPatch(object):
    """
    FxLoop Pure Data patch.

    [adc~ 3]
    "set $1", "bang"
    [r rec]
    [tabwrite~]
    ------------ x 16 -----------
    [r play]
    "set $1", "bang"
    [tabread~]
    [line~]
    [dbtorms~]
    [*~]
    [throw~ out]
    ------------ x N ------------
    [table array-$N <samples>]
    -----------------------------
    [catch~ out]
    [*~ 0.125]
    [+~]
    [dac~]
    """
    NUM_TABLES = 16
    NUM_PLAYERS = 4
    SAMPLING_RATE = 48000
    ARRAY_DURATION = 10.0
    def __init__(self):
        self.main_patch = obj.get_main_patch()
        self.tables_subpatch = self.main_patch.subpatch("tables")
        self.samplers = {} # key is an index.
        for i in range(self.NUM_TABLES):
            self.tables_subpatch.obj("table", "table_" + str(i), int(self.SAMPLING_RATE * self.ARRAY_DURATION))
        for i in range(self.NUM_PLAYERS):
            self.samplers[i] = SamplerSubpatch(i, self.main_patch)
        # objects
        # r = 
        # tgl = patch.obj("tgl")
        # metro = patch.obj("metro", 500)
        # bang = patch.obj("bng")
        # # connections
        # patch.connect(r, 0, tgl, 0)
        # patch.connect(tgl, 0, metro, 0)
        # patch.connect(metro, 0, bang, 0)

    def creation_callback(self, client):
        """
        :param client: PurityClient instance.
        """
        # send messages
        mess_list = self.main_patch.get_fudi() # list of (fudi) lists
        # print(mess_list)
        for mess in mess_list:
            print("%s" % (mess))
            client.send_message(*mess)
        print "sent FUDI message:", "startme", 1
        client.send_message("startme", 1)

if __name__ == "__main__":
    fx = FxPatch()
    deferred = client.create_simple_client()
    deferred.addCallback(fx.creation_callback)
    reactor.run()

