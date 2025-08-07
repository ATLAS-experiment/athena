#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#


#logging
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

def precisionTracking(inflags, RoIs, ion=False, variant=''):

    signatureName = 'electronLRT' if 'LRT' in variant  else 'electron'
    from TrigInDetConfig.TrigInDetConfig import trigInDetPrecisionTrackingCfg
    
    return trigInDetPrecisionTrackingCfg(inflags, RoIs, signatureName, in_view = True)


   
