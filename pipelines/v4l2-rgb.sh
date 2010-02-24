#!/bin/bash
# cannot negociate this format
gst-launch-0.10 v4l2src ! video/x-raw-rgb,width=640,height=480,framerate=30000/1001,interlaced=true ! ffmpegcolorspace ! ximagesink
