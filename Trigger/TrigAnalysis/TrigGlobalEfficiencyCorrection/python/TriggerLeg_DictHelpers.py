# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Helper functions to convert cfg file into python dictionaries

from AthenaCommon.Utils.unixtools import find_datafile


def TriggerDict():

    # Dictionary from TrigGlobalEfficiencyCorrection/Triggers.cfg
    # Key is trigger chain (w/o HLT prefix)
    # Value is empty for single leg trigger or list of legs

    with open(find_datafile('TrigGlobalEfficiencyCorrection/Triggers.cfg'), 'r') as file:
        file_content = file.read()

    # Split the content by lines
    lines = file_content.strip().split('\n')
    # Initialize an empty dictionary to hold the results
    trigger_dict = {}
    # Loop through each line in the file content
    for line in lines:
        # Split the line by whitespace
        parts = line.split()
        if not parts:
            continue
        # The first element is the key, the rest are the values
        key = parts[0]
        if key.startswith('#'):
            continue
        values = parts[1:]
        # Add to dictionary
        trigger_dict[key] = values
    return trigger_dict


def MapKeysDict(target_version):

    # Dictionary from TrigGlobalEfficiencyCorrection/MapKeys.cfg
    # Key is year_leg
    # Value is list of configs available

    with open(find_datafile('TrigGlobalEfficiencyCorrection/MapKeys.cfg'), 'r') as file:
        file_content = file.read()

    # Split the content by lines
    lines = file_content.strip().split('\n')

    # Initialize an empty dictionary to hold the results
    trigger_dict = {}

    # Variables to keep track of the current version and if we are in the right version section
    current_version = None
    in_target_version = False

    for line in lines:
        # Remove leading and trailing whitespaces
        line = line.strip()

        # Skip empty lines and comment lines
        if not line or line.startswith('#'):
            continue

        # Check for version lines
        if line.startswith("[VERSION]"):
            current_version = line.replace("[VERSION]", "").strip()
            in_target_version = target_version in current_version
            continue

        # If we are in the target version section, parse the entries
        if in_target_version:
            parts = line.split()
            if len(parts) < 3:
                continue
            key = f"{parts[0]}_{parts[1]}"
            # skip parts that start with a year
            values = [part for part in parts[2:] if not part.startswith(parts[0])]
            trigger_dict[key] = values

    return trigger_dict
