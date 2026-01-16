# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def _DataOverlayRun3Cfg(flags):
    """Common configuration for HI data overlay for Run 3"""
    flags.Beam.NumberOfCollisions = 0.

    from LArConfiguration.LArConfigRun3 import LArConfigRun3NoPileUp
    LArConfigRun3NoPileUp(flags)

    flags.Reco.EnableHI = True
    from AthenaConfiguration.Enums import HIMode
    flags.Reco.HIMode = HIMode.HI 


def DataOverlay2023(flags):
    """Configuration for HI data overlay for year 2023"""
    _DataOverlayRun3Cfg(flags)

    flags.Overlay.DataOverlayConditions = "OverlayConfiguration.DataOverlayConditions.DataOverlay2023Cfg"


def DataOverlay2024(flags):
    """Configuration for HI data overlay for year 2024"""
    _DataOverlayRun3Cfg(flags)

    flags.Overlay.DataOverlayConditions = "OverlayConfiguration.DataOverlayConditions.DataOverlay2024Cfg"


def DataOverlay2025OO(flags):
    """Configuration for OO data overlay for year 2025"""
    _DataOverlayRun3Cfg(flags)

    from AthenaConfiguration.Enums import HIMode
    flags.Reco.HIMode = HIMode.HIP

    flags.Overlay.DataOverlayConditions = "OverlayConfiguration.DataOverlayConditions.DataOverlay2025OOCfg"
