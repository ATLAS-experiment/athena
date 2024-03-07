#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from TrigInDetConfig.TrigInDetConfig import InDetCacheNames
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
        DataObjects= {('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                      ('InDet::PixelClusterContainerCache', 'PixelTrigClustersCache'),
                      ('PixelRDO_Cache', 'PixRDOCache'),
                      ('InDet::SCT_ClusterContainerCache', 'SCT_ClustersCache'),
                      ('SCT_RDO_Cache', 'SctRDOCache'),
                      ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.PixBSErrCacheKey ),
                      ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTBSErrCacheKey ),
                      ( 'IDCInDetBSErrContainer_Cache' , InDetCacheNames.SCTFlaggedCondCacheKey ),
                      ('SpacePointCache', 'PixelSpacePointCache'),
                      ('SpacePointCache', 'SctSpacePointCache'),
                      ('xAOD::EventInfo', 'EventInfo'),
                      ('TrigRoiDescriptorCollection', str(self.rois)),
                      ( 'TagInfo' , 'DetectorStore+ProcessingTags' )} )

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

    return acc
        
  def viewDataVerifierAfterPattern(self, viewVerifier='IDViewDataVerifierForAmbi') -> ComponentAccumulator:
    
    acc = ComponentAccumulator()

    ViewDataVerifier = \
        CompFactory.AthViews.ViewDataVerifier( 
            name = viewVerifier + "_" + self.signature,
            DataObjects = {
                ( 'InDet::PixelGangedClusterAmbiguities' , 'ITkPixelClusterAmbiguitiesMap'),
                ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map'),
            }
        )

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
      from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg
      acc.merge(ITkTrackParticleCnvAlgCfg(self.flags,
                                          name = "ITkTrigTrackParticleCnvAlg"+signature,
                                          TrackContainerName = self.flags.Tracking.ActiveConfig.trkTracks_FTF,
                                          xAODTrackParticlesFromTracksContainerName = self.flags.Tracking.ActiveConfig.tracks_FTF))
    return acc

  def ambiguitySolver(self) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    if self.inView:
      acc.merge(self.viewDataVerifierAfterPattern())

    from TrkConfig.TrkAmbiguitySolverConfig import ITkTrkAmbiguityScoreCfg
    acc.merge(
      ITkTrkAmbiguityScoreCfg(
        self.flags,
        name = "TrkAmbiguityScore_",
        SiSPSeededTrackCollectionKey=self.flags.Tracking.ActiveConfig.trkTracks_FTF
      )
    )

    from TrkConfig.TrkAmbiguitySolverConfig import ITkTrkAmbiguitySolverCfg
    acc.merge(
      ITkTrkAmbiguitySolverCfg(
        self.flags,
        name  = "TrkAmbiguitySolver_",
        ResolvedTrackCollectionKey=self.flags.Tracking.ActiveConfig.trkTracks_IDTrig+"_Amb"
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
      xAODTrackParticlesFromTracksContainerName = self.flags.Tracking.ActiveConfig.tracks_IDTrig
    ))
    return acc
