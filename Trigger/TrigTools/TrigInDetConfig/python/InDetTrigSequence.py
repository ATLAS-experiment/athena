#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from TrigInDetConfig.TrigInDetConfig import InDetCacheNames
from AthenaConfiguration.Enums import Format
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaCommon.Logging import logging


class InDetTrigSequence(InnerTrackerTrigSequence):

  def __init__(self,flags : AthConfigFlags, signature : str, rois : str, inView : str):
    super().__init__(flags, signature,rois,inView)
    self.log = logging.getLogger("InDetTrigSequence")
    self.log.info(f"signature: {self.signature} rois: {self.rois} inview: {self.inView}")
    
  def offlinePattern(self) -> ComponentAccumulator:

    ca = ComponentAccumulator()

    from InDetConfig.SiSPSeededTrackFinderConfig import TrigSiSPSeededTrackFinderCfg
    ca.merge(TrigSiSPSeededTrackFinderCfg(self.flags,
                                          name = 'EFsiSPSeededTrackFinder'+self.flags.Tracking.ActiveConfig.input_name
    ))

    self.lastTrkCollection = self.flags.Tracking.ActiveConfig.trkTracks_IDTrig
    self.ambiPrefix = "EFAmbi"

    return ca
    

  def viewDataVerifier(self, viewVerifier='IDViewDataVerifier') -> ComponentAccumulator:

    acc = ComponentAccumulator()

    ViewDataVerifier = \
      CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.signature,
                                            DataObjects = {( 'InDet::PixelClusterContainerCache' , InDetCacheNames.Pixel_ClusterKey ),
                                                            ( 'PixelRDO_Cache' , InDetCacheNames.PixRDOCacheKey ),
                                                            ( 'InDet::SCT_ClusterContainerCache' , InDetCacheNames.SCT_ClusterKey ),
                                                            ( 'SCT_RDO_Cache' , InDetCacheNames.SCTRDOCacheKey ),
                                                            ( 'SpacePointCache' , InDetCacheNames.SpacePointCachePix ),
                                                            ( 'SpacePointCache' , InDetCacheNames.SpacePointCacheSCT ),
                                                            ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.PixBSErrCacheKey ),
                                                            ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTBSErrCacheKey ),
                                                            ( 'xAOD::EventInfo' , 'StoreGateSvc+EventInfo' ),
                                                            ( 'TagInfo' , 'DetectorStore+ProcessingTags' )}
                                          )

    isByteStream = self.flags.Input.Format == Format.BS
    if not isByteStream:
      ViewDataVerifier.DataObjects |= {( 'PixelRDO_Container' , 'PixelRDOs' ),
                                       ( 'SCT_RDO_Container' , 'SCT_RDOs' )}

    ViewDataVerifier.DataObjects.add(( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % self.rois ))

    acc.addEventAlgo(ViewDataVerifier)
    return acc


    
  def viewDataVerifierTRT(self, viewVerifier='IDViewDataVerifierTRT') -> ComponentAccumulator:
    
    acc = ComponentAccumulator()

    ViewDataVerifier = \
      CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.signature,
                                             DataObjects = {
                                               ( 'InDet::TRT_DriftCircleContainerCache' , 'StoreGateSvc+TRT_DriftCircleCache'  ),

                                             }
                                            )

    if self.flags.Input.Format == Format.BS:
      ViewDataVerifier.DataObjects.add(( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' ))
    else:
      ViewDataVerifier.DataObjects.add(( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' ))

    acc.addEventAlgo(ViewDataVerifier)
    return acc

  def viewDataVerifierAfterDataPrep(self, viewVerifier='IDViewDataVerifierAfterDataPrep') -> ComponentAccumulator:

    acc = ComponentAccumulator()

    ViewDataVerifier = \
      CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.signature,
                                             DataObjects = {
                                               ( 'SpacePointContainer',           'StoreGateSvc+SCT_TrigSpacePoints' ),
                                               ( 'SpacePointContainer',           'StoreGateSvc+PixelTrigSpacePoints' ),
                                               ( 'SpacePointOverlapCollection',   'StoreGateSvc+OverlapSpacePoints' ),
                                               #( 'InDet::PixelGangedClusterAmbiguities' , 'StoreGateSvc+TrigPixelClusterAmbiguitiesMap' ),
                                               ( 'InDet::SCT_ClusterContainer',   'StoreGateSvc+SCT_TrigClusters' ),
                                               ( 'InDet::PixelClusterContainer',  'StoreGateSvc+PixelTrigClusters' ),
                                             }
                                            )

    if self.flags.Input.Format == Format.BS:
      ViewDataVerifier.DataObjects |= {
        ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' ),
        ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
      }

    acc.addEventAlgo(ViewDataVerifier)
    return acc
    
  def viewDataVerifierAfterPattern(self, viewVerifier='IDViewDataVerifierForAmbi') -> ComponentAccumulator:
    
    acc = ComponentAccumulator()

    ViewDataVerifier = \
      CompFactory.AthViews.ViewDataVerifier( name = viewVerifier + "_" + self.signature,
                                             DataObjects = {
                                               ( 'InDet::PixelGangedClusterAmbiguities' , 'TrigPixelClusterAmbiguitiesMap'),
                                               }
                                            )
    if self.flags.Input.Format == Format.BS:
      ViewDataVerifier.DataObjects |= {
        ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+PixelByteStreamErrs' ),
        ( 'IDCInDetBSErrContainer' , 'StoreGateSvc+SCT_ByteStreamErrs' ),
      }

    acc.addEventAlgo(ViewDataVerifier)
    return acc

  def dataPreparation(self) -> ComponentAccumulator:
    
    signature = self.flags.Tracking.ActiveConfig.input_name
    
    acc = ComponentAccumulator()

    self.log.info(f"DataPrep signature: {self.signature} rois: {self.rois} inview: {self.inView}")

    if self.flags.Input.Format == Format.BS:
      from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import TrigPixelRawDataProviderAlgCfg
      acc.merge(TrigPixelRawDataProviderAlgCfg(self.flags,suffix=signature,RoIs=self.rois))

      from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import TrigSCTRawDataProviderCfg
      acc.merge(TrigSCTRawDataProviderCfg(self.flags,suffix=signature,RoIs=self.rois))
    elif not self.inView:
      from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
      loadRDOs = [( 'PixelRDO_Container' , 'StoreGateSvc+PixelRDOs' ),
                  ( 'SCT_RDO_Container' , 'StoreGateSvc+SCT_RDOs' ) ]
      acc.merge(SGInputLoaderCfg(self.flags, Load=loadRDOs))

    #Clusterisation
    from InDetConfig.InDetPrepRawDataFormationConfig import TrigPixelClusterizationCfg
    acc.merge(TrigPixelClusterizationCfg(self.flags,
                                         self.rois,
                                         name=f"InDetPixelClusterization_{signature}"))


    from InDetConfig.InDetPrepRawDataFormationConfig import TrigSCTClusterizationCfg
    acc.merge(TrigSCTClusterizationCfg(self.flags,
                                       self.rois,
                                       name=f"InDetSCTClusterization_{signature}"))

    return acc
  

  def dataPreparationTRT(self) ->ComponentAccumulator:
  
    acc = ComponentAccumulator()

    acc.merge(self.viewDataVerifierTRT())

    if self.flags.Input.Format == Format.BS:
      from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import TrigTRTRawDataProviderCfg
      acc.merge(TrigTRTRawDataProviderCfg(self.flags, RoIs=self.lastRois))

    elif not self.inView:
      from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
      loadRDOs = [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
      acc.merge(SGInputLoaderCfg(self.flags, Load=loadRDOs))

    from InDetConfig.InDetPrepRawDataFormationConfig import TrigTRTRIOMakerCfg
    signature = self.flags.Tracking.ActiveConfig.input_name
    acc.merge(TrigTRTRIOMakerCfg(self.flags,
                                 self.lastRois,
                                 name=f"TrigTRTDriftCircleMaker_{signature}"))


    return acc


  def spacePointFormation(self) -> ComponentAccumulator:
    
    signature = self.flags.Tracking.ActiveConfig.input_name
    acc = ComponentAccumulator()

    from InDetConfig.SiSpacePointFormationConfig import TrigSiTrackerSpacePointFinderCfg
    acc.merge(TrigSiTrackerSpacePointFinderCfg(self.flags, name="TrigSpacePointFinder"+signature))
    return acc

  def fastTrackFinder(self, 
                      extraFlags : AthConfigFlags = None, 
                      inputTracksName : str = None) -> ComponentAccumulator:
    acc = self.fastTrackFinderBase(extraFlags, inputTracksName)
    if not self.flags.Tracking.ActiveConfig.doZFinderOnly:
      from TrkConfig.TrkParticleCreatorConfig import InDetTrigParticleCreatorToolFTFCfg
      creatorTool = acc.popToolsAndMerge(InDetTrigParticleCreatorToolFTFCfg(self.flags))
      acc.addPublicTool(creatorTool)

      from xAODTrackingCnv.xAODTrackingCnvConfig import TrigTrackParticleCnvAlgCfg
      prefix = "InDet"
      acc.merge(
        TrigTrackParticleCnvAlgCfg(
          self.flags,
          name = prefix+'xAODParticleCreatorAlg'+self.flags.Tracking.ActiveConfig.input_name+'_FTF',
          TrackParticleCreator = creatorTool,
          TrackContainerName = self.lastTrkCollection,
          xAODTrackParticlesFromTracksContainerName = self.flags.Tracking.ActiveConfig.tracks_FTF,
        )
      )
    return acc


  def ambiguitySolver(self) -> ComponentAccumulator:  

    acc = ComponentAccumulator()

    if self.inView:
      acc.merge(self.viewDataVerifierAfterPattern())

    from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguityScore_Trig_Cfg
    acc.merge(
      TrkAmbiguityScore_Trig_Cfg(
        self.flags,
        name = f"{self.ambiPrefix}Score_{self.flags.Tracking.ActiveConfig.input_name}",
        TrackInput = [self.lastTrkCollection],
        AmbiguityScoreProcessor = None
      )
    )

    from TrkConfig.TrkAmbiguitySolverConfig import TrkAmbiguitySolver_Trig_Cfg
    acc.merge(
      TrkAmbiguitySolver_Trig_Cfg(
        self.flags,
        name = f"{self.ambiPrefix}guitySolver_{self.flags.Tracking.ActiveConfig.input_name}",
      )
    )

    self.lastTrkCollection = self.flags.Tracking.ActiveConfig.trkTracks_IDTrig+"_Amb"
    return acc


  def trtExtensions(self) -> ComponentAccumulator:

    acc = self.dataPreparationTRT()

    from InDetConfig.TRT_TrackExtensionAlgConfig import Trig_TRT_TrackExtensionAlgCfg
    acc.merge(Trig_TRT_TrackExtensionAlgCfg(self.flags, self.lastTrkCollection, name="TrigTrackExtensionAlg%s"% self.signature))

    from InDetConfig.InDetExtensionProcessorConfig import TrigInDetExtensionProcessorCfg
    acc.merge(TrigInDetExtensionProcessorCfg(self.flags, name="TrigExtensionProcessor%s"% self.signature))

    self.lastTrkCollection = self.flags.Tracking.ActiveConfig.trkTracks_IDTrig

    return acc

  
  def xAODParticleCreation(self) -> ComponentAccumulator:

    if self.flags.Tracking.ActiveConfig.doTRT:
      acc = self.dataPreparationTRT()
    else:
      acc = ComponentAccumulator()

    from xAODTrackingCnv.xAODTrackingCnvConfig import TrigTrackParticleCnvAlgCfg
    prefix = "InDet"
    acc.merge(
      TrigTrackParticleCnvAlgCfg(
        self.flags,
        name = prefix+'xAODParticleCreatorAlg'+self.flags.Tracking.ActiveConfig.input_name+'_IDTrig',
        TrackContainerName = self.lastTrkCollection,
        xAODTrackParticlesFromTracksContainerName = self.flags.Tracking.ActiveConfig.tracks_IDTrig,
      ))
    return acc

