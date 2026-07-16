#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

####################################################
#                                                  #
# InDetGlobalManager top algorithm                 #
#                                                  #
####################################################



def InDetGlobalMonitoringRun3TestConfig(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, "InDetGlobalMonitoringRun3Test")

    # run on RAW only
    if flags.DQ.Environment in ('online', 'tier0', 'tier0Raw'):
        
        from InDetGlobalMonitoringRun3Test.InDetGlobalTrackMonAlgCfg import (
            InDetGlobalTrackMonAlgCfg)
        InDetGlobalTrackMonAlgCfg(helper, acc, flags)


    if (flags.DQ.Environment in ('online', 'tier0', 'tier0Raw') and
        (flags.Tracking.doLargeD0 or flags.Tracking.doLowPtLargeD0)):
        from InDetGlobalMonitoringRun3Test.InDetGlobalLRTMonAlgCfg import (
            InDetGlobalLRTMonAlgCfg)
        InDetGlobalLRTMonAlgCfg(helper, acc, flags)
        
    # run on ESD
    if flags.DQ.Environment != 'tier0Raw':
        from InDetGlobalMonitoringRun3Test.InDetGlobalPrimaryVertexMonAlgCfg import (
            InDetGlobalPrimaryVertexMonAlgCfg )
        InDetGlobalPrimaryVertexMonAlgCfg(helper, acc, flags)
       
        from InDetGlobalMonitoringRun3Test.InDetGlobalBeamSpotMonAlgCfg import (
            InDetGlobalBeamSpotMonAlgCfg )
        InDetGlobalBeamSpotMonAlgCfg(helper, acc, flags)
        
    acc.merge(helper.result())
    return acc
