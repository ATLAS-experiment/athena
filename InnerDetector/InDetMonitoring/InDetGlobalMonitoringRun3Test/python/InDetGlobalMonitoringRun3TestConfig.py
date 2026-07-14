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

    # Phase II / ITk and Run 3 share the same monitoring entry points.
    # Historically the Track/LRT algorithms only ran for the
    # ('online','tier0','tier0Raw') DQ environments, which excludes
    # the typical RDO->ESD/AOD MC reconstruction (DQ.Environment ==
    # 'tier0ESD' or 'AOD').  For the Run4/ITk migration we relax this
    # gate so that the monitoring is also configured for the offline
    # MC reconstruction path used to validate the framework.
    _trackEnvs = ('online', 'tier0', 'tier0Raw', 'tier0ESD', 'AOD')

    # Track monitoring (uses xAOD::TrackParticle, ITk-friendly)
    if flags.DQ.Environment in _trackEnvs:
        from InDetGlobalMonitoringRun3Test.InDetGlobalTrackMonAlgCfg import (
            InDetGlobalTrackMonAlgCfg)
        InDetGlobalTrackMonAlgCfg(helper, acc, flags)

    # Large radius tracking monitoring
    if (flags.DQ.Environment in _trackEnvs and
        (flags.Tracking.doLargeD0 or flags.Tracking.doLowPtLargeD0)):
        from InDetGlobalMonitoringRun3Test.InDetGlobalLRTMonAlgCfg import (
            InDetGlobalLRTMonAlgCfg)
        InDetGlobalLRTMonAlgCfg(helper, acc, flags)

    # Primary vertex / beamspot monitoring (ESD-level)
    if flags.DQ.Environment != 'tier0Raw':
        from InDetGlobalMonitoringRun3Test.InDetGlobalPrimaryVertexMonAlgCfg import (
            InDetGlobalPrimaryVertexMonAlgCfg )
        InDetGlobalPrimaryVertexMonAlgCfg(helper, acc, flags)
       
        from InDetGlobalMonitoringRun3Test.InDetGlobalBeamSpotMonAlgCfg import (
            InDetGlobalBeamSpotMonAlgCfg )
        InDetGlobalBeamSpotMonAlgCfg(helper, acc, flags)
        
    acc.merge(helper.result())
    return acc
