# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Author: William L. (william.axel.leight@cern.ch)
# Author: FY T. (fang-ying.tsai@cern.ch)

from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def PreTrackingCfg(flags):
   acc = MainServicesCfg(flags)
   from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
   acc.merge(IOVDbSvcCfg(flags))

   # ----------------------------------------------------------------
   # Pool input
   # ----------------------------------------------------------------
   # Load input collection list from POOL metadata
   from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
   acc.merge(SGInputLoaderCfg(flags, Load=[('xAOD::EventInfo', f'StoreGateSvc+{flags.Overlay.BkgPrefix}EventInfo')]))

   from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
   acc.merge(PoolReadCfg(flags))
   if flags.Detector.EnableCalo:
      #In the CaloRecoCfg, TileRawChannelMakerCfg and LArRawChannelBuilderAlgCfg are merged only if the input format is BS
      from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
      acc.merge(TileRawChannelMakerCfg(flags))
      from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
      acc.merge(LArRawChannelBuilderAlgCfg(flags))
      from CaloRec.CaloRecoConfig import CaloRecoCfg
      acc.merge(CaloRecoCfg(flags))

   if flags.Detector.GeometryITk:
       from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
       acc.merge(ITkTrackRecoCfg(flags))
   else:
       from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
       acc.merge(InDetTrackRecoCfg(flags))
              
   if flags.Detector.GeometryITk:
       itemsToRecord= ['TrackCollection#CombinedITkTracks', 'TrackCollection#ResolvedConversionTracks', 'InDet::PixelClusterContainer#ITkPixelClusters', "InDet::SCT_ClusterContainer#ITkStripClusters"]
   else:
       itemsToRecord = ['TrackCollection#CombinedInDetTracks', 'TrackCollection#DisappearingTracks', 'TrackCollection#ResolvedForwardTracks', 'TrackCollection#ExtendedLargeD0Tracks', 'InDet::TRT_DriftCircleContainer#TRT_DriftCircles', "InDet::PixelClusterContainer#PixelClusters", "InDet::SCT_ClusterContainer#SCT_Clusters"]
   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   acc.merge(OutputStreamCfg(flags, "RDO", ItemList=itemsToRecord, takeItemsFromInput=True))
   # Add in-file MetaData
   from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
   acc.merge(SetupMetaDataForStreamCfg(flags, "RDO"))
   return acc
