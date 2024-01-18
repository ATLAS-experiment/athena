# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import AthenaCommon.SystemOfUnits as Units
from AthenaConfiguration.Enums import FlagEnum
from TrkConfig.TrkConfigFlags import PrimaryPassConfig


class VertexSortingSetup(FlagEnum):
    SumPt2Sorting = 'SumPt2Sorting'
    SumPtSorting = 'SumPtSorting'


class VertexSetup(FlagEnum):
    IVF = 'IterativeFinding'
    FastIVF = 'FastIterativeFinding'
    ActsGaussAMVF = 'ActsGaussAdaptiveMultiFinding'
    # Experimental setups, not to be used in production
    ExperimentalActsIVF = 'ExperimentalActsIterativeFinding'


def createPriVertexingFlags():
    from AthenaConfiguration.AthConfigFlags import AthConfigFlags
    flags = AthConfigFlags()

    flags.addFlag("maxAbsEta", 9999.0)
    flags.addFlag("minNTrtHits", 0)
    flags.addFlag("maxNPixelHoles", 1)
    flags.addFlag("maxZ0", 1000.0 * Units.mm)
    flags.addFlag("maxZ0SinTheta", 1000.0 * Units.mm)
    flags.addFlag("minNInnermostLayerHits", 0)
    # MaxTracks cuts are specific to the IterativeFinding config
    flags.addFlag("doMaxTracksCut", lambda pcf:
                  not(pcf.Tracking.PriVertex.useBeamConstraint or
                      pcf.Tracking.PrimaryPassConfig is (
                          PrimaryPassConfig.HeavyIon)))    
    flags.addFlag("maxTracks", 3000)
    flags.addFlag("maxVertices", lambda pcf:
                  1 if pcf.Tracking.PrimaryPassConfig is (
                      PrimaryPassConfig.HeavyIon)
                  else 200)

    # string to store the setup for primary vertexing.

    def vertexSetup(pcf):
        if pcf.Reco.EnableHI:
            return VertexSetup.FastIVF
        elif (pcf.Tracking.doMinBias or
              pcf.Tracking.doLowMu or
              pcf.Tracking.PrimaryPassConfig in [
                  PrimaryPassConfig.VtxLumi,
                  PrimaryPassConfig.VtxBeamSpot,
                  PrimaryPassConfig.HighPileup,
                  PrimaryPassConfig.RobustReco]):
            return VertexSetup.IVF
        else: # Default
            return VertexSetup.ActsGaussAMVF

    flags.addFlag("setup", vertexSetup, enum=VertexSetup)

    # string to store the type of sorting algorithm to separate signal and pile-up vertices.
    flags.addFlag("sortingSetup", VertexSortingSetup.SumPt2Sorting, enum=VertexSortingSetup)
    flags.addFlag("useBeamConstraint", lambda pcf:
                  not(pcf.Tracking.PrimaryPassConfig in [
                      PrimaryPassConfig.VtxLumi,
                      PrimaryPassConfig.VtxBeamSpot,
                      PrimaryPassConfig.RobustReco]))

    def maxD0(pcf):
        if pcf.Detector.GeometryITk:
            return 1.0 * Units.mm
        else:
            if not pcf.Tracking.PriVertex.useBeamConstraint:
                return 10.0 * Units.mm
            else:  # Default ID
                return 4.0 * Units.mm

    flags.addFlag("maxD0", maxD0)

    def minNPixelHits(pcf):
        if pcf.Detector.GeometryITk:
            return 3
        else:
            if pcf.Tracking.PrimaryPassConfig is PrimaryPassConfig.RobustReco:
                return 0
            else: # Default ID
                return 1

    flags.addFlag("minNPixelHits", minNPixelHits)

    def minPt(pcf):
        if pcf.Detector.GeometryITk:
            return 900.0 * Units.MeV
        else:
            if pcf.Tracking.doMinBias or pcf.Tracking.doLowPt:
                return 100.0 * Units.MeV
            elif pcf.Reco.EnableHI or pcf.Tracking.doLowMu:
                return 400.0 * Units.MeV
            else: # Default ID
                return 500.0 * Units.MeV

    flags.addFlag("minPt", minPt)

    def maxSigmaD0(pcf):
        if pcf.Detector.GeometryITk:
            return 0.35 * Units.mm
        else:
            if pcf.Tracking.doLowPt:
                return 0.9 * Units.mm
            else: # Default ID
                return 5.0 * Units.mm

    flags.addFlag("maxSigmaD0", maxSigmaD0)

    idflags = { "maxSigmaZ0SinTheta" : 10.0 * Units.mm,
                "minNSctHits"        : 4,
                "minNSiHits"         : 6,
                "maxZinterval"       : 3}

    itkflags = {"maxSigmaZ0SinTheta" : 2.5 * Units.mm,
                "minNSctHits"        : 0,
                "minNSiHits"         : 7,
                "maxZinterval"       : 0.5}

    for k in idflags:
        # Need to use default arguments in lambda function to keep them set
        # despite the loop
        flags.addFlag(k, lambda pcf, a=idflags[k], b=itkflags[k]:
                      a if pcf.Detector.GeometryID else b)

    return flags
