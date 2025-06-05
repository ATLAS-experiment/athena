"""Construct Overlay configuration flags

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.AutoConfigFlags import GetFileMD


def createOverlayConfigFlags():
    """Return an AthConfigFlags object with required flags"""
    flags = AthConfigFlags()
    # Data overlay flag
    flags.addFlag("Overlay.DataOverlay", lambda prevFlags : GetFileMD(prevFlags.Input.Files).get("IsDataOverlay", "False") == "True")
    # Overlay skip secondary events
    flags.addFlag("Overlay.SkipSecondaryEvents", -1)
    # Overlay flag when reading from ByteStream
    flags.addFlag("Overlay.ByteStream", False)
    # Overlay background StoreGate key prefix
    flags.addFlag("Overlay.BkgPrefix", "Bkg_")
    # Overlay signal StoreGate key prefix
    flags.addFlag("Overlay.SigPrefix", "Sig_")
    # Overlay extra input dependencies
    flags.addFlag("Overlay.ExtraInputs", [("McEventCollection", "TruthEvent")])
    # track overlay flag
    flags.addFlag("Overlay.doTrackOverlay", False)
    return flags
