#!/usr/bin/python3

import json

def removeMovesetLowest(moveset):
    lowest = 101
    for move in moveset:
        if move["level"] < lowest:
            lowest = move["level"]
    for move in moveset[:]:
        if move["level"] == lowest:
            moveset.remove(move)
            break

    return moveset

def getMovesetLowestLevel(moveset):
    lowest = 101
    for move in moveset:
        if move["level"] < lowest:
            lowest = move["level"]

    return lowest

def getLastFourMoves(learnset, level):
    moveset = list()

    for lmove in learnset:
        if level < lmove["level"]:
            continue
        if len(moveset) < 4:
            moveset.append(lmove)
        else:
            if lmove["level"] >= getMovesetLowestLevel(moveset):
                moveset = removeMovesetLowest(moveset)
                moveset.append(lmove)

    return moveset
        


with open('poke.json') as f:
    pokemon_data = json.load(f)

with open("trainers.party") as f:
    lines = f.readlines()

pokemon = ""
ivs = ""
level = 0

for l in lines:
    l = l.strip()
    if l.startswith("-"):
        pokemon = ""
        level = 0
        ivs = ""
    if pokemon and ivs and level and not l.startswith("-"):
        gen3learnset = pokemon_data[pokemon.lower().replace(" ", "_")]["learnsets"]["gen3"]
        moveset = getLastFourMoves(gen3learnset, level)
        pokemon = ""
        ivs = ""
        level = 0
        for move in moveset:
            print("-", move["move"].title())
        print(l)
        continue

    print(l)

    if "=" not in l and ":" not in l and not l.startswith("-"):
        pokemon = l.split("@")[0].strip()
        if pokemon == "Castform":
            pokemon = "castform_normal"


    elif l.startswith("IVs:"):
        ivs = l
    elif l.startswith("Level:"):
        level = int(l.split(":")[-1])
    elif l == "":
        pokemon = ""
        ivs = ""
        level = 0
    
