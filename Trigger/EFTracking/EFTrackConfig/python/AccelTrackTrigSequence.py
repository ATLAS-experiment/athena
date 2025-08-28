#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory

class AccelTrackTrigSequence(InnerTrackerTrigSequence):
  def __init__(self, flags : AthConfigFlags, signature : str, rois : str, inView : str):
    super().__init__(flags, signature,rois,inView)
    self.log = logging.getLogger("AccelTrigTrigSequence")
    self.log.info(f"EFTracking signature: {self.signature} rois: {self.rois} inview: {self.inView}")
  def sequence(self, recoType : str) -> ComponentAccumulator:
    """ main method to instantiate tracking in the trigger menu
        recoType can be used to generate a specific step. The pattern recognition
        and production of xAOD trackparticles should happen in FastTrackFinder step 
    """
    ca = ComponentAccumulator()
      
    if self.inView:
      #to invoke VDV to make sure RoI and other info is available in view
      ca.merge(self.viewDataVerifier())

    #if there is need for splitting of the sequence in steps it can be added here
    if recoType =="FastTrackFinder":
      ca.merge(self.trackFinding())

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

  def trackFinding(self) -> ComponentAccumulator:
    ca = ComponentAccumulator()

    self.log.info(f'merge EFTrackPipeline {self.flags.Trigger.EFTrackPipeline}')

    match self.flags.Trigger.EFTrackPipeline:
      case "F100":
        ca.merge(self.f100())
        pass 
      case "G200":
        pass
    
      #other pipelines can be added here
      
      case _:
        pass
            
    return ca

  def sequenceAfterPattern(self, rois="") -> ComponentAccumulator:
    """ this method is used by the menu to generate precision tracking step.
        for AccelTrackTrigSequence it should just make tracks from pattern available again
    """
    ca = ComponentAccumulator()

    match self.flags.Trigger.EFTrackPipeline:
      case "F100":
        ca.merge(self.f100_precision())
        pass 
      case "G200":
        pass
    
      #other pipelines can be added here
      
      case _:
        pass
            
    return ca

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
                        ('xAOD::EventInfo', 'EventInfo'),
                        ( 'ActsGeometryContext' , 'StoreGateSvc+ActsAlignment' ),
                        ('TrigRoiDescriptorCollection', str(self.rois)),
                        ( 'TagInfo' , 'DetectorStore+ProcessingTags' )} )

      if self.flags.Input.isMC:
          ViewDataVerifier.DataObjects |= {( 'PixelRDO_Container' , 'StoreGateSvc+ITkPixelRDOs' ),
                                           ( 'SCT_RDO_Container' , 'StoreGateSvc+ITkStripRDOs' ),
                                           ( 'InDetSimDataCollection' , 'ITkPixelSDO_Map'),
                        ( 'ActsGeometryContext' , 'StoreGateSvc+ActsAlignment' )}
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
    
