import numpy as np
import matplotlib.pyplot as plt
from scipy.spatial.transform import Rotation as R

def vis_viva(mass: float, distance: float, sma: float):
    return (mass * ((2.0 / distance) - (1.0 / sma))) ** 0.5

def orbital_vel(mass: float, distance: float) -> float:
    return (mass / distance) ** 0.5

def perp(vec: np.ndarray):
    return np.array([-vec[2], vec[1], vec[0]])

def length(vec: np.ndarray):
    return ((vec[0] ** 2.0) + (vec[1] ** 2.0) + (vec[2] ** 2.0)) ** 0.5

def to_cpp_code(output: str):
    output = output.replace(" ", "")
    output = output.replace("[", "{")
    output = output.replace("]", "}")
    output = output.replace(".,", ".0,")
    output = output.replace(".}", ".0}")
    output = output.replace("-0.", "0.")
    output = output.replace(". ", ".0")
    output = output.replace(",", ", ")
    # super redundant ... but it gets the job done
    
    print(output)
    return output


def sync_orbits(distance: float, num_sats: int):
    dtheta = 6.28318531 / num_sats

    positions = []
    velocities = []

    r = R.from_quat([0, 0, np.sin(np.pi/4), np.cos(np.pi/4)])
    plane_normal = np.array([0.0, 1.0, 0.0])

    for n in range(num_sats):
        pos = np.array([distance * np.cos(dtheta * n), 0.0, distance * np.sin(dtheta * n)])
        positions.append(pos)
        tangent = np.cross(plane_normal, pos)
        tangent /= np.linalg.norm(tangent)
        vel = tangent * orbital_vel(1.0, distance) # velocity perpendicular to pos
        axis = pos / np.linalg.norm(pos) # normalized pos
        r = R.from_rotvec(axis * 45.0) # rotate along pos vector
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
    o1, o2 = sync_orbits(8.0, 10)
    # print(f"{o1}\n\n{o2}")
    to_cpp_code(f"positions:\n{np.array2string(o1, precision=5, separator=", ", suppress_small=True)}\nvelocities:\n{np.array2string(o2, precision=5, separator=", ", suppress_small=True)}")
    fig1, (ax1, ax2) = test_positions_velocities(o1, o2)
    plt.show()

if __name__ == "__main__":
    main()