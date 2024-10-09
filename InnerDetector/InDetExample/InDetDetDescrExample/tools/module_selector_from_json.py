# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# This scripts selects the required set of modules from the geometry.json (contains all the modules from the Strip Detector) obtained using geometry_dat_to_joson.py script.
# It has three sets of functions:
# find: this can look for specific set of modules based on parameters: bec(barrel or end cap), layer_disk, phi, eta and side.
# merge: to merge any two json files produced with different configuration
# select_random: to select a given fraction of modules randomly from all the modules (using geometry.json as input) or any other config using its list of json

import json
import random

def find(bec=None, layer_disk=None, phi=None, eta=None, side=None, asdec=False, input_data="geometry.json", output_file=None):
    data = {}

    with open(input_data) as f:
        if input_data.endswith(".json"):
            data = json.load(f)
        else:
            print("Unexpected input file or format.")
            return []

    IDs = []

    for ID, info in data.items():
        if bec is not None and info["BEC"] != str(bec):
            continue

        if layer_disk is not None and info["LayerDisk"] != str(layer_disk):
            continue

        if phi is not None and info["PhiModule"] not in str([i for i in phi]):
            continue

        if eta is not None and info["EtaModule"] > str(eta):
            continue

        if side is not None and info["Side"] != str(side):
            continue

        if asdec:
            info["Decimal_ID"] = str(int(ID, 16))

        IDs.append(ID)

    with open(output_file, "w") as f:
        selected_data = {ID: data[ID] for ID in IDs}
        json.dump(selected_data, f, indent=4)

    return IDs


def merge(file_1="selected_modules_1.json", file_2="selected_modules_2.json", output_file=None):
    data1 = {}
    data2 = {}
    IDs = []

    with open(file_1) as f1:
        if file_1.endswith(".json"):
            data1 = json.load(f1)
        else:
            print("Unexpected input file_1 or format.")
            return []

    IDs.extend(data1.keys())

    with open(file_2) as f2:
        if file_2.endswith(".json"):
            data2 = json.load(f2)
        else:
            print("Unexpected input file_2 or format.")
            return []

    IDs.extend(data2.keys())

    with open(output_file, "w") as f3:
        selected_data = {}
        for ID in IDs:
            if ID in data1:
                selected_data[ID] = data1[ID]
            elif ID in data2:
                selected_data[ID] = data2[ID]
        json.dump(selected_data, f3, indent=4)

    return IDs

def select_random(frac = None, input_data=None, output_file=None):
    data = {}

    with open(input_data) as f:
        if input_data.endswith(".json"):
            data = json.load(f)
        else:
            print("Unexpected input file or format.")
            return []

    IDs = []

    num = int(round(frac * len(data), 0))
    IDs = random.choices(list(data.keys()), k=num)

    for ID, info in data.items():
        info["Decimal_ID"] = str(int(ID, 16))

    with open(output_file, "w") as f:
        selected_data = {ID: data[ID] for ID in IDs}
        json.dump(selected_data, f, indent=4)

    return IDs      


if __name__ == "__main__": 
    #data = select_random(frac = 0.01, input_data="geometry.json", output_file="test_selected_frac_modules.json")
    data = find(bec = 0, layer_disk = 0, phi = [0,1,2,3,4], asdec = True, output_file="test_selected_modules.json")
    #data = merge(file_1="selected_modules_ec_minus_2_layer_0_side_0.json", file_2="selected_modules_ec_minus_2_layer_1_side_0.json", output_file="test_merged_file.json")