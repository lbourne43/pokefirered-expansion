#!/usr/bin/python3

with open("trainers.orig") as f:
    lines = f.readlines()

for l in lines:
    l = l.strip()
    if l.startswith("Level:"):
        parts = l.split()
        new_level = int(parts[1]) * 2
        if new_level > 100:
            new_level = 100
        print(parts[0], new_level)
    else:
        print(l)
