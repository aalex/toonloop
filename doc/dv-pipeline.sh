#!/bin/bash
gst-launch dv1394src ! queue ! decodebin ! queue ! xvimagesink
