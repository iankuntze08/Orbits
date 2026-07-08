import numpy as np
import matplotlib.pyplot as plt

def main():
    file = open("energyRecords.txt")
    lines = file.readlines()
    lines.pop(0)

    nums = []
    for x in range(len(lines)):
        try:
            val = float(lines[x])
        except ValueError: 
            continue
        if (val < 0.0): 
            continue
        nums.append(val)

    energies = np.array(nums)
    mean = np.mean(energies)

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(6, 7))
    
    ax1.plot(np.arange(0, len(energies)), energies, ".")
    ax1.set_xlabel("Time")
    ax1.set_ylabel("Energy")

    ax2.plot(np.arange(0, len(energies)), energies - mean, ".")
    ax2.set_xlabel("Time")
    ax2.set_ylabel("Error")
    # ax2.set_yscale("log")
    ax2.axhline(0, color="red", linestyle="--")


    print(f"Maximum energy: {energies.max()} J")
    print(f"Minimum energy: {energies.min()} J")
    print(f"Mean: {mean}")
    print(f"Standard deviation: {np.std(energies)}")

    plt.show()

if __name__ == "__main__":
    main()