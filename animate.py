#!/usr/bin/env python

import subprocess 
import sys
import glob

# mencoder "mf://*.jpg" -mf fps=12 -o output.mp4 -ovc lavc -lavcopts vcodec=mpeg4
if __name__ == '__main__':
    extension = "jpg"
    try:
        datetime_prefix = sys.argv[1]
    except IndexError:
        print "usage %s <jpeg_base_name>" % (sys.argv[0])
    else:
        command = ['mencoder']
        files = glob.glob("%s*.%s" % (datetime_prefix, extension))
        command.extend(files)
        args = ['-mf', 'fps=12', '-o', 'output.mov', '-ovc', 'lavc', '-lavcopts', 'vcodec=mpeg4']
        command.extend(args)
        print "command: ", command
        print subprocess.Popen(command, stdout=subprocess.PIPE).communicate()[0]

