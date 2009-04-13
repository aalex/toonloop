#!/bin/bash

v4l2-ctl --set-input=1
v4l2-ctl --set-standard=ntsc
v4l2-ctl --set-fmt-video=width=768,height=480
v4l2-ctl --set-ctrl=saturation=65535,contrast=32768
