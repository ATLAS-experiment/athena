# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createUFOConfigFlags():
    flags = AthConfigFlags()

    flags.addFlag("UFO.UseCov", True)
    flags.addFlag("UFO.dR", 0.05)
    # Use the ACTS calo extension (instead of the Trk one) to obtain the track calo entry point
    flags.addFlag("UFO.useActsExtrapolation", False)
    return flags
