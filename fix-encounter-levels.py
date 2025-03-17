#!/usr/bin/python3

with open("encounters.orig.json") as f:
    lines = f.readlines()

for l in lines:
    l = l.rstrip()
    if "max_level" in l or "min_level" in l:
        parts = l.split(":")
        new_level = int(int(parts[1][:-1]) * 1.7)
        if new_level > 90:
            new_level = 90
        new_level = str(new_level) + ","
        print(parts[0] + ":", new_level)
    else:
        print(l)
