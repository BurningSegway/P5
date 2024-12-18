import matplotlib.animation as animation
import numpy as np
import matplotlib.pyplot as plt

# Bottle parameters (cylindrical bottle)
bottle_height = 20  # cm
bottle_radius = 5   # cm
bottle_mass = 100   # g (empty bottle)

# Water parameters
water_density = 1   # g/cm³
initial_water_height = bottle_height / 2  # Half-filled bottle
water_mass = water_density * np.pi * bottle_radius**2 * initial_water_height

# Tilt angles
tilt_angles = np.linspace(0, np.pi/2, 100)  # From 0° to 90°

# Function to compute center of mass
def compute_com(tilt_angle, remaining_water_height):
    # Bottle's CoM (fixed at the middle for a uniform bottle)
    bottle_com = np.array([0, bottle_height / 2])

    # Water's CoM (depends on tilt and water distribution)
    water_volume = np.pi * bottle_radius**2 * remaining_water_height
    water_com_x = (remaining_water_height / 2) * np.sin(tilt_angle)
    water_com_z = (bottle_height - (remaining_water_height / 2)) * np.cos(tilt_angle)
    water_com = np.array([water_com_x, water_com_z])

    # Combined CoM (weighted average)
    total_mass = bottle_mass + water_volume * water_density
    total_com = (bottle_mass * bottle_com + water_volume * water_density * water_com) / total_mass

    return total_com

# Compute distance of CoM from geometric center for various tilt angles
distances = []
for angle in tilt_angles:
    com = compute_com(angle, initial_water_height)
    distance_from_center = np.linalg.norm(com - np.array([0, bottle_height / 2]))
    distances.append(distance_from_center)

# Plot results
plt.figure(figsize=(10, 6))
plt.plot(np.degrees(tilt_angles), distances, label='Distance of CoM from Geometric Center')
plt.xlabel('Tilt Angle (degrees)')
plt.ylabel('Distance from Geometric Center (cm)')
plt.title('Change in Center of Mass During Pouring')
plt.legend()
plt.grid(True)
plt.show()


# Parameters for simulation
frames = 100  # Number of frames in the animation
angles = np.linspace(0, np.pi / 2, frames)  # Tilt angles
water_heights = np.linspace(initial_water_height, 0, frames)  # Water height decreasing

# Initialize the plot
fig, ax = plt.subplots(figsize=(6, 10))
ax.set_xlim(-bottle_radius * 2, bottle_radius * 2)
ax.set_ylim(0, bottle_height * 1.2)
ax.set_aspect('equal')
ax.set_title("Bottle Tilting and Center of Mass")
ax.set_xlabel("Horizontal (cm)")
ax.set_ylabel("Vertical (cm)")

# Elements to animate
bottle_outline, = ax.plot([], [], 'k-', lw=2, label="Bottle Outline")
water_fill, = ax.fill([], [], 'blue', alpha=0.4, label="Water")
com_point, = ax.plot([], [], 'ro', label="Center of Mass")

# Draw the bottle outline function
def draw_bottle(angle):
    # Create bottle outline points
    x_top = bottle_radius * np.sin(angle)
    z_top = bottle_height * np.cos(angle)
    x_base = -x_top
    z_base = 0
    return [x_base, x_top, -x_top, -x_base, x_base], [z_base, z_top, z_top, z_base, z_base]

# Draw the water function
def draw_water(angle, water_height):
    if water_height <= 0:
        return [], []  # No water left
    water_top_x = (water_height / 2) * np.sin(angle)
    water_top_z = (bottle_height - water_height / 2) * np.cos(angle)
    return [-water_top_x, water_top_x, water_top_x, -water_top_x], [0, 0, water_top_z, water_top_z]

# Animation update function
def update(frame):
    angle = angles[frame]
    water_height = water_heights[frame]

    # Update bottle outline
    bottle_x, bottle_z = draw_bottle(angle)
    bottle_outline.set_data(bottle_x, bottle_z)

    # Update water fill 
    water_x, water_z = draw_water(angle, water_height)
    water_fill.set_xy(np.column_stack((water_x, water_z)) if len(water_x) > 0 else np.empty((0, 2)))

    # Update CoM
    com = compute_com(angle, water_height)
    com_point.set_data(com[0], com[1])

    return bottle_outline, water_fill, com_point

# Create animation
ani = animation.FuncAnimation(fig, update, frames=frames, blit=True, interval=50)

# Display the animation
plt.legend()
plt.show()
