#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.Enums import BeamType

def PixelMonitoringConfig(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    forceOnline = False # for testing of online monitoring and 100LB histograms
    doHitMonAlg       = True
    doClusterMonAlg   = True
    doErrorMonAlg     = True
    doMVAMonAlg       = True #allowed only if online, see below

    # run on RAW only
    if flags.DQ.Environment in ('online', 'tier0', 'tier0Raw'):
        if forceOnline: 
            isOnline = True
        else:
            isOnline = flags.Common.isOnline
        if not isOnline: doMVAMonAlg=False
        kwargsHitMonAlg = { 'doOnline'        : isOnline,      #Histograms for online (GlobalMonitoring) running
                            'doLumiBlock'     : not isOnline,  #Turn on/off histograms stored every 1(20) lumi block(s)
                            'doFEPlots'       : True,                       #Turn on/off per FE-I3 histograms
                            'RDOName'         : 'PixelRDOs',
        }

        kwargsClusMonAlg = { 'doOnline'        : isOnline,      #Histograms for online (GlobalMonitoring) running
                             'doLumiBlock'     : not isOnline,  #Turn on/off histograms stored every 1(20) lumi block(s)
                             'doLowOccupancy'  : flags.Beam.Type is BeamType.Cosmics, #Setting up 1D histogram ranges and binnings, if False, high occupancy i.e. collisions settings will be used
                             'doHeavyIonMon'   : flags.Reco.EnableHI,     #Setting up 1D histogram ranges and binnings for heavy ions
                             'doFEPlots'       : True,                       #Turn on/off per FE-I3 histograms
                             'ClusterName'     : 'PixelClusters',
                             'TrackName'       : 'CombinedInDetTracks',
        }

        kwargsErrMonAlg = { 'doOnline'        : isOnline,        #Histograms for online (GlobalMonitoring) running
                            'doLumiBlock'     : not isOnline     #Turn on/off histograms stored every 1(20) lumi block(s)
        }

        kwargsMVAMonAlg = { 'calibFolder'     : '20220503',
                            'RDOName'         : 'PixelRDOs',
                            'ClusterName'     : 'PixelClusters',
                            'TrackParticleContainerName' : 'InDetTrackParticles',
        }

        if doHitMonAlg or doClusterMonAlg or doErrorMonAlg or doMVAMonAlg:
            from PixelConditionsAlgorithms.PixelConditionsConfig import PixelDetectorElementStatusAlgActiveOnlyCfg
            acc.merge(PixelDetectorElementStatusAlgActiveOnlyCfg(flags))

        from AthenaMonitoring import AthMonitorCfgHelper
        helper = AthMonitorCfgHelper(flags, "NewPixelMonitoring")

        from AthenaConfiguration.ComponentFactory import CompFactory
        from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg

        if doHitMonAlg:
            from PixelMonitoring.PixelAthHitMonAlgCfg import PixelAthHitMonAlgCfg
            pixelAthHitMonAlg = helper.addAlgorithm(CompFactory.PixelAthHitMonAlg, 'PixelAthHitMonAlg', addFilterTools = [FilledBunchFilterToolCfg(flags)])
            kwargsHitMonAlg.setdefault(  'PixelDetElStatus',           'PixelDetectorElementStatus' )
            kwargsHitMonAlg.setdefault(  'PixelDetElStatusActiveOnly', 'PixelDetectorElementStatusActiveOnly')
            for k, v in kwargsHitMonAlg.items():
                setattr(pixelAthHitMonAlg, k, v)
            PixelAthHitMonAlgCfg(helper, pixelAthHitMonAlg, **kwargsHitMonAlg)

        if doClusterMonAlg:
            from PixelMonitoring.PixelAthClusterMonAlgCfg import PixelAthClusterMonAlgCfg
            pixelAthClusterMonAlg = helper.addAlgorithm(CompFactory.PixelAthClusterMonAlg, 'PixelAthClusterMonAlg', addFilterTools = [FilledBunchFilterToolCfg(flags)])
            kwargsClusMonAlg.setdefault(  'PixelDetElStatus',           'PixelDetectorElementStatus' )
            kwargsClusMonAlg.setdefault(  'PixelDetElStatusActiveOnly', 'PixelDetectorElementStatusActiveOnly')
            for k, v in kwargsClusMonAlg.items():
                setattr(pixelAthClusterMonAlg, k, v)

            from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
                InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg)
            TrackSelectionTool = acc.popToolsAndMerge(
                InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(
                    flags, maxNPixelHoles = 2)) # Default for TightPrimary is 0
            if flags.Beam.Type is not BeamType.Cosmics:
                TrackSelectionTool.maxD0            = 2
                TrackSelectionTool.maxZ0            = 150

            pixelAthClusterMonAlg.TrackSelectionTool = TrackSelectionTool

            from InDetConfig.InDetTrackHoleSearchConfig import (
                InDetTrackHoleSearchToolCfg)
            pixelAthClusterMonAlg.HoleSearchTool = acc.popToolsAndMerge(
                InDetTrackHoleSearchToolCfg(flags))

            from InDetConfig.InDetTestPixelLayerConfig import InDetTestPixelLayerToolInnerCfg
            pixelAthClusterMonAlg.TestPixelLayerTool = acc.popToolsAndMerge (InDetTestPixelLayerToolInnerCfg (flags,'TestPixelLayerTool'))

            PixelAthClusterMonAlgCfg(helper, pixelAthClusterMonAlg, **kwargsClusMonAlg)

        if doErrorMonAlg:
            from PixelMonitoring.PixelAthErrorMonAlgCfg import PixelAthErrorMonAlgCfg
            pixelAthMonAlgErrorMonAlg = helper.addAlgorithm(CompFactory.PixelAthErrorMonAlg, 'PixelAthErrorMonAlg', addFilterTools = [FilledBunchFilterToolCfg(flags)])
            kwargsErrMonAlg.setdefault(  'PixelDetElStatusActiveOnly', 'PixelDetectorElementStatusActiveOnly')
            kwargsErrMonAlg.setdefault(  'PixelByteStreamErrs', 'PixelByteStreamErrs')
            kwargsErrMonAlg.setdefault(  'UseByteStreamFEI4', not flags.Input.isMC)
            kwargsErrMonAlg.setdefault(  'UseByteStreamFEI3', not flags.Input.isMC)
            kwargsErrMonAlg.setdefault(  'UseByteStreamRD53', False)
            for k, v in kwargsErrMonAlg.items():
                setattr(pixelAthMonAlgErrorMonAlg, k, v)
            PixelAthErrorMonAlgCfg(helper, pixelAthMonAlgErrorMonAlg, **kwargsErrMonAlg)

        if doMVAMonAlg:
            from PixelMonitoring.PixelAthMVAMonAlgCfg import PixelAthMVAMonAlgCfg
            pixelAthMVAMonAlg = helper.addAlgorithm(CompFactory.PixelAthMVAMonAlg, 'PixelAthMVAMonAlg', addFilterTools = [FilledBunchFilterToolCfg(flags)])
            kwargsMVAMonAlg.setdefault(  'PixelDetElStatus',           'PixelDetectorElementStatus' )
            kwargsMVAMonAlg.setdefault(  'PixelDetElStatusActiveOnly', 'PixelDetectorElementStatusActiveOnly')
            for k, v in kwargsMVAMonAlg.items():
                setattr(pixelAthMVAMonAlg, k, v)

            from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
                InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg)
            TrackSelectionTool = acc.popToolsAndMerge(
                InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(flags,
                                                                    maxNPixelHoles
= 1)) # Default for TightPrimary is 0
            if flags.Beam.Type is not BeamType.Cosmics:
                TrackSelectionTool.maxD0            = 2
                TrackSelectionTool.maxZ0            = 150

            from InDetConfig.InDetTrackHoleSearchConfig import InDetTrackHoleSearchToolCfg
            pixelAthMVAMonAlg.TrackSelectionTool = TrackSelectionTool
            pixelAthMVAMonAlg.HoleSearchTool     = acc.popToolsAndMerge(InDetTrackHoleSearchToolCfg(flags))
            from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
            pixelAthMVAMonAlg.Extrapolator       = acc.popToolsAndMerge(InDetExtrapolatorCfg(flags))
            PixelAthMVAMonAlgCfg(helper, pixelAthMVAMonAlg, **kwargsMVAMonAlg)

        acc.merge(helper.result())

    return acc
   
