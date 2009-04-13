ToonLoop documentation
======================

:Author: Alexandre Quessy
:Copyright: GNU General Public License.

.. Note:: ToonLoop is a stop motion tool. See http://toonloop.com for more informations.
  Using a simple video camera, users can take snapshots of drawings
  or objects and create an animation movies with those images. Creating 
  a movie is as simple as that in ToonLoop. 

.. contents::

You can browse the directory of `ToonLoop saved files`_.
Soon, there will also be a RSS feed and a web form.

-- 
.. _ToonLoop saved files: files

=======================================================
 ToonLoop Installation Notes for Ubuntu GNU/Linux 8.10
=======================================================

Install Dependencies
--------------------

Subversion::

  sudo apt-get install subversion

Python Packages::

  sudo apt-get install python-twisted python-numpy python-opengl python-pyglew python-dev

The SDL Library::
  
  sudo apt-get install libsdl1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-ttf2.0-dev libsdl-gfx1.2-dev  libsdl-sound1.2-dev libsmpeg-dev 

libpng, libjpeg and portmidi::

  sudo apt-get install libpng-dev libjpeg-dev libportmidi-dev libtiff4-dev

Mencoder ::

  sudo apt-get install mencoder

Checkout and Build Pygame
-------------------------

The camera module for pygame is available from pygame's svn revision 
1744 or greater::

  mkdir ~/src
  cd ~/src
  svn checkout svn://seul.org/svn/pygame/trunk pygame
  cd pygame 
  python setup.py build
  sudo python setup.py install

Checkout and Run ToonLoop
-------------------------
Check it out using Subversion::

  mkdir ~/src
  cd ~/src
  svn checkout https://toonloop.googlecode.com/svn/trunk/py toonloop
  cd toonloop

ToonLoop should now run::

  cd ~/src/toonloop
  ./toonloop.py

Additional Configuration Notes
------------------------------

You might need to properly configure your V4L2 video device::

  sudo apt-get install ivtv-utils
  v4l2-ctl --set-input=1
  v4l2-ctl --set-standard=ntsc
  v4l2-ctl --set-fmt-video=width=768,height=480
  v4l2-ctl --set-ctrl=saturation=65535,contrast=32768

If you use two displays, you might want to use 2 separate X screens::

  sudo nvidia-settings
  DISPLAY=:0.1 ./toonloop.py

OpenGL and GLSL for Python
--------------------------
It seems like the glUniform3f function doesn't work in the python-opengl 
package. Let's see if we can get it to work when downloaded from bzr::

  sudo apt-get install bzr
  bzr branch lp:pyopengl
  bzr branch lp:pyopengl-demo
  cd pyopengl
  sudo apt-get remove python-opengl
  python setup.py build
  sudo python setup.py install

