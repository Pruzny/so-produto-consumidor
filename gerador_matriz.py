from sys import argv
from random import random
from os import remove, listdir, mkdir, path

reserved = {
    "-min": "MIN",
    "-max": "MAX",
}

values = {
    "MIN": -1,
    "MAX": 1,
}

for flag in reserved.keys():
    for i in range(len(argv)):
        if argv[i] == flag:
            values[reserved[flag]] = argv[i + 1]

N = 50
I = 10
J = 10
MIN = float(values["MIN"])
MAX = float(values["MAX"])

if not path.exists("files"):
    mkdir("files")
for filename in listdir("files"):
    if ".in" in filename:
        remove(f"files/{filename}")
file_list = open("files/entrada.in", "w")
for n in range(N):
    file = open(f"files/matriz{n}.in", "w")
    for _ in range(2):
        for i in range(I):
            for j in range(J):
                value = random() * (MAX - MIN) + MIN
                file.write(f"{value} ")
            file.write("\n")
        file.write("\n")
    file.close()
    file_list.write(f"matriz{n}.in\n")
file_list.close()
