#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

####################################################
#                                                  #
# InDetAlignmentManager top algorithm              #
#                                                  #
####################################################

def InDetAlignmentMonitoringRun3Config(flags, TrackCollectionName = None):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, "InDetAlignmentMonitoringRun3")

    from AthenaConfiguration.ComponentFactory import CompFactory
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (Align_InDetTrackSelectionToolCfg)

    from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg
    from AthenaConfiguration.Enums import BeamType

    # Pick the appropriate Trk::Track collection name for the geometry.
    # Run 1-3 InnerDetector reco produces "ExtendedTracks"; ITk reco
    # (Run 4) produces "CombinedITkTracks".
    if TrackCollectionName is None:
        TrackCollectionName = ("CombinedITkTracks"
                               if flags.Detector.GeometryITk
                               else "ExtendedTracks")

    # Run 4/ITk migration: relax the DQ environment gate so the
    # alignment monitoring also runs in the offline MC reconstruction
    # path (DQ.Environment == 'tier0ESD' or 'AOD').
    _envs = ('online', 'tier0', 'tier0Raw', 'tier0ESD', 'AOD')
    if flags.DQ.Environment in _envs:

        ########### here begins InDetAlignMonGenericTracksAlg ###########
        kwargsIDAlignMonGenericTracksAlg = {
            'vxPrimContainerName' : 'PrimaryVertices', #InDetKeys.xAODVertexContainer(),
            'TrackName'  : TrackCollectionName,
            'TrackName2' : TrackCollectionName,
        }

        from InDetAlignmentMonitoringRun3.IDAlignMonGenericTracksAlgCfg import IDAlignMonGenericTracksAlgCfg
        inDetAlignMonGenericTracksAlg = helper.addAlgorithm(CompFactory.IDAlignMonGenericTracksAlg, 'IDAlignMonGenericTracksAlg'+'_'+kwargsIDAlignMonGenericTracksAlg["TrackName"],
                                                            useExtendedPlots    = True,
                                                            ApplyTrackSelection = False)
        for k, v in kwargsIDAlignMonGenericTracksAlg.items():
            setattr(inDetAlignMonGenericTracksAlg, k, v)

        # Disable TRT lookup on ITk geometry: there is no TRT_ID in detStore
        # and any access would fail.  The C++ algorithm respects the DoTRT
        # boolean and skips all TRT-related ID/hit handling when false.
        if flags.Detector.GeometryITk:
            inDetAlignMonGenericTracksAlg.DoTRT = False

        inDetAlignMonGenericTracksAlg.TrackSelectionTool = acc.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))

        IDAlignMonGenericTracksAlgCfg(helper, inDetAlignMonGenericTracksAlg, flags=flags, **kwargsIDAlignMonGenericTracksAlg)

        ########### here ends InDetAlignMonGenericTracksAlg ###########


        ########### here starts InDetAlignMonResidualsAlgs ###########
        # The residual monitoring is tightly coupled to the Run 1-3 ID
        # detector managers (PixelDetectorManager, SCT_DetectorManager),
        # to TRT calibration tools and to InDetAlignGenTools that have
        # not yet been migrated to ITk.  Skip this algorithm on ITk
        # geometry until a dedicated ITk port is provided.
        if flags.Detector.GeometryID and not flags.Detector.GeometryITk:
            kwargsIDAlignMonResidualsAlg = {
                'TrackName'  : TrackCollectionName,
                'TrackName2' : TrackCollectionName,
            }

            from InDetAlignmentMonitoringRun3.IDAlignMonResidualsAlgCfg import IDAlignMonResidualsAlgCfg
            inDetAlignMonResidualsAlg = helper.addAlgorithm(CompFactory.IDAlignMonResidualsAlg, 'IDAlignMonResidualsAlg',
                                                            addFilterTools = [FilledBunchFilterToolCfg(flags)])

            for k, v in kwargsIDAlignMonResidualsAlg.items():
                setattr(inDetAlignMonResidualsAlg, k, v)

            inDetAlignMonResidualsAlg.TrackSelectionTool = acc.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))

            IDAlignMonResidualsAlgCfg(helper, inDetAlignMonResidualsAlg, **kwargsIDAlignMonResidualsAlg)
        ########### here ends InDetAlignMonResidualsAlg ###########

        ########### here starts InDetAlignPVBiasesAlg ###########
        if flags.Beam.Type is not BeamType.Cosmics:
            kwargsIDAlignMonPVBiasesAlg = { 
                'vxContainerName' : 'PrimaryVertices',
            }
        
            from InDetAlignmentMonitoringRun3.IDAlignMonPVBiasesAlgCfg import IDAlignMonPVBiasesAlgCfg
            inDetAlignMonPVBiasesAlg = helper.addAlgorithm(CompFactory.IDAlignMonPVBiasesAlg, 'IDAlignMonPVBiasesAlg',
                                                           addFilterTools = [FilledBunchFilterToolCfg(flags)])

            from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
            TrackToVertexIPEstimator = acc.popToolsAndMerge(
                TrackToVertexIPEstimatorCfg(flags, name='TrackToVertexIPEstimator'))

            inDetAlignMonPVBiasesAlg.TrackToVertexIPEstimator = TrackToVertexIPEstimator

            IDAlignMonPVBiasesAlgCfg(helper, inDetAlignMonPVBiasesAlg,
                                     TrackCollectionName=TrackCollectionName)

        ########### here ends InDetAlignPVBiasesAlg ###########

    acc.merge(helper.result())

    return acc
#
