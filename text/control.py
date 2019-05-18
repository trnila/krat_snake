import pygame
import serial
BLUE = (0, 0, 255)

s = serial.Serial("/dev/ttyACM0", 9600)
pygame.init()
win = pygame.display.set_mode((400, 400))
pygame.draw.rect(win, BLUE, (100, 100, 50,50))

while True:
    for event in pygame.event.get():
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_LEFT:
                s.write(b'l')
    pygame.display.update()
    pygame.time.wait(100)

