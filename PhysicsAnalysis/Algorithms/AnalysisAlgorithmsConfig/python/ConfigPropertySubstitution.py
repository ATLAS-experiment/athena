# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Property-value substitution across configurables
#
# This file provides helpers that walk a configurable component's
# user-set properties and rewrite string references inside them,
# recursing into private tools, tool-handle arrays, data handles, and
# nested containers. Used by `ConfigAccumulator.renameFinalContainers`
# to strip the auto-generated `_STEP<n>` suffix from each container's
# final name everywhere it appears in the configuration.
#
# Works with both EventLoop (`PythonConfig` / `PrivateToolConfig`) and
# Athena (`GaudiConfig2.Configurable`, `PrivateToolHandleArray`,
# `DataHandle`).


import collections.abc
import re


# Private-tool leaf types — resolved once at import time. Either flavor
# may be absent in a given build:
#   * `AnaAlgorithm.PythonConfig` is the EventLoop side; absent in Athena.
#   * `GaudiConfig2` is the Athena side; absent in AnalysisBase.
try :
    from AnaAlgorithm.PythonConfig import PrivateToolConfig as _ELPrivateToolConfig
except ImportError :
    _ELPrivateToolConfig = None

try :
    from GaudiConfig2 import Configurable as _CAConfigurable
except ImportError :
    _CAConfigurable = None


def _anchoredReplace (text, search, replace) :
    """substring-replace `search` with `replace` in `text`, anchored so the
    match must start at the beginning of the string or immediately after a
    non-identifier character (anything not in [A-Za-z0-9_]).

    This avoids accidentally rewriting e.g. `MyJets_STEP3` when replacing
    `Jets_STEP3`.
    """
    pattern = re.compile (r'(?:^|(?<=[^A-Za-z0-9_]))' + re.escape (search))
    return pattern.sub (replace, text)


def substituteValue (value, substitutions) :
    """recursively rewrite container references inside a property value

    Returns the (possibly new) value; for private-tool values (EventLoop
    `PrivateToolConfig`, Athena `GaudiConfig2.Configurable`) and for
    `PrivateToolHandleArray` the tool(s) are mutated in place and the
    same instance is returned.
    """
    if isinstance (value, str) :
        new = value
        for search, replace in substitutions :
            new = _anchoredReplace (new, search, replace)
        return new
    # EventLoop private tool
    if _ELPrivateToolConfig is not None and isinstance (value, _ELPrivateToolConfig) :
        substituteComponentProperties (value, substitutions)
        return value
    # Athena private tool held via single `ToolHandle`
    if _CAConfigurable is not None and isinstance (value, _CAConfigurable) :
        substituteComponentProperties (value, substitutions)
        return value
    # Athena `ToolHandleArray` (private tools). Detect by class name to
    # avoid importing GaudiHandles; this is the pattern used elsewhere
    # in the codebase (e.g. HypoToolAnalyser).
    if value.__class__.__name__ == 'PrivateToolHandleArray' :
        for tool in value :
            substituteComponentProperties (tool, substitutions)
        return value
    # Athena `DataHandle` (read/write event-store handle). The container
    # name lives in `.Path`. Returning the substituted string lets the
    # property setter reconstruct the handle via its semantics.
    if value.__class__.__name__ == 'DataHandle' :
        newPath = substituteValue (value.Path, substitutions)
        if newPath == value.Path :
            return value
        return newPath
    # Use abstract base classes rather than `list`/`set`/`dict` so we
    # also catch Athena GaudiConfig2 container wrappers (`_ListHelper`,
    # `_SetHelper`, `_DictHelper`), which are MutableSequence/Set/Mapping
    # but not the builtin types.
    if isinstance (value, collections.abc.MutableMapping) :
        return {substituteValue (k, substitutions) : substituteValue (v, substitutions)
                for k, v in value.items()}
    if isinstance (value, collections.abc.MutableSet) :
        return {substituteValue (v, substitutions) for v in value}
    if isinstance (value, collections.abc.MutableSequence) :
        return [substituteValue (v, substitutions) for v in value]
    if isinstance (value, tuple) :
        return tuple (substituteValue (v, substitutions) for v in value)
    return value


def substituteComponentProperties (component, substitutions) :
    """walk a component's user-set properties, substituting container
    references in every value, and write the result back through
    `setattr` so the underlying configuration stays in sync."""
    if hasattr (component, '_props') :
        propDict = component._props
    elif hasattr (component, '_properties') :
        propDict = component._properties
    else :
        return
    for propName in list (propDict.keys()) :
        old = propDict[propName]
        new = substituteValue (old, substitutions)
        # Private tools and tool arrays are mutated in place — same
        # identity, skip the write-back; otherwise re-assign only if the
        # value actually changed.
        if new is old :
            continue
        if new != old :
            setattr (component, propName, new)
