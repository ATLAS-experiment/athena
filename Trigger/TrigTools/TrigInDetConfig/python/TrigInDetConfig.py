#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Logging import logging

class InDetCacheNames(object):
  Pixel_ClusterKey   = "PixelTrigClustersCache"
  SCT_ClusterKey     = "SCT_ClustersCache"
  SpacePointCachePix = "PixelSpacePointCache"
  SpacePointCacheSCT = "SctSpacePointCache"
  SCTBSErrCacheKey   = "SctBSErrCache"
  SCTFlaggedCondCacheKey = "SctFlaggedCondCache"
  SCTRDOCacheKey     = "SctRDOCache"
  PixRDOCacheKey     = "PixRDOCache"
  PixBSErrCacheKey   = "PixBSErrCache"
  TRTRDOCacheKey     = "TrtRDOCache"
  TRT_DriftCircleCacheKey = "TRT_DriftCircleCache"

def InDetIDCCacheCreatorCfg(flags):
  #Create IdentifiableCaches
  acc = ComponentAccumulator()
  InDet__CacheCreator=CompFactory.getComp("InDet::CacheCreator")
  InDetCacheCreatorTrig = InDet__CacheCreator(name = "InDetCacheCreatorTrig",
                                              TRT_DriftCircleKey = InDetCacheNames.TRT_DriftCircleCacheKey,
                                              Pixel_ClusterKey   = InDetCacheNames.Pixel_ClusterKey,
                                              SCT_ClusterKey     = InDetCacheNames.SCT_ClusterKey,
                                              SpacePointCachePix = InDetCacheNames.SpacePointCachePix,
                                              SpacePointCacheSCT = InDetCacheNames.SpacePointCacheSCT,
                                              SCTRDOCacheKey     = InDetCacheNames.SCTRDOCacheKey,
                                              SCTBSErrCacheKey   = InDetCacheNames.SCTBSErrCacheKey,
                                              SCTFlaggedCondCacheKey = InDetCacheNames.SCTFlaggedCondCacheKey,
                                              PixRDOCacheKey     = InDetCacheNames.PixRDOCacheKey,
                                              PixBSErrCacheKey   = InDetCacheNames.PixBSErrCacheKey,
                                              TRTRDOCacheKey     = InDetCacheNames.TRTRDOCacheKey)
  if not flags.Detector.GeometryTRT:
    InDetCacheCreatorTrig.disableTRT = True

  acc.addEventAlgo( InDetCacheCreatorTrig )
  return acc


@AccumulatorCache
def trigInDetFastTrackingCfg( inflags, roisKey="EMRoIs", signatureName='', in_view=True ):

  log = logging.getLogger("trigInDetFastTrackingCfg")
  from TrigInDetConfig.utils import getFlagsForActiveConfig
  flags = getFlagsForActiveConfig(inflags, signatureName, log)
 
  """ Generates fast tracking config, it is a primary config function """

  from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence

  seq = InnerTrackingTrigSequence.create(flags, 
                                         flags.Tracking.ActiveConfig.input_name, 
                                         rois   = roisKey,
                                         inView = "VDVInDetFTF" if in_view else None)
  acc = seq.sequence("FastTrackFinder")

  return acc


@AccumulatorCache
def trigInDetLRTCfg(flags, LRTInputCollection, roisKey, in_view, extra_view_inputs=tuple()):
  from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence
  viewname = "VDVInDetLRT" if in_view else None
  seq = InnerTrackingTrigSequence.create(flags,
                                         flags.Tracking.ActiveConfig.input_name,
                                         rois   = roisKey,
                                         inView = viewname)
  acc = ComponentAccumulator()
  if in_view:
    acc.addEventAlgo( CompFactory.AthViews.ViewDataVerifier(
      name = viewname + "_" + flags.Tracking.ActiveConfig.input_name,
      DataObjects = {
        ( 'TrigRoiDescriptorCollection' ,  f'StoreGateSvc+{roisKey}' ),
        ( 'TrackCollection' ,               'StoreGateSvc+HLT_IDTrkTrack_FS_FTF' ),
        ( 'SpacePointContainer' ,           'StoreGateSvc+SCT_TrigSpacePoints' ),
        ( 'InDet::PixelClusterContainer' ,  'StoreGateSvc+PixelTrigClusters' ),
        ( 'InDet::SCT_ClusterContainer' ,   'StoreGateSvc+SCT_TrigClusters' ),
      } | set(extra_view_inputs)
    ) )

    acc.merge(seq.viewDataVerifier(viewname))
    acc.merge(seq.dataPreparation())
    
  acc.merge(seq.fastTrackFinder(inputTracksName = LRTInputCollection))
  
  return acc



############################################################################################################################
# precision tracking
############################################################################################################################

@AccumulatorCache
def trigInDetPrecisionTrackingCfg( inflags, rois, signatureName, in_view=True ):
 
  """ Generates precision tracking config, it is a primary config function """

  acc = ComponentAccumulator()
  log = logging.getLogger("trigInDetPrecisionTrackingCfg")
  from TrigInDetConfig.utils import getFlagsForActiveConfig
  flags = getFlagsForActiveConfig(inflags, signatureName, log)

  if in_view:

    verifier = CompFactory.AthViews.ViewDataVerifier( name = 'VDVInDetPrecision'+flags.Tracking.ActiveConfig.input_name,
                                                      DataObjects= {('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                                                                    ('TrigRoiDescriptorCollection', flags.Tracking.ActiveConfig.roi),
                                                                    ( 'TagInfo', 'DetectorStore+ProcessingTags' ), 
                                                                    ( 'TrackCollection', flags.Tracking.ActiveConfig.trkTracks_FTF )} )

    acc.addEventAlgo(verifier)

  from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence

  seq = InnerTrackingTrigSequence.create(flags, 
                                         flags.Tracking.ActiveConfig.input_name, 
                                         rois = flags.Tracking.ActiveConfig.roi, 
                                         inView = verifier.getName() if in_view else '')
  
  acc.merge(seq.sequenceAfterPattern(rois = rois))

  return acc

def trigInDetVertexingCfg(flags, inputTracks, outputVtx):
  
  acc = ComponentAccumulator()

  acc.addEventAlgo( CompFactory.AthViews.ViewDataVerifier(
    name = "VtxVDV_" + flags.Tracking.ActiveConfig.input_name,
    DataObjects = {
      ( 'ActsGeometryContext' , 'StoreGateSvc+ActsAlignment' ) 
    } 
  ))
                
  from InDetConfig.InDetPriVxFinderConfig import InDetTrigPriVxFinderCfg
  acc.merge(InDetTrigPriVxFinderCfg(flags, inputTracks = inputTracks, outputVtx =outputVtx))

  return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    ComponentAccumulator.debugMode = "trackCA trackPublicTool trackEventAlgo trackCondAlgo trackPrivateTool"
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.lock()
    # this configuration is not runable, the test checks if there is no mistake in python scripts above
    # output can be used by experts to check actual configuration (e.g. here we configure to run on RAW and it should be reflected in settings)
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg( flags )
    roisKey = "ElectronRoIs"
    
    flags = flags.cloneAndReplace("Tracking.ActiveConfig", "Trigger.InDetTracking.electron")
    acc.merge( trigInDetFastTrackingCfg( flags, roisKey=roisKey, signatureName="electron" ) )
    acc.merge( trigInDetPrecisionTrackingCfg( flags, rois=roisKey, signatureName="electron", in_view=True) )
    acc.merge( trigInDetVertexingCfg( flags, inputTracks=flags.Tracking.ActiveConfig.tracks_FTF, outputVtx="testVtx") )


    acc.printConfig(withDetails=True, summariseProps=True)
    acc.store( open("test.pkl", "wb") )
