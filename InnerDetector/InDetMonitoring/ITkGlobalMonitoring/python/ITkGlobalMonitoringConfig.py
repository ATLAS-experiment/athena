#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

####################################################
#                                                  #
# ITkGlobalMonitoring top configuration            #
#                                                  #
####################################################



def ITkGlobalMonitoringConfig(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, "ITkGlobalMonitoring")

    # In addition to the Run 3 tier0 environments, also run in the
    # offline MC reconstruction path (DQ.Environment == 'tier0ESD' or
    # 'AOD') used to develop and validate the ITk monitoring, since no
    # ITk data exists yet.
    _trackEnvs = ('online', 'tier0', 'tier0Raw', 'tier0ESD', 'AOD')

    # Track monitoring (uses xAOD::TrackParticle, ITk-friendly)
    if flags.DQ.Environment in _trackEnvs:
        from ITkGlobalMonitoring.ITkGlobalTrackMonAlgCfg import (
            ITkGlobalTrackMonAlgCfg)
        ITkGlobalTrackMonAlgCfg(helper, acc, flags)

    # Large radius tracking monitoring
    if (flags.DQ.Environment in _trackEnvs and
        (flags.Tracking.doLargeD0 or flags.Tracking.doLowPtLargeD0)):
        from ITkGlobalMonitoring.ITkGlobalLRTMonAlgCfg import (
            ITkGlobalLRTMonAlgCfg)
        ITkGlobalLRTMonAlgCfg(helper, acc, flags)

    # Primary vertex / beamspot monitoring (ESD-level)
    if flags.DQ.Environment != 'tier0Raw':
        from ITkGlobalMonitoring.ITkGlobalPrimaryVertexMonAlgCfg import (
            ITkGlobalPrimaryVertexMonAlgCfg )
        ITkGlobalPrimaryVertexMonAlgCfg(helper, acc, flags)
       
        from ITkGlobalMonitoring.ITkGlobalBeamSpotMonAlgCfg import (
            ITkGlobalBeamSpotMonAlgCfg )
        ITkGlobalBeamSpotMonAlgCfg(helper, acc, flags)
        
    acc.merge(helper.result())
    return acc
