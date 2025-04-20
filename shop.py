#!/usr/bin/python3

import yaml

def genInventories(name, data):
    nums = ['zero', 'one', 'two', 'three', 'four', 'five', 'six', 'seven', 'eight']
    inv_types = {'balls': None, 'potions': None, 'status': None, 'others': None}
    compiled_inv = {'zero': inv_types.copy(), 'one': inv_types.copy(), 'two': inv_types.copy(), 'three': inv_types.copy(), 'four': inv_types.copy(), 'five': inv_types.copy(), 'six': inv_types.copy(), 'seven': inv_types.copy(), 'eight': inv_types.copy()}
    x = 0
    for num in nums:
        y = 0
        for numa in nums:
            if y <= x:
                if compiled_inv[num]['others'] is None:
                    compiled_inv[num]['others'] = list()
                if data[name][numa] is not None and 'others' in data[name][numa]:
                    compiled_inv[num]['others'] += data[name][numa]['others']
            y += 1
        x += 1
    for num in nums:
        genInventory(name, num, compiled_inv[num])

    name = name.title()
    if name == "Tm":
        name = name.upper()
    print("static const u16 *const sNettux%sInventories[] =" % name)
    print("{")
    for num in nums:
        num = num.title()
        if num == "Eight":
            print("    sNettux%sInventory_EightBadges" % (name))
        else:
            print("    sNettux%sInventory_%sBadges," % (name, num))
    print("};")
    print()

def genInventory(name, num, inv):
    num = num.title()
    name = name.title()
    if name == "Tm":
        name = name.upper()
    print("static const u16 sNettux%sInventory_%sBadges[] = {" % (name, num))
    if inv is not None:
        if 'balls' in inv and inv['balls'] is not None:
            for item in inv["balls"]:
                item = item.upper().replace(" ", "_")
                print("    ITEM_%s," % item)
        if 'potions' in inv and inv['potions'] is not None:
            for item in inv["potions"]:
                item = item.upper().replace(" ", "_")
                print("    ITEM_%s," % item)
        if 'status' in inv and inv['status'] is not None:
            for item in inv["status"]:
                item = item.upper().replace(" ", "_")
                print("    ITEM_%s," % item)
        if 'others' in inv and inv['others'] is not None:
            for item in inv["others"]:
                item = item.upper().replace(" ", "_")
                print("    ITEM_%s," % item)
    print("    ITEM_NONE")
    print("};")
    print()


with open("shop.yml") as stream:
    try:
        data = yaml.safe_load(stream)
    except yaml.YAMLError as exc:
        print(exc)
        exit(1)

genInventories('mart', data)
genInventories('battle', data)
genInventories('training', data)
genInventories('tm', data)
genInventories('evolve', data)
