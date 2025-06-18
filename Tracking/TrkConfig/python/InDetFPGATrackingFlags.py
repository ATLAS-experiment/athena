#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
from TrkConfig.TrackingPassFlags import createITkTrackingPassFlags
from TrkConfig.TrkConfigFlags import TrackingComponent


def createFPGATrackingPassFlags():
    icf = createITkTrackingPassFlags()
    icf.extension = "Acts"
    icf.doAthenaCluster = False
    icf.doAthenaSpacePoint = False
    icf.doAthenaSeed = False
    icf.doAthenaTrack = False
    icf.doAthenaAmbiguityResolution = False

    icf.doActsCluster = True
    icf.doActsSpacePoint = False
    icf.doFPGASpacePoint = True
    icf.doActsSeed = True
    icf.doActsTrack = True

    icf.doFPGACluster = True

    icf.doActsAmbiguityResolution = lambda pcf: pcf.Acts.doAmbiguityResolution
    return icf


def fpgaPassThroughValidation(flags) -> None:
    """flags for Reco_tf with CA used in CI tests: use FPGAChain during reconstruction"""
    flags.Reco.EnableHGTDExtension = False
    flags.Tracking.recoChain = [TrackingComponent.FPGAChain]