#!/usr/bin/env python
#
# Toonloop for Python
#
# Copyright 2008 Alexandre Quessy & Tristan Matthews
# <alexandre@quessy.net> & <le.businessman@gmail.com>
# http://www.toonloop.com
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# Toonloop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Toonloop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with Toonloop.  If not, see <http://www.gnu.org/licenses/>.
#
import pygame
from OpenGL.GL import *
"""
Draws things in OpenGL
"""
# no need for glut for now
# from OpenGL.GLUT import *
# 
# def draw_text(text, font=GLUT_STROKE_ROMAN):
#     """
#     Draws text in OpenGL
#     """
#     #font=GLUT_BITMAP_TIMES_ROMAN_24
#     for c in text:
#         glutStrokeCharacter(font, ord(c))
#         # glutBitmapCharacter(font, ord(c))

def texture_from_image(texture, image, square_texture=False):
    """
    Copies the pixels from a pygame surface to an OpenGL texture object.
    """
    textureData = pygame.image.tostring(image, "RGBX", True) # vertically flipped
    if square_texture:
        glBindTexture(GL_TEXTURE_2D, texture)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.get_width(), image.get_height(), 0, \
                  GL_RGBA, GL_UNSIGNED_BYTE, textureData)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
    else:   
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture)
        glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, image.get_width(), \
            image.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData)
        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST)

def draw_textured_square(w=None, h=None):
    """
    Draws a texture square of 2 x 2 size centered at 0, 0
    
    Make sure to call glEnable(GL_TEXTURE_RECTANGLE_ARB) first.

    :param w: width of the image in pixels
    :param h: height of the image in pixels
    """
    if w is None or h is None:
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0)
        glVertex2f(-1.0, -1.0) # Bottom Left Of The Texture and Quad
        glTexCoord2f(1.0, 0.0)
        glVertex2f(1.0, -1.0) # Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0, 1.0)
        glVertex2f(1.0, 1.0) # Top Right Of The Texture and Quad
        glTexCoord2f(0.0, 1.0)
        glVertex2f(-1.0, 1.0) # Top Left Of The Texture and Quad
        glEnd()
    else:
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0)
        glVertex2f(-1.0, -1.0) # Bottom Left
        glTexCoord2f(w, 0.0)
        glVertex2f(1.0, -1.0) # Bottom Right
        glTexCoord2f(w, h)
        glVertex2f(1.0, 1.0) # Top Right
        glTexCoord2f(0.0, h)
        glVertex2f(-1.0, 1.0) # Top Left
        glEnd()

def draw_square():
    """
    Draws a square of 2 x 2 size centered at 0, 0
    
    Make sure to call glDisable(GL_TEXTURE_RECTANGLE_ARB) first.
    """
    glBegin(GL_QUADS)
    glVertex2f(-1.0, -1.0) # Bottom Left of Quad
    glVertex2f(1.0, -1.0) # Bottom Right of Quad
    glVertex2f(1.0, 1.0) # Top Right Of Quad
    glVertex2f(-1.0, 1.0) # Top Left Of Quad
    glEnd()

def draw_horizontal_progress_bar(background_color=(0.0, 0.0, 0.0, 1.0), foreground_color=(1.0, 1.0, 1.0, 1.0), progress=0.0, line_color=(0.0, 0.0, 0.0, 1.0)):
    """
    Draws an horizontal progress bar.

    The programmer should scale this shape, since its position is between 
    -1 and 1 on both axis.
    
    Make sure to call glDisable(GL_TEXTURE_RECTANGLE_ARB) first.
    
    :param background_color: tuple of 4 floats from 0 to 1
    :param foreground_color: tuple of 4 floats from 0 to 1
    :param progress: float from 0 to 1
    """
    glColor4f(*foreground_color) 
    glBegin(GL_QUADS)
    glVertex2f(-1.0, -1.0) # Bottom Left of Quad
    glVertex2f(progress * 2.0 - 1.0, -1.0) # Bottom Right of Quad
    glVertex2f(progress * 2.0 - 1.0, 1.0) # Top Right Of Quad
    glVertex2f(-1.0, 1.0) # Top Left Of Quad
    glEnd()
    glColor4f(*background_color) 
    glBegin(GL_QUADS)
    glVertex2f(-1.0, -1.0) # Bottom Left of Quad
    glVertex2f(1.0, -1.0) # Bottom Right of Quad
    glVertex2f(1.0, 1.0) # Top Right Of Quad
    glVertex2f(-1.0, 1.0) # Top Left Of Quad
    glEnd()
    #glLineWidth(1.0)
    glColor4f(*line_color) 
    glBegin(GL_LINE_LOOP)
    glVertex2f(-1.0, -1.0) # Bottom Left of Quad
    glVertex2f(1.0, -1.0) # Bottom Right of Quad
    glVertex2f(1.0, 1.0) # Top Right Of Quad
    glVertex2f(-1.0, 1.0) # Top Left Of Quad
    glEnd()
