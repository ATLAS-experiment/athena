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

    # The environment gate exists for the data Tier-0 processing,
    # which splits monitoring across the RAW and ESD steps.  MC
    # reconstruction is a single RAWtoALL step (DQ.Environment
    # resolves to 'tier0ESD' there) and is currently the only way to
    # run the ITk monitoring, so on MC run whenever InDet DQ is
    # enabled instead of relying on the environment value.
    _runTrackMon = (flags.Input.isMC
                    or flags.DQ.Environment in ('online', 'tier0', 'tier0Raw'))

    # Track monitoring (uses xAOD::TrackParticle)
    if _runTrackMon:
        from ITkGlobalMonitoring.ITkGlobalTrackMonAlgCfg import (
            ITkGlobalTrackMonAlgCfg)
        ITkGlobalTrackMonAlgCfg(helper, acc, flags)

    # Large radius tracking monitoring (the Acts-based ITk LRT flag
    # only exists on main; check the legacy flags too for 24.0)
    if (_runTrackMon and
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
