import pygame
import random
from pygame.locals import *
 
if __name__ == '__main__':
    SW,SH = 640,480
    screen = pygame.display.set_mode((SW,SH))
    pygame.display.set_caption('this is a test')
    
    _quit = False
    while not _quit:
        for e in pygame.event.get():
            if (e.type is KEYDOWN and e.key == K_f):
                pygame.display.toggle_fullscreen()
            if e.type is QUIT: 
                _quit = True
            if e.type is KEYDOWN and e.key == K_ESCAPE: 
                _quit = True
        """
        Random squares.
        """
        screen = pygame.display.get_surface()
        rr = random.randrange
        screen.fill((rr(0,256),rr(0,256),rr(0,256)),(rr(0,SW),rr(0,SH),32,32))
        pygame.display.flip()

