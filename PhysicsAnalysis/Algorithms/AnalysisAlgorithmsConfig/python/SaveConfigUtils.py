# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AnaAlgorithm.AlgSequence import AlgSequence
from AnaAlgorithm.PythonConfig import PrivateToolConfig
from AthenaCommon.Logging import logging

import os
import json

def combine_tools_and_algorithms_ELjob(combine_dictionaries = True, text_file = 'tool_config.txt',
                                       alg_file = 'alg_sequence.json', output_file = 'my_analysis_config.json'):
    """
    Parse a tool configuration text file and merge it with an algorithm sequence JSON file.

    Args:
        combine_dictionaries (bool): Whether to load and merge with existing alg_file
        text_file (str): Path to the tool configuration text file
        alg_file (str): Path to the algorithm sequence JSON file
        output_file (str): Path where the merged configuration will be saved

    Returns:
        dict: The resulting configuration, which is a merge of the tool configuration
              and algorithm sequence JSON data.
    """
    log = logging.getLogger("SaveConfigUtils")
    log.info("Calling COMBINE_JSON_FILES with the following arguments:")
    log.info(f" - tool configuration = {text_file}")
    log.info(f" - algorithm sequence = {alg_file}")
    log.info(f" - full output config = {output_file}")

    if not os.path.exists(text_file):
        raise FileNotFoundError(f"The tool configuration file {text_file} was not found.")
    if not os.path.exists(alg_file):
        raise FileNotFoundError(f"The algorithm sequence file {alg_file} was not found.")

    # Load or initialize the configuration
    config = {}
    if combine_dictionaries:
        with open(alg_file, 'r', encoding='utf-8') as f:
            try:
                config = json.load(f)
            except json.JSONDecodeError as e:
                raise ValueError(f"Error parsing JSON in {alg_file}: {e}")

    # Process the tool configuration file
    current_tool_path = None
    current_tool_id = None
    current_tool_properties = {}

    with open(text_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or '=' not in line:
                continue

            path, value = map(str.strip, line.split('=', 1))
            parts = path.split('.')

            # There are two possiblities for 'parts':
            # 1. Declaration of a new tool
            # 2. Setting a property on an existing tool
            if len(value.split("/")) == 2 and "'" not in value and current_tool_path != parts:
                # We are in case 1, meaning we are done with the previous tool (if any)
                # Start by saving the settings of the previous tool
                _register_settings(config, current_tool_path, current_tool_id, current_tool_properties)
                # Then register the new tool
                tool_path = parts[:-1]
                tool_id = parts[-1]
                tool_name = value.split("/")[1]
                _register_new_tool(config, tool_path, tool_id, tool_name)
                # We are ready to collect information about the new tool
                current_tool_path = tool_path
                current_tool_id = tool_id
                current_tool_properties = {}
            else:
                # We are in case 2: record new settings
                property_name = parts[-1]
                current_tool_properties[property_name] = value

    # Register any remaining settings for the last tool if necessary
    if current_tool_properties and current_tool_id:
        _register_settings(config, current_tool_path, current_tool_id, current_tool_properties)

    # Write output
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(config, f, indent=4)

    return config


def _register_settings(config, path, name, settings):
    if not settings or path is None: return
    current_dict = config

    # Iterate through the path to reach the correct nested dictionary
    for key in path:
        if key not in current_dict:
            raise ValueError(f"Couldn't find key {key} in dictionary of algorithms and tools, this should not happen!")
        current_dict = current_dict[key]["Properties"]["Tools"]

    for key,value in settings.items():
        _safe_add_property(key, value, current_dict[name])


def _register_new_tool(config, path, tool_id, name):
    current_dict = config

    # Iterate through the path to reach the correct nested dictionary
    for key in path:
        if key not in current_dict:
            # Create the entry with the appropriate structure
            current_dict[key] = {"Properties": {"Tools": {}}, "Type": ""}
        else:
            # Enforce the appropriate structure
            if "Properties" not in current_dict[key].keys():
                current_dict[key]["Properties"] = {"Tools": {}}
            if "Tools" not in current_dict[key]["Properties"].keys():
                current_dict[key]["Properties"]["Tools"] = {}
        current_dict = current_dict[key]["Properties"]["Tools"]

    # Ensure the tool id exists as a key with a dictionary as its value
    if tool_id not in current_dict:
        current_dict[tool_id] = {"Properties": {}}

    # Set the 'Name:' key with the given name
    if "Properties" not in current_dict[tool_id]:
        current_dict[tool_id]["Properties"] = {}
    current_dict[tool_id]["Properties"]["Name"] = name


def save_algs_from_sequence_ELjob(sequence, output_dict):
    """
    Save the algorithms in the sequence without the sequence structure.
    Does not save the tools. You need to get them with PrintToolConfigAlg.

    Args:
        sequence (AnaAlgorithm.AlgSequence: the algorithm sequence to parse
        output_dict (dict): the dictionary to populate
    """
    # Ensure output_dict is a dictionary
    if not isinstance(output_dict, dict):
        raise ValueError("output_dict must be a dictionary")

    # Handle recursive case for AlgSequence
    if isinstance(sequence, AlgSequence):
        for alg in sequence:
            nested_dict = {}
            save_algs_from_sequence_ELjob(alg, nested_dict)
            output_dict[alg.name()] = nested_dict
    else:
        # Handle the algorithm itself
        _save_algorithm(sequence, output_dict)


def _save_algorithm(sequence, output_dict):
    """ Helper function to save individual algorithm properties """
    output_dict['Type'] = sequence.type()
    output_dict['Properties'] = {'Tools': {}}

    for key, value in sorted(sequence._props.items()):
        if isinstance(value, str):
            output_dict['Properties'][key] = value
        elif isinstance(value, PrivateToolConfig):
            _handle_tool_config(value, output_dict)
        else:
            _safe_add_property(key, value, output_dict)


def _handle_tool_config(tool_config, output_dict):
    """ Handle tool configurations inside algorithms """
    log = logging.getLogger("SaveConfigUtils")
    if tool_config._prefix in output_dict['Properties']['Tools']:
        log.warning(f"Already have this tool in the dict, prefix: {tool_config._prefix}. You need to ID the name somehow.")
    else:
        output_dict['Properties']['Tools'][tool_config._prefix] = {'Type': tool_config._type}


def _safe_add_property(key, value, output_dict):
    """ Safely adds properties to the output_dict with error handling """
    log = logging.getLogger("SaveConfigUtils")
    try:
        output_dict['Properties'][key] = value
    except Exception as e:
        log.warning(f"Couldn't add property '{key}' to dict. Value: {value}. Error: {e}")
