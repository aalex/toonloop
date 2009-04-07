#!/bin/bash

v4l2-ctl --set-input=1
v4l2-ctl --set-standard=ntsc
v4l2-ctl --set-fmt-video=width=768,height=480

