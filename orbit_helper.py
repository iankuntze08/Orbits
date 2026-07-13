import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial.transform import Rotation as R
import argparse

def vis_viva(mass: float, distance: float, sma: float):
    return (mass * ((2.0 / distance) - (1.0 / sma))) ** 0.5

def orbital_vel(mass: float, distance: float) -> float:
    return (mass / distance) ** 0.5

def perp(vec: np.ndarray):
    return np.array([-vec[2], vec[1], vec[0]])

def length(vec: np.ndarray):
    return ((vec[0] ** 2.0) + (vec[1] ** 2.0) + (vec[2] ** 2.0)) ** 0.5

def to_cpp_code(arr_1: np.ndarray, arr_2: np.ndarray):
    output = f"{np.array2string(arr_1, precision=5, separator=", ", suppress_small=True)}\n{np.array2string(arr_2, precision=5, separator=", ", suppress_small=True)}"
    
    output = output.replace(". ", ".0")
    output = output.replace(" ", "")
    output = output.replace("[[", "glm::vec3(")
    output = output.replace("]]", ")\n+")
    output = output.replace("]", ")")
    output = output.replace("[", "glm::vec3(")
    output = output.replace(",\n", "\n")
    output = output.replace(",", ", ")
    output = output.replace("-0.0,", "0.0,")

    vel_and_pos = list(filter(bool, output.split("+")))
    pos_lines = vel_and_pos[0].splitlines()
    vel_lines = vel_and_pos[1].splitlines()
    vel_lines.pop(0)

    final = ""

    for i in range(len(pos_lines)):
        if (i != len(pos_lines) - 1):
            final += "Body{" + pos_lines[i] + ", " + vel_lines[i] + ", 0.5},\n"
        else:
            final += "Body{" + pos_lines[i] + ", " + vel_lines[i] + ", 0.5}\n"

    
    print(final)
    return output


def sync_orbits(distance: float, num_sats: int, inclination: float):
    dtheta = 6.28318531 / num_sats

    positions = []
    velocities = []

    plane_normal = np.array([0.0, 1.0, 0.0])

    for n in range(num_sats):
        pos = np.array([distance * np.cos(dtheta * n), 0.0, distance * np.sin(dtheta * n)])
        positions.append(pos)
        tangent = np.cross(plane_normal, pos)
        tangent /= np.linalg.norm(tangent)
        vel = tangent * orbital_vel(1.0, distance) # velocity perpendicular to pos
        axis = pos / np.linalg.norm(pos) # normalized pos
        r = R.from_rotvec(axis * inclination, degrees=True) # rotate along pos vector
        vel = r.apply(vel) # apply rotation
        velocities.append(vel)
    return np.array(positions), np.array(velocities)

def test_positions_velocities(positions: np.ndarray, velocities: np.ndarray):
    fig, (ax1, ax2) = plt.subplots(1, 2, subplot_kw={"projection": "3d"})

    px = positions[:, 0]
    py = positions[:, 1]
    pz = positions[:, 2]
    ax1.scatter(px, pz, py)

    vx = velocities[:, 0]
    vy = velocities[:, 1]
    vz = velocities[:, 2]
    ax2.quiver(px, pz, py, vx, vy, vz, length=0.1, normalize=True)

    return fig, (ax1, ax2)

def main():
    # o1, o2 = sync_orbits(2.0, 10, 45.0)
    parser = argparse.ArgumentParser("Get orbital position and velocity")
    parser.add_argument("distance", help="Distance from center", type=float)
    parser.add_argument("inclination", help="Orbital inclination", type=float)
    parser.add_argument("mass", help="Central mass", type=float)
    args = parser.parse_args()

    distance = args.distance
    inclination = args.inclination
    mass = args.mass

    args = parser.parse_args()
    vel = np.array([0.0, 0.0, orbital_vel(mass, distance)])
    r = R.from_rotvec(np.array([1.0, 0.0, 0.0]) * inclination, degrees=True)
    # 28.58 deg
    print(f"Velocity:\n{r.apply(vel)}".replace("  0.", ", 0."))
    print(f"Position:\n{np.array([distance, 0.0, 0.0])}".replace("  0.", ", 0."))
    # print(f"{o1}\n\n{o2}")
    # to_cpp_code(o1, o2)
    # fig1, (ax1, ax2) = test_positions_velocities(o1, o2)
    # plt.show()

if __name__ == "__main__":
    main()