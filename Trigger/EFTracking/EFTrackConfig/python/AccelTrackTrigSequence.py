#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import FlagEnum

class DataLocation(FlagEnum):
    HOST   = "host"    # Athena objects on CPU
    DEVICE = "device"  # Traccc buffers on GPU

doTruth = False

class AccelTrackTrigSequence(InnerTrackerTrigSequence):
  def __init__(self, flags : AthConfigFlags, signature : str, rois : str, inView : str):
    super().__init__(flags, signature,rois,inView)
    self.log = logging.getLogger("AccelTrigTrigSequence")
    self.log.info(f"EFTracking signature: {self.signature} rois: {self.rois} inview: {self.inView}")
    self.log.info(f"AOD config: {self.flags.Trigger.AODEDMSet}")
  def sequence(self, recoType : str) -> ComponentAccumulator:
    """ main method to instantiate tracking in the trigger menu
        recoType can be used to generate a specific step. The pattern recognition
        and production of xAOD trackparticles should happen in FastTrackFinder step
    """
    ca = ComponentAccumulator()

    from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
    ca.merge(GEN_AOD2xAODCfg(self.flags))
    
    pipeline = str(self.flags.Trigger.EFTrackPipeline).upper()

    if self.inView:
      #to invoke VDV to make sure RoI and other info is available in view
      self.log.info(f"Inside view '{self.inView}', configuring ViewDataVerifier (pipeline={pipeline}).")
      match pipeline:
        case "F100":
          self.log.info(f"Configuring ViewDataVerifier for F100 pipeline in view '{self.inView}'")
          ca.merge(self.viewDataVerifierFPGA())
          pass 
        case "G200":
          self.log.info(f"Configuring ViewDataVerifier for G200 pipeline in view '{self.inView}'")
          ca.merge(self.viewDataVerifierGPU())
          pass
    else:
      self.log.info("Not inside view, skipping ViewDataVerifier configuration adding SGInputLoader instead")

      # Add SGInputLoader 
      if self.flags.Input.isMC:
        self.log.info(f"Added SGInputLoader for view '{self.inView}' (MC data)")
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        sgil_load = [('PixelRDO_Container', 'StoreGateSvc+ITkPixelRDOs'),
                    ('SCT_RDO_Container', 'StoreGateSvc+ITkStripRDOs'),
                    ('InDetSimDataCollection', 'ITkPixelSDO_Map'),
                    ('InDetSimDataCollection' , 'StoreGateSvc+ITkStripSDO_Map'),
                    ( 'McEventCollection' , 'StoreGateSvc+TruthEvent' )]
        ca.merge(SGInputLoaderCfg(self.flags, Load=sgil_load))


    #if there is need for splitting of the sequence in steps it can be added here
    if recoType =="FastTrackFinder":
      ca.merge(self.trackFinding())

    return ca
  
  def g200(self) -> ComponentAccumulator:

    ca = ComponentAccumulator()
    
    from ActsGPUGeometry.ActsGPUGeometryConfig import DeviceDetectorDescriptionCondAlgCfg
    ca.merge(DeviceDetectorDescriptionCondAlgCfg(self.flags))

    # setup RDO converter
    if self.flags.Acts.EDM.PhaseII :
        from ActsConfig.ActsPhaseIIRawDataEdmConfig import (
            PhaseIIPixelRawDataContainerCfg,
            PhaseIIStripRawDataContainerCfg,
        )
        ca.merge(PhaseIIPixelRawDataContainerCfg(self.flags))
        ca.merge(PhaseIIStripRawDataContainerCfg(self.flags))
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import PhaseIIRDOtoTracccCellConverterAlgCfg
        ca.merge(PhaseIIRDOtoTracccCellConverterAlgCfg(self.flags,
            name=f"RDOtoTracccCellConverterAlg_{self.signature}",
            TracccCells = "TracccCellCollection",
            ))
    else:
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg
        ca.merge(RDOtoTracccCellConverterAlgCfg(self.flags,
            name=f"RDOtoTracccCellConverterAlg_{self.signature}",
            RoIs=self.rois,
            isRoI_Seeded= (False if self.signature == "fullScan" else True),
            TracccCells = "TracccCellCollection",
            ))

    # setup traccc clusterization
    from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceClusterizationAlgCfg
    ca.merge(DeviceClusterizationAlgCfg(self.flags,
        name=f"DeviceClusterizationAlg_{self.signature}",
        InputTracccCells="TracccCellCollection",
        OutputTracccMeasurements="TracccMeasurementCollection",
        OutputTracccClusters="TracccClusterCollection",
        RetrieveClusterCells=self.flags.Tracking.doTruth,
        previousExtension=None,
        ))
    clustersLocation = DataLocation.DEVICE

    # --- Seeding ---
    if self.flags.Acts.Device.doSeeding:

        if clustersLocation is not DataLocation.DEVICE:
            raise ValueError("Device seeding requires device clusterization "
                "(self.flags.Acts.Device.doClusterization=True): it reads the "
                "traccc measurement collection straight out of device memory "
                "and there is currently no host->device measurement converter.")

        # Pixel space point formation on device
        from ActsGPUDataPreparation.ActsGPUDataPreparationConfig import DeviceSPFormationAlgCfg
        ca.merge(DeviceSPFormationAlgCfg(self.flags,
            name=f"DeviceSPFormationAlg_{self.signature}",
            InputTracccMeasurements="TracccMeasurementCollection",
            OutputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
            )) 

        from ActsConfig.ActsConfigFlags import SeedingStrategy

        if self.flags.Acts.Device.seedingStrategy is SeedingStrategy.Gbts:
            from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceGBTSSeedingAlgCfg
            ca.merge(DeviceGBTSSeedingAlgCfg(self.flags,
                name=f"DeviceGBTSSeedingAlg_{self.signature}",
                InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
                InputTracccMeasurements="TracccMeasurementCollection",
                OutputTracccPixelSeeds="TracccPixelSeedCollection"))
        else:
            from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceTripletSeedingAlgCfg
            ca.merge(DeviceTripletSeedingAlgCfg(self.flags,
                name=f"DeviceTripletSeedingAlg_{self.signature}",
                InputTracccPixelSpacepoints="TracccPixelSpacepointCollection",
                OutputTracccPixelSeeds="TracccPixelSeedCollection"))

        seedsLocation = DataLocation.DEVICE

    else:

        # If clusterization was on device, need to copy them to host first
        # and schedule space point formation
        if clustersLocation is DataLocation.DEVICE:
            from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccMeasurementConverterAlgCfg
            ca.merge(TracccMeasurementConverterAlgCfg(self.flags,
                name=f"TracccMeasurementConverterAlg_{self.signature}",
                InputMeasurements="TracccMeasurementCollection",
                InputClusters="TracccClusterCollection",
                InputCells="TracccCellCollection",
                ConvertClustersWithCells = self.flags.Tracking.doTruth,
                OutputPixelSpacePoints="ITkPixelSpacePoints",
                OutputPixelClusters="ITkPixelClusters",
                OutputStripClusters="ITkStripClusters"
            ))

            from ActsConfig.ActsSpacePointFormationConfig import ActsStripSpacePointFormationAlgCfg
            ca.merge(ActsStripSpacePointFormationAlgCfg(self.flags,
                name=f"StripSpacePointFormationAlg_{self.signature}",
                StripClusters="ITkStripClusters",
                StripSpacePoints="ITkStripSpacePoints",
                StripOverlapSpacePoints="ITkStripOverlapSpacePoints"))

            clustersLocation = DataLocation.HOST

            # Truth (as configured in ITkActsDataPreparationCfg)
            # this truth must only be done if you do PRD and SpacePointformation
            # If you only do the latter (== running on ESD) then the needed input (simdata)
            # is not in ESD but the resulting truth (clustertruth) is already there ...
            if self.flags.Tracking.doTruth:
                from ActsConfig.ActsTruthConfig import ActsTruthAssociationAlgCfg, ActsTruthParticleHitCountAlgCfg
                ca.merge(ActsTruthAssociationAlgCfg(self.flags))
                ca.merge(ActsTruthParticleHitCountAlgCfg(self.flags))

        from ActsConfig.ActsSeedingConfig import ActsSeedingCfg
        ca.merge(ActsSeedingCfg(self.flags))
        seedsLocation = DataLocation.HOST


    # --- Track Reconstruction ---
    if self.flags.Acts.Device.doTrackReconstruction:

        if clustersLocation is not DataLocation.DEVICE or seedsLocation is not DataLocation.DEVICE:
            raise ValueError("Device track reconstruction requires device clusterization "
                "(flags.Acts.Device.doClusterization=True) and device seeding "
                "(flags.Acts.Device.doSeeding=True): it reads the traccc measurement "
                "and seed collections straight out of device memory and there is "
                "currently no host->device converter for these yet.")


        from ActsGPUMagField.ActsGPUMagFieldConfig import JSONDeviceMagFieldProviderSvcCfg
        ca.merge(JSONDeviceMagFieldProviderSvcCfg(self.flags,
            DeviceMagFieldObjectName="TracccMagneticField",
            HostMagFieldObjectName="TracccHostMagField",
        ))

        from ActsGPUPatternRecognition.ActsGPUPatternRecognitionConfig import DeviceTrkParamEstimationAlgCfg, DeviceTrackFindingAlgCfg
        ca.merge(DeviceTrkParamEstimationAlgCfg(self.flags,
            name=f"DeviceTrkParamEstimationAlg_{self.signature}",
            InputTracccSpacepoints="TracccPixelSpacepointCollection",
            InputTracccMeasurements="TracccMeasurementCollection",
            InputTracccSeeds="TracccPixelSeedCollection",
            InputTracccMagField="TracccMagneticField",
            OutputTracccTrackParameters="TracccTrackParameterCollection",
        ))

        ca.merge(DeviceTrackFindingAlgCfg(self.flags,
            name=f"DeviceTrackFindingAlg_{self.signature}",
            InputTracccMeasurements="TracccMeasurementCollection",
            InputTracccMagField="TracccMagneticField",
            InputTracccTrackParameters="TracccTrackParameterCollection",
            InputTracccDetectorGeometry="TracccDeviceDetectorGeometry",
            OutputTracccTracks="TracccTrackCollection",
        ))
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccMeasurementConverterAlgCfg
        if clustersLocation is not DataLocation.DEVICE:
            ca.merge(TracccMeasurementConverterAlgCfg(self.flags,
                name=f"TracccMeasurementConverterAlg_{self.signature}",
                InputMeasurements="TracccMeasurementCollection",
                InputClusters="TracccClusterCollection",
                InputCells="TracccCellCollection",
                ConvertClustersWithCells = self.flags.Tracking.doTruth,
                OutputPixelClusters="ITkPixelClusters",
                OutputPixelSpacePoints="ITkPixelSpacePoints",
                OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
                OutputMeasToStripCl="ITkTracccMeasToStripCl",
                OutputStripClusters="ITkStripClusters",
                GeoIdMapping="TracccGeoIdMapping",
            ))

        from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccTrackConverterAlgCfg
        if clustersLocation is DataLocation.DEVICE:
            ca.merge(TracccMeasurementConverterAlgCfg(self.flags,
                    name=f"TracccMeasurementConverterAlg_{self.signature}",
                    InputMeasurements="TracccMeasurementCollection",
                    InputClusters="TracccClusterCollection",
                    InputCells="TracccCellCollection",
                    ConvertClustersWithCells = self.flags.Tracking.doTruth,
                    OutputPixelClusters="ITkPixelClusters",
                    OutputPixelSpacePoints="ITkPixelSpacePoints",
                    OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
                    OutputMeasToStripCl="ITkTracccMeasToStripCl",
                    OutputStripClusters="ITkStripClusters"
            ))

            # Strip clusters were just produced above, but nothing forms strip
            # space points on the device path — mirror the host-side flow
            if self.flags.Tracking.ActiveConfig.useITkStripSeeding or (self.flags.Acts.SpacePoints.doStrip and not self.flags.Tracking.ActiveConfig.isSecondaryPass):
                from ActsConfig.ActsSpacePointFormationConfig import ActsStripSpacePointFormationAlgCfg
                ca.merge(ActsStripSpacePointFormationAlgCfg(self.flags,
                    name=f"StripSpacePointFormationAlg_{self.signature}",
                    StripClusters="ITkStripClusters",
                    StripSpacePoints="ITkStripSpacePoints",
                    StripOverlapSpacePoints="ITkStripOverlapSpacePoints"))
        
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
        ca.merge(ActsTrackingGeometrySvcCfg(self.flags))

        ca.merge(TracccTrackConverterAlgCfg(self.flags,
            name=f"TracccTrackConverterAlg_{self.signature}",
            InputPixelClusters="ITkPixelClusters",
            InputStripClusters="ITkStripClusters",
            InputMeasToPixelSP="ITkTracccMeasToPixelSP",
            InputMeasToStripCl="ITkTracccMeasToStripCl",
            InputTracks="TracccTrackCollection",
            OutputTracks=self.flags.Tracking.ActiveConfig.trkTracks_FTF,
        ))

        # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer
        from ActsConfig.ActsEventCnvConfig import ActsTrackToTrackParticleCnvAlgCfg
        ca.merge(ActsTrackToTrackParticleCnvAlgCfg(self.flags, name=f"ActsTracccTrackToTrackParticleCnvAlgSeqFTF_{self.signature}",
                                                ACTSTracksLocation=[self.flags.Tracking.ActiveConfig.trkTracks_FTF],
                                                TrackParticlesOutKey=self.flags.Tracking.ActiveConfig.tracks_FTF))

        
    #-----flags.Acts.Device.doTrackReconstruction =False ---
    else:
      
      # If clusterization was on device, need to copy the measurements to host first
      if clustersLocation is DataLocation.DEVICE:
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccMeasurementConverterAlgCfg
        ca.merge(TracccMeasurementConverterAlgCfg(self.flags,
            InputMeasurements="TracccMeasurementCollection",
            InputClusters="TracccClusterCollection",
            InputCells="TracccCellCollection",
            ConvertClustersWithCells = self.flags.Tracking.doTruth,
            OutputPixelClusters="ITkPixelClusters",
            OutputPixelSpacePoints="ITkPixelSpacePoints",
            OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
            OutputMeasToStripCl="ITkTracccMeasToStripCl",
            OutputStripClusters="ITkStripClusters"
        ))
        from ActsConfig.ActsSpacePointFormationConfig import ActsStripSpacePointFormationAlgCfg
        ca.merge(ActsStripSpacePointFormationAlgCfg(self.flags,
            name=f"{self.flags.Tracking.ActiveConfig.extension}StripSpacePointFormationAlg",
            StripClusters="ITkStripClusters",
            StripSpacePoints="ITkStripSpacePoints",
            StripOverlapSpacePoints="ITkStripOverlapSpacePoints"))
        clustersLocation = DataLocation.HOST

      if seedsLocation is DataLocation.DEVICE:
        from ActsGPUEventCnv.ActsGPUEventCnvConfig import TracccSeedConverterAlgCfg
        
        ca.merge(TracccSeedConverterAlgCfg(self.flags,
            name="TracccSeedConverterAlg",
            InputSpacepointsDevice="TracccPixelSpacepointCollection",
            InputSpacepoints="ITkPixelSpacePoints",
            InputMeasToPixelSP="ITkTracccMeasToPixelSP",
            InputSeeds="TracccPixelSeedCollection",
            OutputSeeds=f'{self.flags.Tracking.ActiveConfig.extension}PixelSeeds'))
        seedsLocation = DataLocation.HOST
                      
      # CKF
      from ActsConfig.ActsTrackFindingConfig import ActsTrackFindingCfg
      ca.merge(ActsTrackFindingCfg(self.flags))

      # Ambiguity Resolution
      if self.flags.Acts.doAmbiguityResolution:
          from ActsConfig.ActsTrackFindingConfig import ActsAmbiguityResolutionCfg
          ca.merge(ActsAmbiguityResolutionCfg(self.flags))

    return ca

  def g200_precision(self) -> ComponentAccumulator:
    ca = ComponentAccumulator()
    if self.inView:

      verifier = CompFactory.AthViews.ViewDataVerifier( name = 'VDVInDetPrecision'+self.signature,
                                                    DataObjects= {('InDet::SiDetectorElementStatus', 'StoreGateSvc+ITkPixelDetectorElementStatus'),
                                                                  ('InDet::SiDetectorElementStatus', 'StoreGateSvc+ITkStripDetectorElementStatus'),
                                                                  ('xAOD::StripClusterContainer' , 'StoreGateSvc+ITkStripClusters' ),
                                                                  ('xAOD::PixelClusterContainer' , 'StoreGateSvc+ITkPixelClusters' ),
                                                                  ('ActsTrk::GeometryContext' , 'StoreGateSvc+ActsAlignment' ),
                                                                })

      ca.addEventAlgo(verifier)

    # Convert Acts tracks to xAOD TrackParticles for ID trigger, since there is no "precision tracking" so far
    from ActsConfig.ActsEventCnvConfig import ActsTrackToTrackParticleCnvAlgCfg
    ca.merge(ActsTrackToTrackParticleCnvAlgCfg(self.flags, name=f"ActsTracccTrackToTrackParticleCnvAlgSeqIDtrig{self.signature}",
                                                  ACTSTracksLocation=[self.flags.Tracking.ActiveConfig.trkTracks_FTF],
                                                  TrackParticlesOutKey=self.flags.Tracking.ActiveConfig.tracks_IDTrig) )

    return ca


  def f100(self) -> ComponentAccumulator:
    import EFTrackConfig.F100Config as F100Config
    from TrigInDetConfig.ActsTrigSequence import ActsTrigSequence

    ca = ComponentAccumulator()

    seq = ActsTrigSequence(self.flags, self.signature, self.rois, self.inView)

    ca.merge(F100Config.dataPreparation(self.flags, self.signature, self.inView, self.rois))
    ca.merge(seq.fastTrackFinder())

    return ca
  
  def f100_precision(self) -> ComponentAccumulator:
    from TrigInDetConfig.ActsTrigSequence import ActsTrigSequence

    ca = ComponentAccumulator()

    seq = ActsTrigSequence(self.flags, self.signature, self.rois, self.inView)

    #this does not require new data preparation of its own
    ca.merge(seq.sequenceAfterPattern())

    return ca

  def viewDataVerifierGPU(self, viewVerifier='IDViewDataVerifier', extra_data_objects=None) -> ComponentAccumulator:
    
    from AthenaConfiguration.ComponentFactory import CompFactory
    acc = ComponentAccumulator()
     
    viewDataVerifier = CompFactory.AthViews.ViewDataVerifier("TrackingRDOViewDataVerifier")
          
    # Base data objects that are always needed
    base_data_objects = [
      # Add Acts geometry context required by Acts-based algorithms
      ('ActsGeometryContext', 'StoreGateSvc+ActsAlignment'),
      ('ActsTrk::GeometryContext' , 'StoreGateSvc+ActsAlignment'),
      # RDO containers
      ('PixelRDO_Container', 'StoreGateSvc+ITkPixelRDOs'),
      ('SCT_RDO_Container', 'StoreGateSvc+ITkStripRDOs'),
      ('PixelRDO_Cache', 'PixRDOCache'),
      ('SCT_RDO_Cache', 'SctRDOCache'),
      ('InDet::SiDetectorElementStatus', 'StoreGateSvc+ITkPixelDetectorElementStatus'),
      ('InDet::SiDetectorElementStatus', 'StoreGateSvc+ITkStripDetectorElementStatus'),
      ('TrigRoiDescriptorCollection', str(self.rois)),
      ('xAOD::EventInfo', 'StoreGateSvc+EventInfo'),
      ('xAOD::EventInfo', 'EventInfo'),
      ('TagInfo', 'DetectorStore+ProcessingTags' ),
      ('xAOD::TruthEventContainer', 'StoreGateSvc+TruthEvents'),
      ('xAOD::TruthParticleContainer' , 'StoreGateSvc+TruthParticles' ),
      ('McEventCollection' , 'StoreGateSvc+TruthEvent' ),
    ]
    
    # Add SGInputLoader 
    if self.flags.Input.isMC:
      self.log.info(f"Added SGInputLoader for view '{self.inView}' (MC data)")
      from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
      sgil_load = [('PixelRDO_Container', 'StoreGateSvc+ITkPixelRDOs'),
                  ('SCT_RDO_Container', 'StoreGateSvc+ITkStripRDOs'),
                  ('InDetSimDataCollection', 'ITkPixelSDO_Map'),
                  ('InDetSimDataCollection' , 'StoreGateSvc+ITkStripSDO_Map'),
                  ( 'McEventCollection' , 'StoreGateSvc+TruthEvent' )]
      acc.merge(SGInputLoaderCfg(self.flags, Load=sgil_load))

    if doTruth:
      base_data_objects.extend([
        ('InDetSimDataCollection', 'StoreGateSvc+ITkPixelSDO_Map'),
        ('InDetSimDataCollection', 'StoreGateSvc+ITkStripSDO_Map'),
      ])
      
    if extra_data_objects:
      base_data_objects.extend(extra_data_objects)
    
    viewDataVerifier.DataObjects = base_data_objects
    viewDataVerifier.name= viewVerifier + "_" + self.signature    
    acc.addEventAlgo(viewDataVerifier, primary=True)
      
    return acc

  def trackFinding(self) -> ComponentAccumulator:
    ca = ComponentAccumulator()

    pipeline = str(self.flags.Trigger.EFTrackPipeline).upper()

    match pipeline:
      case "F100":
        ca.merge(self.f100())
        pass 
      case "G200":
        ca.merge(self.g200())
        pass
      case _:
        pass


    return ca

  def sequenceAfterPattern(self, rois="") -> ComponentAccumulator:
    """ this method is used by the menu to generate precision tracking step.
        for AccelTrackTrigSequence it should just make tracks from pattern available again
    """
    ca = ComponentAccumulator()

    pipeline = str(self.flags.Trigger.EFTrackPipeline).upper()

    match pipeline:
      case "F100":
        ca.merge(self.f100_precision())
        pass 
      case "G200":
        ca.merge(self.g200_precision())
        pass
      case _:
        pass
            
    return ca

  def viewDataVerifierFPGA(self, viewVerifier='IDViewDataVerifier') -> ComponentAccumulator:

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
                        ('xAOD::EventInfo', 'EventInfo'),
                        ('ActsTrk::GeometryContext' , 'StoreGateSvc+ActsAlignment' ),
                        ('TrigRoiDescriptorCollection', str(self.rois)),
                        ( 'TagInfo' , 'DetectorStore+ProcessingTags' )} )

      if self.flags.Input.isMC:
          ViewDataVerifier.DataObjects |= {( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                                           ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                                           ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map'),
                        ('ActsTrk::GeometryContext' , 'StoreGateSvc+ActsAlignment' )}
          from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
          sgil_load = [( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                      ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                      ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map'),]
          acc.merge(SGInputLoaderCfg(self.flags, Load=sgil_load))

      ViewDataVerifier.DataObjects |= {
        ('InDet::SiDetectorElementStatus' ,   'StoreGateSvc+ITkPixelDetectorElementStatus' ),
        ('InDet::SiDetectorElementStatus' , 'StoreGateSvc+ITkStripDetectorElementStatus' ),
      }

      acc.addEventAlgo(ViewDataVerifier)
      return acc
    
