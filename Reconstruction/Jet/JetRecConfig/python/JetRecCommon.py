# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def isMC(flags):
    """A simple filter function for  testing if we're running in MC
    returns (bool, str) where the str contains an explanation of why the bool is False.
    (probably worth re-allocating somehere else)"""
    from AthenaConfiguration.Enums import Format # Test & exclude reading MC BS, no truth info
    return (flags.Input.isMC or flags.Overlay.DataOverlay) and flags.Input.Format!=Format.BS, "Input file is not MC"
