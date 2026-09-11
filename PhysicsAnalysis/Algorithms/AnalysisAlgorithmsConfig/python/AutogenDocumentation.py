# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Baptiste Ravina
"""
Core methods to extract options information from ConfigBlock classes and merge with
output variables metadata from a YAML file.
"""

import inspect
import yaml
import re
import logging
from typing import Any, Dict, List, Type, Optional, Union

from AthenaCommon.Utils.unixtools import find_datafile

logger = logging.getLogger("AutogenDocumentation")

def load_output_variables(yaml_filepath: str) -> Dict[str, List[Dict[str, Any]]]:
    """
    Load output variables metadata from YAML file.

    Expected YAML format:
        BlockClassName:
          - name: variable_name
            description: Variable description
            toggled_by: Optional condition description

    Args:
        yaml_filepath: Path to the YAML file

    Returns:
        Dictionary mapping block class names to their output variables
    """
    with open(yaml_filepath, "r") as f:
        data = yaml.safe_load(f)
    return data if data else {}


def extract_block_options(block_class: Type) -> Dict[str, Any]:
    """
    Extract options information from a ConfigBlock subclass.

    Args:
        block_class: A class that inherits from ConfigBlock

    Returns:
        A dictionary containing the class name and its options
    """
    # Create a temporary instance to access the options
    instance = block_class()

    # Get the options dictionary
    options_dict = instance.getOptions()

    # Extract information for each option
    options_list = []
    for option_name, option_obj in options_dict.items():
        # skip some specific options
        if option_name in ["groupName", "propertyOverrides", "ignoreDependencies"]:
            continue
        option_info = {
            "label": option_name,
            "type": option_obj.type.__name__ if option_obj.type is not None else "None",
            "default": option_obj.default,
            "info": option_obj.info,
            "required": option_obj.required,
            "noneAction": option_obj.noneAction,
            "physicalUnit": interpret_physical_unit(option_obj.info),
            "meta": option_obj.meta,
        }
        # Check if this option has expert mode settings
        if (
            hasattr(instance, "_expertModeSettings")
            and option_name in instance._expertModeSettings
        ):
            expert_rule = instance._expertModeSettings[option_name]
            if not isinstance(expert_rule, list):
                expert_rule = [expert_rule]
        else:
            expert_rule = None
        option_info["expertMode"] = expert_rule
        options_list.append(option_info)

    return {
        "class": block_class.__name__,
        "module": block_class.__module__,
        "docstring": inspect.getdoc(block_class),
        "options": options_list,
    }


def interpret_physical_unit(info: str) -> Optional[str]:
    """
    Extract a physical unit from an info string.
    Currently looks for energy units like MeV or GeV.

    Args:
        info: The information string from an option.

    Returns:
        The detected unit as a string ("MeV", "GeV", etc.) or None if no unit is found.
    """
    if not info:
        return None

    # Check for specific units
    patterns = [
        r"\[MeV\]",
        r"\(MeV\)",
        r"\(in MeV\)",
        r"\[in MeV\]",
        r"\[GeV\]",
        r"\(GeV\)",
        r"\(in GeV\)",
        r"\[in GeV\]",
        r"\[mm\]",
        r"\(mm\)",
        r"\(in mm\)",
        r"\[in mm\]",
    ]
    for pattern in patterns:
        if re.search(pattern, info):
            if "MeV" in pattern:
                return "MeV"
            elif "GeV" in pattern:
                return "GeV"
            elif "mm" in pattern:
                return "mm"
    return None


def process_info_links(info: str) -> str:
    """
    Scan an info string for backtick-enclosed substrings of the form `A::B`.
    Turn them into a link to the appropriate module/files.

    Args:
        info: The input info string.

    Returns:
        The processed string with Markdown links where applicable.
    """
    if not info:
        return info

    # Regex to match `A::B` inside backticks
    pattern = r"`([^`]+)::([^`]+)`"

    def replace_match(match):
        A, B = match.group(1), match.group(2)
        if A == "CP" or A == "ORUtils":
            url = f"https://acode-browser1.usatlas.bnl.gov/lxr/search?%21v=head&_filestring=**{B}**&_string="
            return f"[`{A}::{B}`]({url})"
        elif A == "xAOD" or A == "AthOnnx":
            url = f"https://acode-browser1.usatlas.bnl.gov/lxr/ident?v=head&_i={B}&_identdefonly=1&_remember=1"
        else:
            # TODO: any other cases to handle?
            return f"`{A}::{B}`"

    return re.sub(pattern, replace_match, info)


def link_jira_tickets(info: str) -> str:
    """
    Convert JIRA ticket references in a string to Markdown links.

    - JIRA tickets are of the form: all-caps letters, a dash, then digits (e.g., ATLASG-2358)
    - Converted to Markdown links: [ATLASG-2358](https://its.cern.ch/jira/browse/ATLASG-2358)

    Args:
        info: Input string that may contain JIRA tickets.

    Returns:
        The string with JIRA tickets converted to Markdown links.
    """
    if not info:
        return info

    # Regex pattern: one or more uppercase letters, dash, one or more digits
    pattern = r"\b([A-Z]+-\d+)\b"

    def replace_match(match):
        ticket = match.group(1)
        url = f"https://its.cern.ch/jira/browse/{ticket}"
        return f"[{ticket}]({url})"

    return re.sub(pattern, replace_match, info)


def extract_from_classes(
    block_classes: List[Type], output_vars_yaml: Optional[Union[str, List[str]]] = None
) -> List[Dict[str, Any]]:
    """
    Extract options information from a list of ConfigBlock classes and merge
    with output variables metadata.

    Args:
        block_classes: List of classes that inherit from ConfigBlock
        output_vars_yaml: Optional path to YAML file or list of paths to YAML files
                         containing output variables. If multiple files provided,
                         their contents will be merged. Files are located using
                         find_datafile.

    Returns:
        List of dictionaries, each containing information about a block class
    """
    # Load output variables if YAML file(s) provided
    output_vars_map = {}
    if output_vars_yaml:
        # Normalize to list for uniform processing
        yaml_files = (
            [output_vars_yaml]
            if isinstance(output_vars_yaml, str)
            else output_vars_yaml
        )

        # Load and merge all YAML files
        for yaml_file in yaml_files:
            # Locate the file
            resolved_path = find_datafile(yaml_file)
            if resolved_path is None:
                raise FileNotFoundError(f"Could not locate YAML file: {yaml_file}")

            file_vars = load_output_variables(resolved_path)
            # Merge with existing map (later files can override earlier ones)
            for class_name, variables in file_vars.items():
                if class_name in output_vars_map:
                    # Merge variable lists, avoiding duplicates if needed
                    output_vars_map[class_name].extend(variables)
                else:
                    output_vars_map[class_name] = variables

    results = []
    for block_class in block_classes:
        info = extract_block_options(block_class)
        # Merge output variables if available
        class_name = block_class.__name__
        info["output_variables"] = output_vars_map.get(class_name, [])

        results.append(info)

    return results


def save_as_yaml(data: List[Dict[str, Any]], filepath: str) -> None:
    """Save extracted data as YAML."""
    with open(filepath, "w") as f:
        yaml.dump(data, f, default_flow_style=False, sort_keys=False)
    logger.info(f"Saved YAML to {filepath}")


def generate_block_markdown(block_info: Dict[str, Any]) -> str:
    """
    Generate Markdown documentation for a single block.

    Args:
        block_info: Dictionary containing block information with keys:
            - class: Block class name
            - module: Module containing the block
            - options: List of option dictionaries
            - output_variables: List of output variable dictionaries

    Returns:
        Markdown string for this block
    """
    markdown = ""

    # Options section
    if block_info.get("options"):
        for opt in block_info["options"]:
            name = opt["label"]

            # Skip these settings unless they are True
            if name in ["skipOnData", "skipOnMC", "skipWithSystematics"]:
                if not opt["default"] is True:
                    continue
            # Skip these settings unless they are set
            if name in ["onlyForDSIDs"]:
                if not opt["default"] is []:
                    continue

            # Option label with type, expert flag, required flag
            label = f"`{opt['label']}` ({opt['type']})"
            if opt["expertMode"] is not None:
                expertOptions = list(opt["expertMode"])
                label += f" **[expert-only options: {','.join(['`' + str(x) + '`' for x in expertOptions])}]**"
            if opt["required"] is True or opt["noneAction"] != "ignore":
                label += " **[REQUIRED]**"

            markdown += f"{label}\n"
            info_string = opt["info"]
            info_string = process_info_links(info_string)
            info_string = link_jira_tickets(info_string)
            markdown += f":   {info_string}"

            if opt.get("default") != "":
                default_val = opt["default"]
                default_str = repr(default_val)

                # Add unit information if available
                unit = opt.get("physicalUnit")
                if unit is None or default_val is None:
                    default_display = f"`{default_str}`"
                elif unit == "GeV":
                    default_display = f"`{default_str}` GeV"
                elif unit == "MeV":
                    # Convert MeV to GeV for simplified display
                    try:
                        if isinstance(default_val, (list, tuple)):
                            converted = [float(x) / 1000 for x in default_val]
                            converted_str = (
                                "[" + ", ".join(f"{x}" for x in converted) + "]"
                            )
                        else:
                            converted = float(default_val) / 1000
                            converted_str = f"{converted}"
                    except (TypeError, ValueError):
                        converted_str = "?"
                    default_display = f"`{default_str}` MeV (`{converted_str}` GeV)"
                else:
                    default_display = f"`{default_str}` {unit}"

                markdown += f" Default: {default_display}."

            markdown += "\n\n"

    # Output variables section
    if block_info.get("output_variables"):
        # Separate variables into always-saved and toggled
        always_saved = []
        toggled_vars = {}  # toggled_by condition -> list of variables

        for var in block_info["output_variables"]:
            if var.get("toggled_by"):
                condition = var["toggled_by"]
                if condition not in toggled_vars:
                    toggled_vars[condition] = []
                toggled_vars[condition].append(var)
            else:
                always_saved.append(var)

        # Always-saved variables section
        if always_saved:
            markdown += '!!! success "Registers the following variables:"\n'
            for var in always_saved:
                var_name = var.get("name", "N/A")
                var_desc = var.get("description", "")
                markdown += f"    - `{var_name}`: {var_desc}\n"
            markdown += "\n"

        # Toggled variables sections
        for condition, vars_list in toggled_vars.items():
            markdown += (
                f'!!! success "Additional variables toggled by `{condition}`:"\n'
            )
            for var in vars_list:
                var_name = var.get("name", "N/A")
                var_desc = var.get("description", "")
                markdown += f"    - `{var_name}`: {var_desc}\n"
            markdown += "\n"
    else:
        logger.warning(
            f"Block {block_info.get('class')} didn't register any output variables."
        )

    return markdown


def process_markdown_with_autogen(
    input_filepath: str, output_filepath: str, block_data: List[Dict[str, Any]]
) -> None:
    """
    Process an input markdown file, replacing AUTOGEN<BlockName> markers
    with generated block documentation.

    Looks for lines of the form "AUTOGEN<BlockName>" and replaces them
    with the generated markdown for that block. All other content is
    left untouched.

    Args:
        input_filepath: Path to the input markdown file
        output_filepath: Path to write the processed output
        block_data: List of extracted block information dictionaries

    Raises:
        FileNotFoundError: If input file does not exist
        ValueError: If a referenced block is not found in block_data
    """
    # Create a mapping of block names to their markdown
    block_markdown_map = {
        block["class"]: generate_block_markdown(block) for block in block_data
    }

    # Read input file
    with open(input_filepath, "r") as f:
        lines = f.readlines()

    output_lines = []
    for line in lines:
        stripped = line.strip()

        # Check if this line is an AUTOGEN marker
        if stripped.startswith("AUTOGEN<") and stripped.endswith(">"):
            # Extract block name from AUTOGEN<BlockName>
            block_name = stripped[8:-1]  # Remove 'AUTOGEN<' and '>'

            if block_name in block_markdown_map:
                output_lines.append(block_markdown_map[block_name])
            else:
                raise ValueError(
                    f"Block '{block_name}' not found in extracted block data"
                )
        else:
            output_lines.append(line)

    # Write output file
    with open(output_filepath, "w") as f:
        f.writelines(output_lines)

    logger.info(f"Processed markdown saved to {output_filepath}.")
