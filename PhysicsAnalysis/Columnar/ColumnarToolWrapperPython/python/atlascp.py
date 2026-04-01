# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# @author Giordon Stark

# Syntactic sugar for creating CP tools by class name rather than raw
# type strings.  Module-level __getattr__ dispatches attribute access
# to a constructor factory so that, e.g.:
#
#   from ColumnarToolWrapperPython import atlascp
#   tool = atlascp.MuonEfficiencyScaleFactors('myTool')
#
# is equivalent to:
#
#   Tool("CP::MuonEfficiencyScaleFactors/myTool")

import uuid

import awkward as ak

from ColumnarToolWrapperPython.tool import Tool
from ColumnarToolWrapperPython.config import load_config, merge_tool_config

_class_cache = {}


def _make_tool_class(tool_type_name):
    """Return a named Tool subclass for tool_type_name, creating it if needed.

    Parameters
    ----------
    tool_type_name:
        The C++ class name without namespace, e.g. "MuonEfficiencyScaleFactors".

    Returns
    -------
    type
        A subclass of Tool whose ``__name__`` equals ``tool_type_name``.
        Repeated calls with the same name return the same class object.
    """
    if tool_type_name not in _class_cache:
        _class_cache[tool_type_name] = type(tool_type_name, (Tool,), {})
    return _class_cache[tool_type_name]


def __getattr__(name):
    """Return a constructor for the named CP tool type.

    Called when code accesses ``atlascp.<ToolTypeName>``. Returns a callable
    that creates and initializes a ``Tool`` subclass instance.

    Parameters
    ----------
    name:
        Tool type name without namespace, e.g. "MuonEfficiencyScaleFactors".

    Returns
    -------
    callable
        A constructor with signature
        ``(instance_name=None, /, *, namespace="CP", properties=None, rename_containers=None)``.
        ``instance_name`` is positional-only and optional; all other arguments are keyword-only.
        When omitted, an auto-generated unique name is used.
    """
    def constructor(instance_name=None, /, *, namespace="CP", properties=None, rename_containers=None):
        if instance_name is None:
            instance_name = f"unique{uuid.uuid4().hex[:12]}"
        type_and_name = f"{namespace}::{name}/{instance_name}"
        cls = _make_tool_class(name)
        instance = cls.__new__(cls)
        Tool.__init__(instance, type_and_name, properties=properties, rename_containers=rename_containers)
        return instance
    constructor.__name__ = name
    constructor.__qualname__ = name
    return constructor


class Corrections:
    """Ordered list-like container of configured CP tools.

    Returned by ``atlascp.configure()``. Multiple tools of the same type are
    supported. Iterating yields ``(tool_type_name, Tool)`` pairs in config
    file order.

    Use ``corrections[type_name]`` to get a list of all tools of that type.
    Use ``corrections.apply(events)`` to run all tools and merge outputs.
    """

    def __init__(self, tools):
        self._tools = list(tools)  # list of (tool_type_name, Tool)

    def __getitem__(self, tool_type_name):
        """Return list of tools matching tool_type_name."""
        matches = [tool for name, tool in self._tools if name == tool_type_name]
        if not matches:
            raise KeyError(tool_type_name)
        return matches

    def __contains__(self, tool_type_name):
        return any(name == tool_type_name for name, _ in self._tools)

    def __iter__(self):
        return iter(name for name, _ in self._tools)

    def items(self):
        return iter(self._tools)

    def apply(self, events):
        """Run all tools on events and return merged output columns.

        Parameters
        ----------
        events:
            An ak.Array with fields matching the configured containers.

        Returns
        -------
        ak.Array
            Record array with all output columns from all tools merged.
        """
        outputs = {}
        for _name, tool in self._tools:
            result = tool(events)
            for field in ak.fields(result):
                outputs[field] = result[field]
        return ak.Array(outputs)


def configure(path, *, namespace="CP"):
    """Load a TOML config file and return an initialized Corrections object.

    The ``[global]`` section provides default ``rename_containers`` and
    ``properties``. Tool entries are defined as ``[[tool.TypeName]]`` blocks;
    multiple blocks of the same type create multiple tool instances.

    Parameters
    ----------
    path:
        Path to a TOML configuration file (str or Path).
    namespace:
        C++ namespace for all tools (default ``"CP"``).

    Returns
    -------
    Corrections
    """
    cfg = load_config(path)
    global_cfg = cfg.get("global", {})
    tools = []
    for type_name, entries in cfg.get("tool", {}).items():
        for entry in entries:
            merged = merge_tool_config(global_cfg, entry)
            constructor = __getattr__(type_name)
            tool = constructor(
                namespace=namespace,
                properties=merged["properties"] or None,
                rename_containers=merged["rename_containers"] or None,
            )
            tools.append((type_name, tool))
    return Corrections(tools)
