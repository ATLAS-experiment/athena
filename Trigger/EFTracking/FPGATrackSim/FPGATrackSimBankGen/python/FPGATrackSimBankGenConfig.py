# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
'''
@file FPGATrackSimBankGenConfig.py
@author Riley Xu - rixu@cern.ch
@date Sept 22, 2020
@brief This file declares functions to configure components in FPGATrackSimBankGen
'''

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from FPGATrackSimConfTools.FPGATrackSimAnalysisConfig import FPGATrackSimRoadUnionToolCfg
from AthenaCommon.SystemOfUnits import GeV


def FPGATrackSimSpacePointsToolCfg(flags):
    result=ComponentAccumulator()
    SpacePointTool = CompFactory.FPGATrackSimSpacePointsTool_v2()
    SpacePointTool.Filtering = flags.Trigger.FPGATrackSim.spacePointFiltering
    SpacePointTool.FilteringClosePoints = False
    SpacePointTool.PhiWindow = 0.008
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

    FPGATrackSimSGInputTool = CompFactory.FPGATrackSimSGToRawHitsTool(maxEta=3.2, minPt=0.8 * GeV,
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

    flags = prepareFlagsForFPGATrackSimBankGen(flags)

    acc = ComponentAccumulator()

    theFPGATrackSimMatrixGenAlg = CompFactory.FPGATrackSimMatrixGenAlgo()
    theFPGATrackSimMatrixGenAlg.Clustering = True
    theFPGATrackSimMatrixGenAlg.IdealiseGeometry = 2
    theFPGATrackSimMatrixGenAlg.SingleSector = False
    theFPGATrackSimMatrixGenAlg.HoughConstants = True
    theFPGATrackSimMatrixGenAlg.PT_THRESHOLD = 1.0 # GeV
    theFPGATrackSimMatrixGenAlg.D0_THRESHOLD = 2.0 # mm
    theFPGATrackSimMatrixGenAlg.TRAIN_PDG = 13
    theFPGATrackSimMatrixGenAlg.NBanks = 1

    ### the use of spacepoints hasn't been moved over to rel24, it will be done in the future
    # Require that we have at least 4/5 2D hits (complete spacepoints + pixels).
    # This will *disallow* tracks that are missing a pixel hit and a spacepoint,
    # and also disallow tracks that are missing more than one spacepoint.
    # This can also be achieved by just running a 8/9 threshold.
    #####theFPGATrackSimMatrixGenAlg.minSpacePlusPixel = 4
    ###theFPGATrackSimMatrixGenAlg.doSpacePoints = True
    #theFPGATrackSimMatrixGenAlg.SpacePointTool = acc.getPrimaryAndMerge(FPGATrackSimSpacePointsToolCfg(flags))

    # Override this. It gets set somewhere from bank_tag.
    theFPGATrackSimMatrixGenAlg.WCmax = 2
    theFPGATrackSimMatrixGenAlg.RoadFinder = acc.getPrimaryAndMerge(FPGATrackSimRoadUnionToolCfg(flags))

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    acc.merge(BeamSpotCondAlgCfg(flags))

    theFPGATrackSimMatrixGenAlg.FPGATrackSimSGToRawHitsTool = acc.popToolsAndMerge(FPGATrackSimSGToRawHitsToolCfg(flags))
    theFPGATrackSimMatrixGenAlg.FPGATrackSimClusteringFTKTool = CompFactory.FPGATrackSimClusteringTool()

    acc.addEventAlgo(theFPGATrackSimMatrixGenAlg)

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)

    flags.fillFromArgs()
    flags.lock()

    acc=MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    acc.merge(FPGATrackSimBankGenCfg(flags))
    acc.store(open('FPGATrackSimMapMakerConfig.pkl','wb'))

    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, acc)

    MatrixFileName="matrix.root"
    acc.addService(CompFactory.THistSvc(Output = ["TRIGFPGATrackSimMATRIXOUT DATAFILE='"+MatrixFileName+"', OPT='RECREATE'"]))

    statusCode = acc.run()
    assert statusCode.isSuccess() is True, "Application execution did not succeed"

