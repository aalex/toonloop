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

def creation_callback(client):
    """
    :param client: PurityClient instance.
    """
    main_patch = obj.get_main_patch()
    # subpatch
    patch = main_patch.subpatch("metropatch", visible=True)
    # objects
    r = patch.receive("startme")
    tgl = patch.obj("tgl")
    metro = patch.obj("metro", 500)
    bang = patch.obj("bng")
    # connections
    patch.connect(r, 0, tgl, 0)
    patch.connect(tgl, 0, metro, 0)
    patch.connect(metro, 0, bang, 0)
    # send messages
    mess_list = main_patch.get_fudi() # list of (fudi) lists
    # print(mess_list)
    for mess in mess_list:
        print("%s" % (mess))
        client.send_message(*mess)
    print "sent FUDI message:", "startme", 1
    client.send_message("startme", 1)

if __name__ == "__main__":
    deferred = client.create_simple_client()
    deferred.addCallback(creation_callback)
    reactor.run()

