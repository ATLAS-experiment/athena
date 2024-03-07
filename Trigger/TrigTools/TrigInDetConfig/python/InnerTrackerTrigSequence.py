#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

class InnerTrackerTrigSequence(object):
  def __init__(self, flags : AthConfigFlags, signature : str, rois : str, inView : str):
      
    self.flags = flags
    self.signature = signature
    self.rois = rois
    self.lastRois = rois
    self.lastTrkCollection = self.flags.Tracking.ActiveConfig.trkTracks_FTF
    self.inView = inView
    self.ambiPrefix = "TrigAmbi"
    self.log = logging.getLogger("InnerTrackerTrigSequence")
    
  def sequence(self, recoType : str) -> ComponentAccumulator:

    ca = ComponentAccumulator()
    
    if self.inView:
      ca.merge(self.viewDataVerifier(self.inView))

    if recoType == "dataPreparation":
      ca.merge(self.dataPreparation())
      return ca

    if recoType =="spacePointFormation":
      ca.merge(self.dataPreparation())
      ca.merge(self.spacePointFormation())
      return ca

    if recoType =="FastTrackFinder":
      ca.merge(self.dataPreparation())
      ca.merge(self.spacePointFormation())
      ca.merge(self.fastTrackFinder())
      return ca

    if recoType =="Offline":
      ca.merge(self.dataPreparation())
      ca.merge(self.spacePointFormation())
      ca.merge(self.offlinePattern())
      ca.merge(self.sequenceAfterPattern())

    if recoType =="OfflineNoDataPrep":
      ca.merge(self.viewDataVerifierAfterDataPrep())
      ca.merge(self.offlinePattern())
      ca.merge(self.sequenceAfterPattern())

    return ca

  def fastTrackFinderBase(self, 
                          extraFlags : AthConfigFlags = None, 
                          inputTracksName : str = None) -> ComponentAccumulator:
    """
    return ComponentAccumulator of the FTF instance
    if another instance of flags is passed in this is for a second instance of FTF
    if inputTracksName is specified it is also a second instance but is invoked as first? what about the previous steps? TODO
    """

    acc = ComponentAccumulator()

    ftfargs = {}
    flags = self.flags

    signature = flags.Tracking.ActiveConfig.input_name

    if extraFlags:
      flags = extraFlags
      ftfargs["inputTracksName"] = self.flags.Tracking.ActiveConfig.trkTracks_FTF
      #TODO move from .name to .input_name for consistency after migration to private tools
      signature = flags.Tracking.ActiveConfig.name
    elif inputTracksName:
      ftfargs["inputTracksName"] = inputTracksName

    from TrigFastTrackFinder.TrigFastTrackFinderConfig import TrigFastTrackFinderCfg
    acc.merge(TrigFastTrackFinderCfg(flags, "TrigFastTrackFinder_" + signature,
                                     self.rois, **ftfargs))
    return acc

  
  
  def sequenceAfterPattern(self, recoType : str = "PrecisionTracking", rois : str = "") -> ComponentAccumulator:

    ca = ComponentAccumulator()

    if rois:
      self.lastRois = rois
      if self.lastRois != self.rois:
        self.log.info(f"Sequence after patternReco for signature: {self.signature} RoIs: {self.rois} inview: {self.inView} with new RoIs {self.lastRois} - they must be a subvolume.")

    ca.merge(self.ambiguitySolver())
    if self.flags.Tracking.ActiveConfig.doTRT:
        ca.merge(self.trtExtensions())
    ca.merge(self.xAODParticleCreation())

    return ca

