# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

try:
    import cppjit
except ModuleNotFoundError:
    cppjit = None

def loadEnumHeader (h):
    """Load a header file defining an enum into cppjit."""
    if cppjit is not None:
        cppjit.include (h)
    return
