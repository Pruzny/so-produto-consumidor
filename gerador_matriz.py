from sys import argv
from random import random

reserved = {
    "-n": "N",
    "-i": "I",
    "-j": "J",
    "-min": "MIN",
    "-max": "MAX",
}

values = {
    "N": 10,
    "I": 10,
    "J": 10,
    "MIN": -1,
    "MAX": 1,
}

for flag in reserved.keys():
    for i in range(len(argv)):
        if argv[i] == flag:
            values[reserved[flag]] = argv[i + 1]

N = int(values["N"])
I = int(values["I"])
J = int(values["J"])
MIN = float(values["MIN"])
MAX = float(values["MAX"])

for n in range(N):
    file = open(f"matriz{n}.txt", "w")
    for i in range(I):
        for j in range(J):
            value = random() * (MAX - MIN) + MIN
            file.write(f"{value:.1f} ")
        file.write("\n")
    file.close()
