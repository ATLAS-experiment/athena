# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# TOML-based configuration loading for ColumnarToolWrapperPython.
# Supports global defaults and per-tool-type overrides for rename_containers
# and properties. Multiple instances of the same tool type are supported via
# TOML array-of-tables: [[tool.TypeName]].

from pathlib import Path

try:
    import tomllib
except ImportError:
    try:
        import tomli as tomllib
    except ImportError as exc:
        raise ImportError(
            "TOML support requires Python >= 3.11 or 'tomli' package. "
            "Install with: pip install tomli"
        ) from exc


def load_config(path):
    """Load a TOML configuration file.

    Parameters
    ----------
    path:
        Path to a TOML file (str or Path).

    Returns
    -------
    dict
        Raw parsed TOML dict. Tool entries are under ``cfg["tool"][TypeName]``
        as a list (one entry per ``[[tool.TypeName]]`` block).
    """
    with Path(path).open("rb") as f:
        return tomllib.load(f)


def merge_tool_config(global_cfg, tool_entry):
    """Merge global config with a single tool entry's overrides.

    Parameters
    ----------
    global_cfg:
        The ``cfg["global"]`` dict (may be empty).
    tool_entry:
        A single entry from ``cfg["tool"][TypeName]`` (one ``[[tool.TypeName]]``
        block). May be empty or contain ``rename_containers`` / ``properties``.

    Returns
    -------
    dict
        Dict with keys ``"rename_containers"`` and ``"properties"``, each a
        dict. Tool-level values override global values.
    """
    rename_containers = {**global_cfg.get("rename_containers", {}),
                         **tool_entry.get("rename_containers", {})}
    properties = {**global_cfg.get("properties", {}),
                  **tool_entry.get("properties", {})}

    return {"rename_containers": rename_containers, "properties": properties}
