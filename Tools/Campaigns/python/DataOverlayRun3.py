# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def DataOverlay2023(flags):
    """Configuration for HI data overlay for year 2023"""
    flags.Beam.NumberOfCollisions = 0.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
    LArConfigRun3PileUp(flags)

    flags.LAr.OFCShapeFolder = "4samples1phase"
    flags.Tile.BestPhaseFromCOOL = False
    flags.Tile.correctTime = False

    flags.Reco.EnableHI = True
    from AthenaConfiguration.Enums import HIMode
    flags.Reco.HIMode = HIMode.HI
