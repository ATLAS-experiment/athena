#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from AthenaCommon.Logging import logging

class AccelTrackTrigSequence(InnerTrackerTrigSequence):
  def __init__(self, flags : AthConfigFlags, signature : str, rois : str, inView : str):
    super().__init__(flags, signature,rois,inView)
    self.log = logging.getLogger("AccelTrigTrigSequence")
    self.log.info(f"signature: {self.signature} rois: {self.rois} inview: {self.inView}")
    
  def sequence(self, recoType : str) -> ComponentAccumulator:
    """ main method to instantiate tracking in the trigger menu
        recoType can be used to generate a specific step. The pattern recognition
        and production of xAOD trackparticles should happen in FastTrackFinder step 
    """
    
    ca = ComponentAccumulator()
      
    if self.inView:
      #to invoke VDV to make sure RoI and other info is available in view
      pass

    #if there is need for splitting of the sequence in steps it can be added here
    if recoType =="FastTrackFinder":
      ca.merge(self.trackFinding())

    return ca

  def trackFinding(self) -> ComponentAccumulator:
    ca = ComponentAccumulator()

    self.log.info(f'merge EFTrackPipeline {self.flags.Trigger.EFTrackPipeline}')

    match self.flags.Trigger.EFTrackPipeline:
      case "F100":
        #example which depends on XRT available
        #from EFTrackingFPGAPipeline.BenchmarkConfig import BenchmarkCfg 
        #ca.merge(BenchmarkCfg(self.flags))
        pass 
      case "G200":
        pass
    
      #other pipelines can be added here
      
      case _:
        pass
            
    return ca

  def sequenceAfterPattern(self) -> ComponentAccumulator:
    """ this method is used by the menu to generate precision tracking step.
        for AccelTrackTrigSequence it should just make tracks from pattern available again
    """
    pass
    
