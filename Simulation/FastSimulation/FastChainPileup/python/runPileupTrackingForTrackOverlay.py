# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Author: William L. (william.axel.leight@cern.ch)
# Author: FY T. (fang-ying.tsai@cern.ch)

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg, MessageSvcCfg
from AthenaConfiguration.Enums import LHCPeriod

flags = initConfigFlags()
flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO_BKG/ATLAS-P2-RUN4-01-01-00/RUN4_presampling.mu200.25events.RDO.pool.root']
flags.Input.isMC = True

import sys
flags.Exec.SkipEvents = int(sys.argv[1])*100
flags.Exec.MaxEvents = 1
#see MainServicesConfig.py
flags.Concurrency.NumThreads = 1

#flags.Output.doWriteBS = True # default False. #  write out RDO ByteStream file
flags.Output.doWriteRDO = True  
if not flags.Output.RDOFileName:
    flags.Output.RDOFileName = 'RDO_test.pool.root'

# InDetConfigFlags.py is based upon InDetFlags. Dropped lots of flags. 
flags.Tracking.doVertexFinding = False # from VertexFindingFlags.py

# Updates are based on LArConfigFlags.py
from LArConfiguration.LArConfigRun3 import LArConfigRun3PileUp
LArConfigRun3PileUp(flags)
if flags.GeoModel.Run==LHCPeriod.Run4:
   flags.LAr.ROD.NumberOfCollisions = 200 

# Updates are based DigitizationConfigFlags.py
flags.Digitization.ReadParametersFromDB = False #MR:https://gitlab.cern.ch/atlas/athena/-/merge_requests/60144
#To set the output stream eventInfoKey to be Bkg_EventInfo, see OutputStreamConfig.py
from AthenaConfiguration.Enums import ProductionStep
flags.Common.ProductionStep = ProductionStep.PileUpPresampling

flags.Overlay.SigPrefix="Bkg_"

flags.lock()
flags.dump()
acc = MainServicesCfg(flags)

from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
acc.merge(IOVDbSvcCfg(flags))

# ----------------------------------------------------------------
# Pool input
# ----------------------------------------------------------------
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

#to get the eGammaTopoCluster collection (copied from SystemRec_config.py)
# Detector flags from DetectorConfigFlags.py
if flags.Detector.EnableCalo:
    from egammaAlgs.egammaTopoClusterCopierConfig import egammaTopoClusterCopierCfg
    acc.merge(egammaTopoClusterCopierCfg(flags))

from InDetConfig.BCM_ZeroSuppressionConfig import BCM_ZeroSuppressionCfg
acc.merge(BCM_ZeroSuppressionCfg(flags))

from CaloRec.CaloBCIDAvgAlgConfig import CaloBCIDAvgAlgCfg
acc.merge(CaloBCIDAvgAlgCfg(flags))
acc.getEventAlgo('CaloBCIDAvgAlg').EventInfoKey="Bkg_EventInfo"

if flags.Detector.GeometryITk:
    itemsToRecord= ['TrackCollection#CombinedITkTracks', 'TrackCollection#ResolvedConversionTracks', 'InDet::PixelClusterContainer#ITkPixelClusters', "InDet::SCT_ClusterContainer#ITkStripClusters"]
else:
    itemsToRecord = ['TrackCollection#CombinedInDetTracks', 'TrackCollection#DisappearingTracks', 'TrackCollection#ResolvedForwardTracks', 'TrackCollection#ExtendedLargeD0Tracks', 'InDet::TRT_DriftCircleContainer#TRT_DriftCircles', "InDet::PixelClusterContainer#PixelClusters", "InDet::SCT_ClusterContainer#SCT_Clusters"]

metadata_items = ["EventStreamInfo#StreamRDO"]
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from AthenaConfiguration.Enums import MetadataCategory
from xAODMetaDataCnv.InfileMetaDataConfig import propagateMetaData

eventStreamInfo, resul_evt = propagateMetaData(flags, "RDO", category=MetadataCategory.EventStreamInfo)
acc.merge(OutputStreamCfg(flags, "RDO", ItemList=itemsToRecord, takeItemsFromInput=True, HelperTools=eventStreamInfo.helperTools, MetadataItemList=metadata_items))
acc.getEventAlgo("EventInfoTagBuilder").EventInfoKey="Bkg_EventInfo" # see OutputStreamConfig.py

# -------------------------------------------------------------
# MessageSvc
# -------------------------------------------------------------
acc.merge(MessageSvcCfg(flags))
acc.getService("MessageSvc").Format = "% F%40W%C%4W%R%e%s%8W%R%T %0W%M"
acc.getService("MessageSvc").enableSuppression = True
acc.run(1)
