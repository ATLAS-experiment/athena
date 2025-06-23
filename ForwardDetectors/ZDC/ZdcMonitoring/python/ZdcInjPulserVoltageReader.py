#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

import json
import os

def load_voltage_steps(run_number, voltage_values, voltage_strings, file_path="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/ZdcConditions/INJpulser_combined_2024.json"):
    """Reads a JSON file, determines the correct voltage step configuration for a given run number, 
    and fills two lists:
      - voltage_values: List of raw float values after applying ScaleFactor.
      - voltage_strings: List of string representations rounded to 5 decimal places in "X.XXXXX" format.

    Args:
        run_number (int): The run number to find the voltage steps for.
        voltage_values (list): A list that will be filled with the raw scaled voltage steps as floats.
        voltage_strings (list): A list that will be filled with the formatted voltage steps as strings.
        file_path (str): The path to the JSON file. Defaults to the provided path.
    """
    
    if not os.path.exists(file_path):
        print(f"Error: File not found at {file_path}")
        return
    
    # Load JSON file
    with open(file_path, "r") as file:
        data = json.load(file)
    
    # Determine the correct StepsConfig and ScaleFactor for the given run number
    steps_config_name = None
    scale_factor = None

    for run_range in data["RunRanges"]:
        if run_range["First"] <= run_number < run_range["Last"]:
            steps_config_name = run_range["StepsConfig"]
            scale_factor = run_range["ScaleFactor"]
            break
    
    if not steps_config_name or scale_factor is None:
        print(f"Warning: No voltage step configuration found for run number {run_number}.")
        return
    
    # Retrieve the voltage step configuration
    step_config = data["StepConfigurations"].get(steps_config_name)
    if not step_config:
        print(f"Error: Step configuration '{steps_config_name}' not found in the JSON file.")
        return
    
    # Clear the voltage lists
    voltage_values.clear()
    voltage_strings.clear()

    # Generate the voltage steps, apply ScaleFactor, and store in both lists
    for step in step_config:
        if "nStep" in step and "vStart" in step and "vStep" in step:
            n_steps = step["nStep"]
            v_start = step["vStart"]
            v_step = step["vStep"]
            
            for i in range(n_steps):
                voltage = (v_start + i * v_step) * scale_factor
                voltage_values.append(voltage)
                # voltage_strings.append(f"{round(voltage, 5):.5f}V")
                voltage_strings.append(f"{int(voltage*1000000):d}muV")

