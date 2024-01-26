# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Author: William L. (william.axel.leight@cern.ch)
# Author: FY T. (fang-ying.tsai@cern.ch)

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg, MessageSvcCfg
from AthenaConfiguration.Enums import Format, BeamType
from TileConfiguration.TileConfigFlags import TileRunType

flags = initConfigFlags()
flags.Input.Files = ['../RDO.31293587._000001.pool.root.1']
flags.Input.isMC = True

import sys
flags.Exec.SkipEvents = int(sys.argv[1])*100
flags.Exec.MaxEvents = 100
#see MainServicesConfig.py
flags.Concurrency.NumThreads = 1
flags.Input.Format = Format.POOL

# Set final Cond/Geo tag based on input file
flags.GeoModel.AtlasVersion='ATLAS-R3S-2021-02-00-00'
flags.IOVDb.GlobalTag = 'OFLCOND-MC21-SDR-RUN3-07'

flags.Beam.Type = BeamType.Collisions
flags.Beam.BunchSpacing = 25
flags.Beam.NumberOfCollisions = 60

flags.Common.isOnline = False

#flags.Output.doWriteBS = True # default False. #  write out RDO ByteStream file
flags.Output.doWriteRDO = True  
if not flags.Output.RDOFileName:
    flags.Output.RDOFileName = 'RDO_test.pool.root'
flags.Detector.GeometryID = True
flags.Detector.GeometryCalo = True
flags.Detector.GeometryForward = False
flags.Detector.EnableCalo = True

# InDetConfigFlags.py is based upon InDetFlags. Dropped lots of flags. 
flags.Tracking.doCaloSeededAmbi = False
flags.Tracking.doTIDE_Ambi = True
flags.Tracking.doCaloSeededBrem = True
flags.Tracking.doBackTracking = True
flags.Tracking.doTruth = True #Turn running of truth matching on and off (by default on for MC off for data)
flags.Tracking.doVertexFinding = False # from VertexFindingFlags.py

# Updated for Monitoring from DQConfigFlags.py
# Missing? doMonitoringPrimaryVertexingEnhanced
flags.DQ.Steering.InDet.doGlobalMon = False
flags.DQ.Steering.doPixelMon = False
flags.DQ.Steering.doSCTMon = False
flags.DQ.Steering.doTRTMon = False
flags.DQ.Steering.InDet.doAlignMon = False

##Updates are based on LArConfigFlags.py. This stuff is in the LArConfigRun3PileUp, see LArConfigRun3.py
flags.LAr.ROD.UseHighestGainAutoCorr = True # default is False, but if setting up LArConfigRun3PileUp it must be True
flags.LAr.ROD.FirstSample = 0 # default
flags.LAr.ROD.nSamples = 4 # default is 5
flags.LAr.ROD.NumberOfCollisions = 60 # default is 0, but Run-3 default is 60

# Updates are based DigitizationConfigFlags.py
flags.Digitization.HighGainEMECIW = False #default is True, but the LArConfigRun3.py sets this to False
flags.Digitization.HighGainFCal = False # default is False
flags.Digitization.ReadParametersFromDB = False
# Updates are  based on TileConfigFlags.py
flags.Tile.RunType = TileRunType.PHY # physics run type. # Tile run types: UNDEFINED, PHY, PED, LAS, BILAS, CIS, MONOCIS
flags.Tile.doOpt2 = False  # disable optimal filter with iterations
flags.Tile.doOptATLAS = True # run optimal filter without iterations
flags.Tile.correctAmplitude = True # apply parabolic correction
flags.Tile.correctTime = False # don't need time correction if best phase is used
flags.Tile.BestPhaseFromCOOL = True # use best phase stored in DB
#To set the output stream eventInfoKey to be Bkg_EventInfo, see OutputStreamConfig.py
from AthenaConfiguration.Enums import ProductionStep
flags.Common.ProductionStep = ProductionStep.PileUpPresampling


flags.lock()
flags.dump()
acc = MainServicesCfg(flags)

from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
acc.merge(IOVDbSvcCfg(flags))

# ----------------------------------------------------------------
# Pool input
# ----------------------------------------------------------------
print("flags.Input.Format", flags.Input.Format)
print("flags.Trigger.Online.isPartition", flags.Trigger.Online.isPartition)

# Load input collection list from POOL metadata
from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
acc.merge(SGInputLoaderCfg( flags, Load=[( 'xAOD::EventInfo' , 'StoreGateSvc+Bkg_EventInfo' )]))

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge(PoolReadCfg(flags))
#set up detector stuff
from CaloRec.CaloRecoConfig import CaloRecoCfg
acc.merge(CaloRecoCfg(flags))
#from CaloRecoConfig.py
acc.getEventAlgo('CaloCellMaker').CaloCellMakerToolNames[1].EventInfo="Bkg_EventInfo"
from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
acc.merge(TileRawChannelMakerCfg(flags))
acc.getEventAlgo('TileRChMaker').TileDigitsContainer="Bkg_TileDigitsCnt"
from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterCfg
acc.merge(TileDigitsFilterCfg(flags))
acc.getEventAlgo('TileDigitsFilter').InputDigitsContainer="Bkg_TileDigitsCnt"
from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
acc.merge(LArRawChannelBuilderAlgCfg(flags,  LArDigitKey = "Bkg_LArDigitContainer_MC"))
from LArCellRec.LArNoisyROSummaryConfig import LArNoisyROSummaryCfg
acc.merge(LArNoisyROSummaryCfg(flags))
acc.getEventAlgo('LArNoisyROAlg').EventInfoKey="Bkg_EventInfo"
#RuntimeError: Attempt to modify locked flag container
# from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
# acc.merge(LArConfigRun3PileUp(flags))

#to get the eGammaTopoCluster collection (copied from SystemRec_config.py)
# Detector flags from DetectorConfigFlags.py
if flags.Detector.EnableCalo:
    from egammaAlgs.egammaTopoClusterCopierConfig import egammaTopoClusterCopierCfg
    acc.merge(egammaTopoClusterCopierCfg(flags))
# Reco flags from RecoConfigFlags.py
if flags.Reco.EnableTracking:
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(flags))
from InDetConfig.InDetPrepRawDataFormationConfig import PixelClusterizationCfg, SCTClusterizationCfg, InDetTRT_RIO_MakerCfg

acc.merge(PixelClusterizationCfg(flags, DataObjectName="Bkg_PixelRDOs"))
acc.merge(SCTClusterizationCfg(flags, DataObjectName="Bkg_SCT_RDOs"))
acc.merge(InDetTRT_RIO_MakerCfg(flags, TRTRDOLocation="Bkg_TRT_RDOs"))
from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
acc.merge(LuminosityCondAlgCfg(flags))
acc.getCondAlgo('LuminosityCondAlg').EventInfoKey="Bkg_EventInfo"
acc.getCondAlgo('LuminosityCondAlg').actualMuKey="Bkg_EventInfo.actualInteractionsPerCrossing"
acc.getCondAlgo('LuminosityCondAlg').averageMuKey="Bkg_EventInfo.averageInteractionsPerCrossing"

# See TRTStandaloneConfig.py
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.EventInfoKey="Bkg_EventInfo"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.actualInteractionsPerCrossingKey="Bkg_EventInfo.actualInteractionsPerCrossing"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.averageInteractionsPerCrossingKey="Bkg_EventInfo.averageInteractionsPerCrossing"
acc.getEventAlgo('InDetSiSpTrackFinder').EventInfoKey="Bkg_EventInfo"
acc.getEventAlgo('InDetSiSpTrackFinderR3LargeD0').EventInfoKey="Bkg_EventInfo"
acc.getEventAlgo('InDetSiSpTrackFinderForward').EventInfoKey="Bkg_EventInfo"
acc.getEventAlgo('InDetSiSpTrackFinderDisappearing').EventInfoKey="Bkg_EventInfo"
#Set the PRD_MultiTruthMaker SDO map name
acc.getEventAlgo("InDetTRT_PRD_MultiTruthMaker").SimDataMapNameTRT="Bkg_TRT_SDO_Map"
from InDetConfig.InDetTruthAlgsConfig import InDetPRD_MultiTruthMakerSiCfg
acc.merge(InDetPRD_MultiTruthMakerSiCfg(flags))
acc.getEventAlgo("InDetPRD_MultiTruthMakerSi").SimDataMapNamePixel="Bkg_PixelSDO_Map"
acc.getEventAlgo("InDetPRD_MultiTruthMakerSi").SimDataMapNameSCT="Bkg_SCT_SDO_Map"

from CaloRec.CaloBCIDAvgAlgConfig import CaloBCIDAvgAlgCfg
acc.merge(CaloBCIDAvgAlgCfg(flags))
acc.getEventAlgo('CaloBCIDAvgAlg').EventInfoKey="Bkg_EventInfo"
from InDetConfig.BCM_ZeroSuppressionConfig import BCM_ZeroSuppressionCfg
acc.merge(BCM_ZeroSuppressionCfg(flags))
acc.getEventAlgo('InDetBCM_ZeroSuppression').BcmContainerName="Bkg_BCM_RDOs"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.EventInfoKey="Bkg_EventInfo"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.actualInteractionsPerCrossingKey="Bkg_EventInfo.actualInteractionsPerCrossing"
acc.getPublicTool('InDetTRT_StandaloneScoringTool').LuminosityTool.averageInteractionsPerCrossingKey="Bkg_EventInfo.averageInteractionsPerCrossing"

itemsToRecord = ['TrackCollection#CombinedInDetTracks', 'TrackCollection#DisappearingTracks', 'TrackCollection#ResolvedForwardTracks', 'TrackCollection#ExtendedLargeD0Tracks', 'InDet::TRT_DriftCircleContainer#TRT_DriftCircles', "InDet::PixelClusterContainer#PixelClusters", "InDet::SCT_ClusterContainer#SCT_Clusters"]

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge(OutputStreamCfg(flags,"RDO", ItemList=itemsToRecord, takeItemsFromInput=True))
acc.getEventAlgo("EventInfoTagBuilder").EventInfoKey="Bkg_EventInfo" # see OutputStreamConfig.py

# -------------------------------------------------------------
# MessageSvc
# -------------------------------------------------------------
acc.merge(MessageSvcCfg(flags))
acc.getService("MessageSvc").Format = "% F%40W%C%4W%R%e%s%8W%R%T %0W%M"
acc.getService("MessageSvc").enableSuppression = True
acc.run(2)

