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

def rotate_by_quat(vec: np.ndarray, quat):
    vector = np.array([0.0, 0.0, 0.0])
    num12 = quat[0] + quat[0];
    num2 = quat[1] + quat[1];
    num = quat[2] + quat[2];
    num11 = quat[3] * num12;
    num10 = quat[3] * num2;
    num9 = quat[3] * num;
    num8 = quat[0] * num12;
    num7 = quat[0] * num2;
    num6 = quat[0] * num;
    num5 = quat[1] * num2;
    num4 = quat[1] * num;
    num3 = quat[2] * num;
    num15 = ((vec[0] * ((1 - num5) - num3)) + (vec[1] * (num7 - num9))) + (vec[2] * (num6 + num10));
    num14 = ((vec[0] * (num7 + num9)) + (vec[1] * ((1 - num8) - num3))) + (vec[2] * (num4 - num11));
    num13 = ((vec[0] * (num6 - num10)) + (vec[1] * (num4 + num11))) + (vec[2] * ((1 - num8) - num5));
    vector[0] = num15;
    vector[1] = num14;
    vector[2] = num13;
    return vector


def sync_orbits(distance: float, num_sats: int):
    dtheta = 6.28318531 / num_sats

    positions = []
    velocities = []

    quat = R.from_quat([0, 0, np.sin(np.pi/4), np.cos(np.pi/4)])
    mat = quat.as_matrix()

    for n in range(num_sats):
        pos = np.array([distance * np.cos(dtheta * n), 0.0, distance * np.sin(dtheta * n)])
        positions.append(pos)
        vec = perp(pos)
        uvec = vec / length(vec)
        vel = rotate_by_quat(uvec, quat.as_quat()) * orbital_vel(1.0, distance)
        velocities.append(vel)
    return positions, velocities

def test_positions_velocities(positions: list, velocities: list):
    fig, (ax1, ax2) = plt.subplots(1, 2, subplot_kw={"projection": "3d"})

    positions_x = np.arange(len(positions), dtype=float)
    positions_y = np.arange(len(positions), dtype=float)
    positions_z = np.arange(len(positions), dtype=float)
    for i in range(len(positions)):
        positions_x[i] = positions[i][0]
        positions_y[i] = positions[i][1]
        positions_z[i] = positions[i][2]
    ax1.scatter(positions_x, positions_z, positions_y)
    pos2 = (positions_x, positions_z)

    velocities_x = np.arange(len(velocities), dtype=float)
    velocities_y = np.arange(len(velocities), dtype=float)
    velocities_z = np.arange(len(velocities), dtype=float)
    for i in range(len(velocities)):
        velocities_x[i] = velocities[i][0]
        velocities_z[i] = velocities[i][1]
        velocities_z[i] = velocities[i][2]
    ax2.quiver(positions_x, positions_z, positions_y, velocities_x, velocities_z, velocities_y, length=0.01)
    vel2 = (velocities_x, velocities_z)

    return fig, (ax1, ax2)

def main():
    o1, o2 = sync_orbits(1.0, 20)
    print(f"{o1}\n\n{o2}")
    fig1, (ax1, ax2) = test_positions_velocities(o1, o2)
    plt.show()

if __name__ == "__main__":
    main()