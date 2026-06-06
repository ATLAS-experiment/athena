# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Public PowhegControl package interface.

The legacy control class is loaded lazily so importing CA configuration data
does not import the stateful Powheg implementation.
"""

__all__ = ["PowhegControl"]


def __getattr__(name):
    if name == "PowhegControl":
        from .powheg_control import PowhegControl
        return PowhegControl
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")
