#!/bin/bash
gst-launch hdv1394src ! queue ! decodebin ! queue ! xvimagesink
