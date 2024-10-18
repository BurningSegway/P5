import pygame
import sys



pygame.init()

window_size = (400, 300)
screen = pygame.display.set_mode(window_size)
pygame.display.set_caption("Wrist Dashboard")

# Set up font
font = pygame.font.SysFont(None, 48)

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
BOX_COLOR = (150, 200, 255)

# Initial variable
variable_value = 42

def draw_box_with_variable(value):

    # Create the text surface with the current value
    text_surface = font.render(str(value), True, BLACK)

    # Define the box size
    box_width = 200
    box_height = 100

    box_x = (window_size[0] - box_width) // 2
    box_y = 0
    #box_y = (window_size[1] - box_height) // 2

    # Draw a box (rectangle)
    pygame.draw.rect(screen, BOX_COLOR, (box_x, box_y, box_width, box_height))

    # Get the text rectangle and center it in the box
    text_rect = text_surface.get_rect(center=(box_x + box_width // 2, box_y + box_height // 2))

    # Blit the text onto the screen
    screen.blit(text_surface, text_rect)

running = True

angle = 0
resolution = 5

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.KEYDOWN:
            # Check for specific key presses
            if event.key == pygame.K_ESCAPE:
                running = False

            if event.key == pygame.K_SPACE:
                angle = 0

            if event.key == pygame.K_LEFT:
                angle += resolution
                if angle > 90:
                    angle = 90

            if event.key == pygame.K_RIGHT:
                angle += -resolution
                if angle < -90:
                    angle = -90

    draw_box_with_variable(angle)

    
    #print(f"Control input:\nPitch: {self.pitch}\t Roll: {self.roll}\t Throttle: {self.throttle}\t Yaw: {self.yaw}")
    # Update the display
    pygame.display.flip()

pygame.quit()
sys.exit()