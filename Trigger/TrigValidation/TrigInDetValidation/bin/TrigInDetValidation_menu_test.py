#!/usr/bin/env python
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
import importlib

from TrigInDetValidation.Chains import Chains
tc = Chains()

tida_dict = tc.get_menu_dict()

menu_chains = {}

for slice, cfg in tida_dict.items():
    menus = []
    if(cfg["menu"] != ''):
        menus.append(cfg["menu"])
    else:
        menus = ["MC_pp_run3_v1", "MC_pp_run4_v1"]
    
    for m in menus:
        if m not in menu_chains:
            menu_chains[m] = []
        
        menu_chains[m] += cfg["chains"]

def find_slices(chain_name):
    output = set()

    for slice, cfg in tida_dict.items():
        if chain_name in cfg["chains"]:
            output.add(slice)

    return output

def get_menu(name):
    # Import menu by name
    menumodule = importlib.import_module(f'TriggerMenuMT.HLT.Menu.{menu_name}')
    menu = menumodule.setupMenu()

    output = []

    for group, chains in menu.items():
        output += [c.name for c in chains]
    
    return output

has_missing = False

for menu_name, chains in menu_chains.items():
    valid_chains = get_menu(menu_name)

    if(len(valid_chains) < 1):
        print("Menu {} is empty, skipping".format(menu_name))
        continue

    for c in chains:
        if c not in valid_chains:
            print("!!!! {0} used in slice {1} is not a valid chain in menu {2}".format(c, find_slices(c), menu_name))
            has_missing = True

if has_missing:
    print("FAIL: Some chains are defined for TrigInDetValidation tests but are not defined in the menu")
    exit(255)