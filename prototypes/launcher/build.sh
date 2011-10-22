#!/bin/bash
gcc -Wall `pkg-config --libs --cflags glib-2.0` -o spawn spawn.c

