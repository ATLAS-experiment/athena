#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TrigInDetConfig.InnerTrackerTrigSequence import InnerTrackerTrigSequence
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

class InnerTrackingTrigSequence:

  def create(flags : AthConfigFlags, *args, **kwargs) -> InnerTrackerTrigSequence:
    if flags.Trigger.EFTrackPipeline:
      from EFTrackConfig.AccelTrackTrigSequence import AccelTrackTrigSequence as InnerTrackingSequence
    elif flags.Trigger.useActsTracking:
      from TrigInDetConfig.ActsTrigSequence import ActsTrigSequence as InnerTrackingSequence
    elif flags.Detector.GeometryITk:
      from TrigInDetConfig.ITkTrigSequence import ITkTrigSequence as InnerTrackingSequence
    else:
      from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence as InnerTrackingSequence

    return InnerTrackingSequence(flags, *args, **kwargs)
