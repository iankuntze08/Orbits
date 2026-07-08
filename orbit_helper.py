import numpy as np

def vis_viva(mass: float, distance: float, sma: float):
    return (mass * ((2.0 / distance) - (1.0 / sma))) ** 0.5

def main():
    print(vis_viva(1.0, 7.0, 7.0))

if __name__ == "__main__":
    main()