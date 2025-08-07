#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

def fastTracking(inflags, RoIs, variant=''):
    acc = ComponentAccumulator()
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence
    log = logging.getLogger("trigElectron"+variant+"InDetFastTrackingCfg")
    signatureName = 'electronLRT' if 'LRT' in variant  else 'electron'
    flags = getFlagsForActiveConfig(inflags, signatureName, log)

    seq = InnerTrackingTrigSequence.create(flags, 
                                       flags.Tracking.ActiveConfig.input_name, 
                                       rois = RoIs, 
                                       inView = "fastTracking"+variant+'VDV')
    
    acc = seq.sequence("FastTrackFinder")
      
    return acc, flags

