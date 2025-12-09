#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# This scripts selects the required set of modules from the geometry.json (contains all the modules from the Strip/Pixel Detectors) obtained using geometry_dat_to_json.py script.
# Each functions contains a small description of its purpose.

import json
import random
import os

def load_json(file_path = "Geometry.json"):

    '''
    Function to load properly module informations from an input json file.
    '''
    
    if file_path.endswith(".json"):
        try:
            with open(file_path, "r") as file:
                return json.load(file)
        except Exception as e:
            print(f"Can not read file at {file_path}: {e}.")
            return {}
    else:
        print("Unexpected input file or format.")
        return {}
    
def find(bec = None, layer_disk = None, phi = None, eta = None, side = None, asdec = False, input_data = "Geometry.json", output_file = "Geometry_find.json"):
    
    '''
    this can look for specific set of modules based on parameters: bec(barrel or end cap), layer_disk, phi, eta and side.
    '''

    data = load_json(input_data)
    IDs = []

    for ID, info in data.items():
        if bec is not None and int(info["BEC"]) != bec:
            continue

        if layer_disk is not None and int(info["LayerDisk"]) != layer_disk:
            continue

        if phi is not None and int(info["PhiModule"]) not in [i for i in phi]:
            continue
        
        if eta is not None and int(info["EtaModule"]) > eta:
            continue

        if side is not None and int(info["Side"]) != side:
            continue
        
        if asdec:
            info["Decimal_ID"] = str(int(ID, 16))

        IDs.append(ID)

    with open(output_file, "w") as f:
        selected_data = {ID: data[ID] for ID in IDs}
        json.dump(selected_data, f, indent=4)

    return IDs

def merge(file_1 = "Geometry_1.json", file_2 = "Geometry_2.json", output_file = "Geometry_merge.json"):
    
    '''
    Function to merge any two json files produced with different configuration.
    '''
    
    data1 = load_json(file_1)
    data2 = load_json(file_2)
    IDs = []

    IDs.extend(data1.keys())
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

def select_random(frac = 0, input_data = "Geometry.json", output_file = "Geometry_rand.json", seed = None):

    '''
    Function to select a given fraction of modules randomly from all the modules using Geometry.json as input. The seed can be change.
    '''
    
    data = load_json(input_data)
    IDs = []

    num = int(round(frac * len(data), 0))
    
    if seed:
        random.seed(seed)
    
    # Use of random.sample() instead of random.choice() to avoid selecting the same module multiple times.
    IDs = random.sample(list(data.keys()), k=num) 

    for ID, info in data.items():
        info["Decimal_ID"] = str(int(ID, 16))

    with open(output_file, "w") as f:
        # In a dictionary, keys must be unique. 
        # Therefore, if the same module is selected multiple times, 
        # the key will only appear once. 
        # As a result, the number of modules selected may not match the desired count.
        selected_data = {ID: data[ID] for ID in IDs}
        json.dump(selected_data, f, indent=4)

    return IDs      

def generate_uncorr(fractions = [], input_data = "Geometry.json", prefix = "Geometry_rand", output_dir = None, seed = None):
    
    '''
    Function to generate uncorrelated files. Each file contains randomly selected modules according to the values in fractions.

    Exemple : 
        Running : 
            generate_uncorr(fraction=[1,5], input_data = "PixelGeometry.json", prefix = "PixelGeometry" output_dir = "path/to/dir")
        will create 2 files:
            - PixelGeometry_1pct_corr.json : Contains 1% of modules from PixelGeometry.json file.
            - PixelGeometry_5pct_corr.json : Contains 5% of modules from PixelGeometry.json file (Some module can be in the first file but not the second one and vis versa.)
    '''
    
    if output_dir is None:
        raise ValueError("No output_dir was provided.")

    IDs = []
    for frac in sorted(fractions):
        output_file = f"{output_dir}/{prefix}_{int(frac*100)}pct_uncorr.json"
        data = select_random(frac, input_data, output_file, seed)
        
        IDs.append(data)

    return IDs

def generate_corr(fractions = [], input_data = "geometry.json", prefix = "Geometry_rand", output_dir = None, seed = None):

    '''
    Generate correlated files using a random selection for each fractions, but modules that have been selected are kept in the next files and removed from the input_data file.
    
    Exemple : 
        Running :
            generate_corr(fraction=[1,5], input_data = "PixelGeometry.json", prefix = "PixelGeometry" output_dir = "path/to/dir")
        will create 3 files : 
            - PixelGeometry_1pct_corr.json : Contains 1% of modules from PixelGeometry.json file
            - PixelGeometry_5pct_corr.json : Contains all selected modules in the 1% file + additional Pixel modules to get to a total of 5% module w.r.t to the input file, PixelGeometry.json
            - PixelGeometry_not_selected.json : Contains all modules that have not been randomly selected.

    '''
    
    if output_dir is None:
        raise ValueError("No output_dir was provided.")
    
    not_selected_yet_file=f"{output_dir}/{prefix}_not_selected.json"

    IDs = []
    output_files = []

    for i, frac in enumerate(sorted(fractions)):
        pct = int(frac*100)
        output_file = f"{output_dir}/{prefix}_{pct}pct_corr.json"
        output_file_temp = f"{output_dir}/{prefix}_{pct}pct_temp.json"
        
        if i == 0:
            data = select_random(frac, input_data, output_file, seed)
            data_not_masked = difference(input_data, output_file, not_selected_yet_file)
            output_files.append(output_file)

        else:
            new_frac = frac_convertor(frac, input_data, output_files[i-1], not_selected_yet_file)

            select_random(new_frac, not_selected_yet_file, output_file_temp, seed)
            data_not_masked = difference(not_selected_yet_file, output_file_temp, not_selected_yet_file)

            data = merge(output_files[i-1], output_file_temp, output_file)
            output_files.append(output_file)

            os.remove(output_file_temp)

        IDs.append(data)

        if i == len(fractions) - 1:
            IDs.append(data_not_masked)    

    return IDs

def difference(file_1 = "Geometry_1.json", file_2 = "Geometry_2.json", output_file = "Geometry_diff.json"):
    
    '''
    Function to perform the differnence between two json files produces with different configuration
    '''
    
    data1=load_json(file_1)
    data2=load_json(file_2)
    IDs = []

    IDs = [ID for ID in list(data1.keys()) if ID not in list(data2.keys())]

    with open(output_file, "w") as f:
        selected_data = {ID : data1[ID] for ID in IDs}
        json.dump(selected_data, f, indent=4)

    return IDs

def frac_convertor(frac = None, file_1 = "Geometry.json", file_2 = "Selected_geometry.json", file_3 = "Not_selected_yet.json"):
    
    '''
    Function used in the generate_corr() function to compute the new fraction of module to select.

    Since the input_data file in select_random() contains less and less modules as one selects them, the fraction of module to be selected has to be recompute to match the desired one.

    Exemple : 
        In file A, there are 100 modules. One wants to create two correlated files : 
            - file B : 10% of modules ( size = 10 modules )
            - file C : 50% of modules ( size = 50 modules = 10 modules from file A + 40 new modules )

            For file C, one can not use 0.5 as a fraction because it would select 45 modules and file C would contains 55 modules instead of 50, as desired:
            
                # of selected modules = len(file A - file B) * frac = (100 - 10) * 0.5 = 45
                So, 45 + 10 = 55 !

            The new fraction should be:

                new_frac = [ (number of wanted modules in final file) - (number of already selected modules) ] / (number of not yet selected modules) 
                         = [ (len(file A) * frac) - len(file B) ] / (len(file A - file B))
                         = [ 50 - 10 ] / 90
                         =  4/9 ~ 0.4445

            So the file C contains :

                # of selected modules = len(file A - file B) * new_frac = (100 - 10) * 4/9 = 40
                So, 40 + 10 = 50 !
    '''
 
    data1=load_json(file_1)
    data2=load_json(file_2)
    data3=load_json(file_3)

    nIDs1=len(data1)
    nIDs2=len(data2)
    nIDs3=len(data3)

    nIDs=int(frac*nIDs1) - nIDs2
    new_frac = nIDs / nIDs3 if nIDs3 > 0 else 1

    return new_frac

if __name__ == "__main__": 

    current_dir = os.path.dirname(os.path.abspath(__file__)) # For the examples to work.

    '''
    Generate 1 file with pixel modules from the layer 0 in the barrel (i.e the InnerMostLayer).
    '''

    data = find(bec = 0, layer_disk = 0, phi = None, eta = None, side = None, asdec = False, input_data = "PixelGeometry.json", output_file = "IBL_modules.json")


    '''
    Generate 1 file with 5% of strip modules.
    '''

    # data = select_random(frac = 0.5, input_data = "StripGeometry.json", output_file= "StripGeometry_rand_5pct.json", seed = 1234)


    '''
    Generate 4 uncorrelated files with randomly selected modules according to 1, 5, 10 and 15% of the pixel modules in the input_data file (e.g PixelGeometry.json). 
    '''

    # data = generate_uncorr(fractions = [0.01, 0.05, 0.1, 0.15], input_data = "PixelGeometry.json", prefix = "PixelGeometry", output_dir = current_dir, seed = 1234)


    '''
    Generate 4 correlated files with randomly selected modules according to 1, 5, 10 and 15% of the input_data file size (e.g PixelGeometry.json). 
    Each file contains modules that have been masked in the previous configuration.
    '''

    # data = generate_corr(fractions = [0.01, 0.05, 0.1, 0.15], input_data = "PixelGeometry.json", prefix = "PixelGeometry", output_dir = current_dir, seed = 1234)
