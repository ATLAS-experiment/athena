# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
'''
@file FPGATrackSimBankGenConfig.py
@author Riley Xu - rixu@cern.ch
@date Sept 22, 2020
@brief This file declares functions to configure components in FPGATrackSimBankGen
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import FPGATrackSimRoadUnionToolCfg,FPGATrackSimRoadUnionTool1DCfg,FPGATrackSimRoadUnionToolGenScanCfg
from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import FPGATrackSimRawLogicCfg, FPGATrackSimMappingCfg, FPGATrackSimEventSelectionSvcCfg
from AthenaCommon.SystemOfUnits import GeV


def FPGATrackSimSpacePointsToolCfg(flags):
    result=ComponentAccumulator()
    SpacePointTool = CompFactory.FPGATrackSimSpacePointsTool()
    SpacePointTool.Filtering = flags.Trigger.FPGATrackSim.ActiveConfig.spacePointFiltering
    SpacePointTool.FilteringClosePoints = False
    SpacePointTool.PhiWindow = 0.004
    SpacePointTool.Duplication = True
    result.setPrivateTools(SpacePointTool)
    return result

def prepareFlagsForFPGATrackSimBankGen(flags):
    newFlags = flags.cloneAndReplace("Trigger.FPGATrackSim.ActiveConfig", "Trigger.FPGATrackSim." + flags.Trigger.FPGATrackSim.algoTag)
    return newFlags

def FPGATrackSimSGToRawHitsToolCfg(flags):
    result=ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    MyExtrapolator = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))

    from TrkConfig.TrkTruthCreatorToolsConfig import TruthToTrackToolCfg
    MyTruthToTrack = result.popToolsAndMerge(TruthToTrackToolCfg(flags))

    FPGATrackSimSGInputTool = CompFactory.FPGATrackSimSGToRawHitsTool(maxEta=5.0, minPt=0.8 * GeV,
        dumpHitsOnTracks=False,
        dumpTruthIntersections=False,
        ReadOfflineClusters=False,
        ReadTruthTracks=True,
        ReadOfflineTracks=False,
        UseNominalOrigin = True,
        Extrapolator = MyExtrapolator,
        TruthToTrackTool = MyTruthToTrack )
    result.setPrivateTools(FPGATrackSimSGInputTool)
    return result

def FPGATrackSimBankGenCfg(flags, **kwargs):

    acc = ComponentAccumulator()

    theFPGATrackSimMatrixGenAlg = CompFactory.FPGATrackSimMatrixGenAlgo()
    theFPGATrackSimMatrixGenAlg.Clustering = True
    theFPGATrackSimMatrixGenAlg.IdealiseGeometry = 2
    theFPGATrackSimMatrixGenAlg.SingleSector = False
    theFPGATrackSimMatrixGenAlg.HoughConstants = True
    theFPGATrackSimMatrixGenAlg.DeltaPhiConstants = False
    theFPGATrackSimMatrixGenAlg.PT_THRESHOLD = 1.0 # GeV
    theFPGATrackSimMatrixGenAlg.D0_THRESHOLD = 2.0 # mm
    theFPGATrackSimMatrixGenAlg.TRAIN_PDG = 13
    theFPGATrackSimMatrixGenAlg.NBanks = 1

    theFPGATrackSimMatrixGenAlg.SpacePoints = flags.Trigger.FPGATrackSim.spacePoints
    if flags.Trigger.FPGATrackSim.spacePoints:
        theFPGATrackSimMatrixGenAlg.SpacePointTool = acc.getPrimaryAndMerge(FPGATrackSimSpacePointsToolCfg(flags))
    theFPGATrackSimMatrixGenAlg.minSpacePlusPixel = flags.Trigger.FPGATrackSim.minSpacePlusPixel

    theFPGATrackSimMatrixGenAlg.FPGATrackSimEventSelectionSvc = acc.getPrimaryAndMerge(FPGATrackSimEventSelectionSvcCfg(flags))
    theFPGATrackSimMatrixGenAlg.FPGATrackSimMappingSvc = acc.getPrimaryAndMerge(FPGATrackSimMappingCfg(flags))

    if (flags.Trigger.FPGATrackSim.ActiveConfig.secondStage):
        from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import FPGATrackSimTrackFitterToolCfg,FPGATrackSimOverlapRemovalToolCfg
        from FPGATrackSimConfTools.FPGATrackSimSecondStageConfig import FPGATrackSimWindowExtensionToolCfg
        theFPGATrackSimMatrixGenAlg.TrackFitter_1st = acc.getPrimaryAndMerge(FPGATrackSimTrackFitterToolCfg(flags))
        theFPGATrackSimMatrixGenAlg.OverlapRemoval_1st = acc.getPrimaryAndMerge(FPGATrackSimOverlapRemovalToolCfg(flags))
        theFPGATrackSimMatrixGenAlg.TrackExtensionTool = acc.getPrimaryAndMerge(FPGATrackSimWindowExtensionToolCfg(flags))
        theFPGATrackSimMatrixGenAlg.SecondStage = True
    else:
        theFPGATrackSimMatrixGenAlg.SecondStage = False

    
    # Override this. It gets set somewhere from bank_tag.
    theFPGATrackSimMatrixGenAlg.WCmax = 2
    theFPGATrackSimMatrixGenAlg.dropHitsAndFill = flags.dropHitsAndFill
    
    theFPGATrackSimMatrixGenAlg.FPGATrackSimRawToLogicalHitsTool = acc.getPrimaryAndMerge(FPGATrackSimRawLogicCfg(flags))
    if (flags.Trigger.FPGATrackSim.ActiveConfig.genScan):
        theFPGATrackSimMatrixGenAlg.RoadFinder = acc.getPrimaryAndMerge(FPGATrackSimRoadUnionToolGenScanCfg(flags))
    elif (flags.Trigger.FPGATrackSim.ActiveConfig.hough1D):
        theFPGATrackSimMatrixGenAlg.RoadFinder = acc.getPrimaryAndMerge(FPGATrackSimRoadUnionTool1DCfg(flags))
    else:
      theFPGATrackSimMatrixGenAlg.RoadFinder = acc.getPrimaryAndMerge(FPGATrackSimRoadUnionToolCfg(flags))

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    theFPGATrackSimMatrixGenAlg.FPGATrackSimSGToRawHitsTool = acc.popToolsAndMerge(FPGATrackSimSGToRawHitsToolCfg(flags))
    theFPGATrackSimMatrixGenAlg.FPGATrackSimClusteringFTKTool = CompFactory.FPGATrackSimClusteringTool()

    theFPGATrackSimMatrixGenAlg.sectorQPtBins = [-0.001, -0.0005, 0, 0.0005, 0.001] ### hard-code this for now
    theFPGATrackSimMatrixGenAlg.qptAbsBinning = False

    acc.addEventAlgo(theFPGATrackSimMatrixGenAlg)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.addFlag('dropHitsAndFill', False)

    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags.fillFromArgs()
    flags.Trigger.FPGATrackSim.Hough.IdealGeoRoads = False 
    flags = prepareFlagsForFPGATrackSimBankGen(flags)

    ### we don't want to load sectors when running bank gen, set this to false

    flags.lock()

    acc=MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    acc.merge(FPGATrackSimBankGenCfg(flags))
    acc.store(open('FPGATrackSimMapMakerConfig.pkl','wb'))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    acc.foreach_component("FPGATrackSim*").OutputLevel=flags.Trigger.FPGATrackSim.loglevel
    if flags.Trigger.FPGATrackSim.msgLimit!=-1:
        acc.getService("MessageSvc").debugLimit = flags.Trigger.FPGATrackSim.msgLimit
        acc.getService("MessageSvc").infoLimit = flags.Trigger.FPGATrackSim.msgLimit

    MatrixFileName="matrix.root"
    acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimMATRIXOUT DATAFILE='"+MatrixFileName+"', OPT='RECREATE'"]))

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

