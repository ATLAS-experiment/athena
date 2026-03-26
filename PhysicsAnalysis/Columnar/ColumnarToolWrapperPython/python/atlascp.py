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

from ColumnarToolWrapperPython.tool import Tool

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
        ``(instance_name, /, *, namespace="CP", properties=None, rename_containers=None)``.
        ``instance_name`` is positional-only; all other arguments are keyword-only.
    """
    def constructor(instance_name, /, *, namespace="CP", properties=None, rename_containers=None):
        type_and_name = f"{namespace}::{name}/{instance_name}"
        cls = _make_tool_class(name)
        instance = cls.__new__(cls)
        Tool.__init__(instance, type_and_name, properties=properties, rename_containers=rename_containers)
        return instance
    constructor.__name__ = name
    constructor.__qualname__ = name
    return constructor
