#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Logging import logging

class ITkTrigSequence(InnerTrackerTrigSequence):
  def __init__(self, flags : AthConfigFlags, signature : str, rois : str, inView : str):
    super().__init__(flags, signature,rois,inView)
    self.log = logging.getLogger("ITkTrigSequence")
    self.log.info(f"signature: {self.signature} rois: {self.rois} inview: {self.inView}")
      
    
  def viewDataVerifier(self, viewVerifier='IDViewDataVerifier') -> ComponentAccumulator:

    acc = ComponentAccumulator()

    ViewDataVerifier = CompFactory.AthViews.ViewDataVerifier( 
        name = viewVerifier + "_" + self.signature,
        DataObjects= {('xAOD::EventInfo',                   'StoreGateSvc+EventInfo'),
                      ('InDet::PixelClusterContainerCache', self.flags.Trigger.ITkTracking.PixelClusterCacheKey),
                      ('PixelRDO_Cache',                    self.flags.Trigger.ITkTracking.PixRDOCacheKey),
                      ('InDet::SCT_ClusterContainerCache',  self.flags.Trigger.ITkTracking.SCTClusterCacheKey),
                      ('SCT_RDO_Cache',                     self.flags.Trigger.ITkTracking.SCTRDOCacheKey),
                      ('IDCInDetBSErrContainer_Cache' ,     self.flags.Trigger.ITkTracking.PixBSErrCacheKey ),
                      ('IDCInDetBSErrContainer_Cache' ,     self.flags.Trigger.ITkTracking.SCTBSErrCacheKey ),
                      ('IDCInDetBSErrContainer_Cache' ,     self.flags.Trigger.ITkTracking.SCTFlaggedCondCacheKey ),
                      ('SpacePointCache',                   self.flags.Trigger.ITkTracking.SpacePointCachePix),
                      ('SpacePointCache',                   self.flags.Trigger.ITkTracking.SpacePointCacheSCT),
                      ('xAOD::EventInfo',                   'EventInfo'),
                      ('TrigRoiDescriptorCollection',       str(self.rois)),
                      ( 'TagInfo' ,                         'DetectorStore+ProcessingTags' )} )

    if self.flags.Input.isMC:
        ViewDataVerifier.DataObjects |= {( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                                         ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                                         ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map')}
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        sgil_load = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                    ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                    ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map')]
        acc.merge(SGInputLoaderCfg(self.flags, Load=sgil_load))

    acc.addEventAlgo(ViewDataVerifier)
    return acc

  def dataPreparation(self) -> ComponentAccumulator:
    
    signature = self.flags.Tracking.ActiveConfig.input_name
    
    acc = ComponentAccumulator()

    self.log.info(f"DataPrep signature: {self.signature} rois: {self.rois} inview: {self.inView}")

    if not self.inView:
      from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
      loadRDOs = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                  ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                  ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map') ]
      acc.merge(SGInputLoaderCfg(self.flags, Load=loadRDOs))

    #Clusterisation
    from InDetConfig.InDetPrepRawDataFormationConfig import ITkTrigPixelClusterizationCfg, ITkTrigStripClusterizationCfg
    acc.merge(ITkTrigPixelClusterizationCfg(self.flags, roisKey=self.rois, signature=signature))
    acc.merge(ITkTrigStripClusterizationCfg(self.flags, roisKey=self.rois, signature=signature))
    from InDetConfig.TrackRecoConfig import SiDetectorElementStatusCfg
    acc.merge(SiDetectorElementStatusCfg(self.flags,f"_{signature}"))
    return acc
        
  def viewDataVerifierAfterPattern(self, viewVerifier='IDViewDataVerifierForAmbi') -> ComponentAccumulator:
    
    acc = ComponentAccumulator()

    dataObjects = [
                ( 'InDet::PixelGangedClusterAmbiguities' , self.flags.Trigger.ITkTracking.ClusterAmbiguitiesMap),
                ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map')
            ]
    from TrigInDetConfig.TrigInDetConfig import InDetExtraDataObjectsFromDataPrep
    InDetExtraDataObjectsFromDataPrep(self.flags,dataObjects)

    ViewDataVerifier = \
        CompFactory.AthViews.ViewDataVerifier( 
            name = viewVerifier + "_" + self.signature,
            DataObjects = dataObjects
        )
    from TrigInDetConfig.TrigInDetConfig import InDetExtraDataObjectsFromDataPrep

    acc.addEventAlgo(ViewDataVerifier)
    return acc


  def spacePointFormation(self) -> ComponentAccumulator:
      
    signature = self.flags.Tracking.ActiveConfig.input_name
    acc = ComponentAccumulator()

    from InDetConfig.SiSpacePointFormationConfig import ITkTrigSiTrackerSpacePointFinderCfg
    acc.merge(ITkTrigSiTrackerSpacePointFinderCfg(self.flags, signature=signature))
    return acc

  def fastTrackFinder(self, 
                      extraFlags : AthConfigFlags = None, 
                      inputTracksName : str = None) -> ComponentAccumulator:
    acc = self.fastTrackFinderBase(extraFlags, inputTracksName)
    signature = self.flags.Tracking.ActiveConfig.input_name

    if not self.flags.Tracking.ActiveConfig.doZFinderOnly:
      self.lastTrkCollection = self.flags.Tracking.ActiveConfig.trkTracks_FTF
      from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
      acc.merge(ITkTrackParticleCnvAlgCfg(self.flags,
                                          name = "ITkTrigTrackParticleCnvAlg"+signature,
                                          TrackContainerName = self.lastTrkCollection,
                                          xAODTrackParticlesFromTracksContainerName = self.flags.Tracking.ActiveConfig.tracks_FTF))
    
    return acc

  def ambiguitySolver(self) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    if self.inView:
      acc.merge(self.viewDataVerifierAfterPattern())

    from TrkConfig.TrkAmbiguitySolverConfig import TrigITkTrkAmbiguityScoreCfg
    acc.merge(
      TrigITkTrkAmbiguityScoreCfg(
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

  def xAODParticleCreation(self) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
    prefix = "ITk"
    acc.merge(ITkTrackParticleCnvAlgCfg(
      self.flags,
      name = prefix+'xAODParticleCreatorAlg'+self.flags.Tracking.ActiveConfig.input_name+'_IDTrig',
      TrackContainerName = self.lastTrkCollection,
      xAODTrackParticlesFromTracksContainerName = self.flags.Tracking.ActiveConfig.tracks_IDTrig, 
    ))
    return acc

  def offlinePattern(self) -> ComponentAccumulator:

    ca = ComponentAccumulator()

    from InDetConfig.SiSPSeededTrackFinderConfig import TrigITkSiSPSeededTrackFinderCfg

    self.lastTrkCollection = self.flags.Tracking.ActiveConfig.trkTracks_IDTrig
    ca.merge(TrigITkSiSPSeededTrackFinderCfg(self.flags,
                                             name = 'EFsiSPSeededTrackFinder'+self.flags.Tracking.ActiveConfig.input_name,
                                             TracksLocation = self.lastTrkCollection, 
    ))

    self.ambiPrefix = "EFAmbi"

    return ca
