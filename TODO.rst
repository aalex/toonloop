Things to do for version 1.0 beta
=================================

 - gl: display text (fastest as possible and without GLUT if it is better)
 - gl: redimension rendering area when in full screen (or not: configurable)
 - gl: Modify toon/glsl_shader.py (ShaderProgram) to conform to Python PEP 8. Add the shader inline in toon/keying.py
 - display frame number on both sides
 - Implement basic OSC callbacks (intervalometer, frame, save, reset, shot)
 - In the fragment keying shader, pass alpha color from the vertex shader.
 - use shot id for file name when saving a movie
 - installation instructions for Mac OS X 10.4 Intel (from scratch) See trunk/doc/INSTALL.txt

Things for later (version 1.0 rc_1)
===================================

 - project name choice
 - gl: take a lot of low-res textures and disply in a loop a the bottom
 - shot name/id
 - jellyfy project (name, date, folder, file, images, movies)
 - Press 'p' to open the Quicktime video camera settings dialog. (if available)
 - Press LEFT or RIGHT to move the insertion point
 - x-offset configuration parameter
 - MIDI ctl input using python
 - Playback direction : forward, backward, back&forth. 

Version 1.0 (Installer)
=======================
 - static files in /usr/local/share/toonloop/ or other location
 - install.py and py2app script. 

