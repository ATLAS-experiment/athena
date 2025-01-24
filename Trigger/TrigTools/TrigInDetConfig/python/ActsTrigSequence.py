#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Logging import logging

class ActsTrigSequence(InnerTrackerTrigSequence):
  def __init__(self, flags : AthConfigFlags, signature : str, rois : str, inView : str):
    super().__init__(flags, signature,rois,inView)
    self.log = logging.getLogger("ActsTrigSequence")
    self.log.info(f"signature: {self.signature} rois: {self.rois} inview: {self.inView}")
      
    
  def viewDataVerifier(self, viewVerifier='IDViewDataVerifier') -> ComponentAccumulator:

    acc = ComponentAccumulator()

    ViewDataVerifier = CompFactory.AthViews.ViewDataVerifier( 
        name = viewVerifier + "_" + self.signature,
        DataObjects= {('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
                      ('PixelRDO_Cache', 'PixRDOCache'),
                      ('SCT_RDO_Cache', 'SctRDOCache'),
                      #enable later when BS ready
                      #( 'IDCInDetBSErrContainer_Cache' , self.flags.Trigger.ITkTracking.PixBSErrCacheKey ),
                      #( 'IDCInDetBSErrContainer_Cache' , self.flags.Trigger.ITkTracking.SCTBSErrCacheKey ),
                      #( 'IDCInDetBSErrContainer_Cache' , self.flags.Trigger.ITkTracking.SCTFlaggedCondCacheKey ),
                      ( 'ActsTrk::Cache::Handles<xAOD::SpacePoint>::IDCBackend' , 'StoreGateSvc+ActsPixelSpacePointCache_Back' ),
                      ('ActsTrk::Cache::Handles<xAOD::SpacePoint>::IDCBackend', 'StoreGateSvc+ActsStripSpacePointCache_Back'),
                      ('ActsTrk::Cache::Handles<xAOD::SpacePoint>::IDCBackend', 'StoreGateSvc+ActsStripOverlapSpacePointCache_Back'),
                      ( 'ActsTrk::Cache::Handles<xAOD::PixelCluster>::IDCBackend' , 'StoreGateSvc+ActsPixelClusterCache_Back' ),
                      ( 'ActsTrk::Cache::Handles<xAOD::StripCluster>::IDCBackend' , 'StoreGateSvc+ActsStripClusterCache_Back' ),
                      ('xAOD::EventInfo', 'EventInfo'),
                      ( 'ActsGeometryContext' , 'StoreGateSvc+ActsAlignment' ),
                      ('TrigRoiDescriptorCollection', str(self.rois)),
                      ( 'TagInfo' , 'DetectorStore+ProcessingTags' )} )

    if self.flags.Input.isMC:
        ViewDataVerifier.DataObjects |= {( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                                         ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                                         ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map'),
                      ( 'ActsTrk::Cache::Handles<xAOD::SpacePoint>::IDCBackend' , 'StoreGateSvc+ActsPixelSpacePointCache_Back' ),
                      ('ActsTrk::Cache::Handles<xAOD::SpacePoint>::IDCBackend', 'StoreGateSvc+ActsStripSpacePointCache_Back'),
                      ('ActsTrk::Cache::Handles<xAOD::SpacePoint>::IDCBackend', 'StoreGateSvc+ActsStripOverlapSpacePointCache_Back'),
                      ( 'ActsTrk::Cache::Handles<xAOD::PixelCluster>::IDCBackend' , 'StoreGateSvc+ActsPixelClusterCache_Back' ),
                      ( 'ActsTrk::Cache::Handles<xAOD::StripCluster>::IDCBackend' , 'StoreGateSvc+ActsStripClusterCache_Back' ),
                      ( 'ActsGeometryContext' , 'StoreGateSvc+ActsAlignment' )}
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        sgil_load = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                    ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                    ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map'),]
        acc.merge(SGInputLoaderCfg(self.flags, Load=sgil_load))

    acc.addEventAlgo(ViewDataVerifier)
    return acc

  def dataPreparation(self) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    self.log.info(f"DataPrep signature: {self.signature} rois: {self.rois} inview: {self.inView}")

    if not self.inView:
      from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
      loadRDOs = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                  ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                  ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map') ]
      acc.merge(SGInputLoaderCfg(self.flags, Load=loadRDOs))

    #Clusterisation
    from ActsConfig.ActsClusterizationConfig import ActsPixelClusterizationAlgCfg,ActsStripClusterizationAlgCfg,ActsPixelClusterPreparationAlgCfg,ActsStripClusterPreparationAlgCfg

    acc.merge(ActsPixelClusterizationAlgCfg(self.flags, name="ActsPixelClusterizationAlg_"+self.signature,useCache=True, RoIs=self.rois,ClustersKey="ITkPixelClusters_"+self.signature))
    acc.merge(ActsStripClusterizationAlgCfg(self.flags, name="ActsStripClusterizationAlg_"+self.signature,useCache=True, RoIs=self.rois,ClustersKey="ITkStripClusters_"+self.signature))

    if self.flags.Acts.useCache:
      acc.merge(ActsPixelClusterPreparationAlgCfg(self.flags, "ActsPixelClusterViewFiller_"+self.signature, True,OutputCollection="ITkPixelClusters_Cached", InputIDC="ActsPixelClustersCache", RoIs=self.rois))
      acc.merge(ActsStripClusterPreparationAlgCfg(self.flags, "ActsStripClusterViewFiller_"+self.signature, True,OutputCollection="ITkStripClusters_Cached", InputIDC="ActsStripClustersCache", RoIs=self.rois))

    return acc
        
  def viewDataVerifierAfterPattern(self, viewVerifier='ActsViewDataVerifierForAmbi') -> ComponentAccumulator:
    
    acc = ComponentAccumulator()

    ViewDataVerifier = \
        CompFactory.AthViews.ViewDataVerifier( 
            name = viewVerifier + "_" + self.signature,
            DataObjects = {
                ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map'),
                ( 'ActsGeometryContext' , 'StoreGateSvc+ActsAlignment' )
            }
        )

    acc.addEventAlgo(ViewDataVerifier)
    return acc


  def spacePointFormation(self) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from ActsConfig.ActsSpacePointFormationConfig import ActsPixelSpacePointFormationAlgCfg,ActsPixelSpacePointPreparationAlgCfg

    acc.merge(ActsPixelSpacePointFormationAlgCfg(self.flags,name="PixelSPFormation_"+self.signature,useCache=self.flags.Acts.useCache, PixelClusters = "ITkPixelClusters_Cached" if self.flags.Acts.useCache else "ITkPixelClusters_"+self.signature, PixelSpacePoints = "ITkPixelSpacepoints_"+self.signature))
    
    if self.flags.Acts.useCache:
      acc.merge(ActsPixelSpacePointPreparationAlgCfg(self.flags,name="PixelSPVF_"+self.signature,useCache=True, RoIs=self.rois, OutputCollection="ITkPixelSpacePoints_Cached", InputIDC="ActsPixelSpacePointCache"))

    return acc

  def fastTrackFinder(self, 
                      extraFlags : AthConfigFlags = None, 
                      inputTracksName : str = None) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from ActsConfig.ActsSeedingConfig import ActsPixelSeedingAlgCfg

    acc.merge(ActsPixelSeedingAlgCfg(self.flags, name="ActsPixelSeedingAlg_"+self.signature, InputSpacePoints=['ITkPixelSpacePoints_Cached'] if self.flags.Acts.useCache else ['ITkPixelSpacepoints_'+self.signature], useFastTracking=True))

    from ActsConfig.ActsTrackFindingConfig import ActsMainTrackFindingAlgCfg, ActsTrackToTrackParticleCnvAlgCfg
    measurements = ["ITkPixelClusters_Cached" if self.flags.Acts.useCache else "ITkPixelClusters_"+self.signature,
                    "ITkStripClusters_Cached" if self.flags.Acts.useCache else "ITkStripClusters_"+self.signature]
    
    trackfinding = ActsMainTrackFindingAlgCfg(self.flags, 
                                              name="ActsTrackFindingAlg_"+self.signature, 
                                              ACTSTracksLocation=self.flags.Tracking.ActiveConfig.trkTracks_FTF,
                                              SeedLabels=["PPP"],SeedContainerKeys=["ActsPixelSeeds"],
                                              DetectorElementsKeys=['ITkPixelDetectorElementCollection'],
                                              UncalibratedMeasurementContainerKeys=measurements)
    
    acc.merge(trackfinding)
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(self.flags,name="ActsTrackParticleCreator_"+self.signature, ACTSTracksLocation=[self.flags.Tracking.ActiveConfig.trkTracks_FTF], TrackParticlesOutKey=self.flags.Tracking.ActiveConfig.tracks_FTF))

    return acc

  def ambiguitySolver(self) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if self.inView:
      acc.merge(self.viewDataVerifierAfterPattern())

    from ActsConfig.ActsTrackFindingConfig import ActsMainAmbiguityResolutionAlgCfg

    acc.merge(ActsMainAmbiguityResolutionAlgCfg(self.flags, "ActsAmbiguityResolutionAlg_"+self.signature, TracksLocation=self.flags.Tracking.ActiveConfig.trkTracks_FTF, ResolvedTracksLocation=self.flags.Tracking.ActiveConfig.trkTracks_IDTrig))
    

    return acc

  def xAODParticleCreation(self) -> ComponentAccumulator:

    acc = ComponentAccumulator()
    from ActsConfig.ActsTrackFindingConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(self.flags,"ActsTrackParticleCreator_Ambi_"+self.signature, ACTSTracksLocation=[self.flags.Tracking.ActiveConfig.trkTracks_IDTrig], TrackParticlesOutKey=self.flags.Tracking.ActiveConfig.tracks_IDTrig))

    return acc
