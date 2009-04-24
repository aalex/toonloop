ToonLoop Live Documentation
===========================

.. image:: data/icon.png

:Author: Alexandre Quessy & Tristan Matthews
:Copyright: GNU General Public License.

You can browse the directory of `ToonLoop saved files`_.

The `ToonLoop Media RSS Feed`_ lists all saved movies. 

.. _`ToonLoop saved files`: /files
.. _`ToonLoop Media RSS Feed`: /rss

.. Note:: 

  ToonLoop is a stop motion tool. See http://www.toonloop.com for more informations.
  Using a simple video camera, users can take snapshots of drawings
  or objects and create an animation movies with those images. Creating 
  a movie is as simple as that in ToonLoop. 



.. contents::

=============
 User Manual
=============

Control Keys
------------

.. image:: data/keyboard_keys.png



Command-line arguments
----------------------

To list possible options::

  toonloop -l

To print help::

  toonloop -h

For verbose output::

  toonloop -v

The output of toonloop -h::

    Usage: toonloop1.0 beta

    Options:
      --version             show program's version number and exit
      -h, --help            show this help message and exit
      -d DEVICE, --device=DEVICE
                            Specifies v4l2 device to grab image from.
      -v, --verbose         Sets the output to verbose.
      -o OPTION, --option=OPTION
                            Sets any toonloop option by name.
      -l, --list-options    Prints the list of options and exit.
      -f FPS, --fps=FPS     Sets the rendering frame rate.
      -t INTERVALOMETER_RATE_SECONDS, --intervalometer-rate-seconds=INTERVALOMETER_RATE_SECONDS
                            Sets intervalometer interval in seconds.
      -H TOONLOOP_HOME, --toonloop-home=TOONLOOP_HOME
                            Path to saved files.
      -i, --intervalometer-on
                            Starts the intervalometer at startup.
      -e, --intervalometer-enabled
                            Enables/disables the use of the intervalometer.
      -w IMAGE_WIDTH, --image-width=IMAGE_WIDTH
                            Width of the images grabbed from the camera.
