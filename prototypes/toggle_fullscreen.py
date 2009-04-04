import pygame
import random
from pygame.locals import *
 
def toggle_fullscreen():
    """
    Changes from fullscreen to windowed, and the way around
    """
    screen = pygame.display.get_surface()
    tmp = screen.convert()
    caption = pygame.display.get_caption()
    cursor = pygame.mouse.get_cursor()  # Duoas 16-04-2007 
    w, h = screen.get_width(), screen.get_height()
    flags = screen.get_flags()
    bits = screen.get_bitsize()
    
    pygame.display.quit()
    pygame.display.init()
    
    screen = pygame.display.set_mode((w, h), flags ^ FULLSCREEN, bits)
    screen.blit(tmp, (0, 0))
    pygame.display.set_caption(*caption)
 
    pygame.key.set_mods(0) #HACK: work-a-round for a SDL bug??
    pygame.mouse.set_cursor( *cursor )  # Duoas 16-04-2007
    return screen
 
if __name__ == '__main__':
    SW,SH = 640,480
    screen = pygame.display.set_mode((SW,SH))
    pygame.display.set_caption('this is a test')
    
    _quit = False
    while not _quit:
        for e in pygame.event.get():
            if (e.type is KEYDOWN and e.key == K_f):
                toggle_fullscreen()
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

