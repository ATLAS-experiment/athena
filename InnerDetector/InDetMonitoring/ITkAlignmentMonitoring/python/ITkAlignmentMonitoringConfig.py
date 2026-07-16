#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

####################################################
#                                                  #
# ITkAlignmentMonitoring top configuration         #
#                                                  #
####################################################

def ITkAlignmentMonitoringConfig(flags, TrackCollectionName = None):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, "ITkAlignmentMonitoring")

    from AthenaConfiguration.ComponentFactory import CompFactory
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (Align_InDetTrackSelectionToolCfg)

    from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg
    from AthenaConfiguration.Enums import BeamType

    if TrackCollectionName is None:
        TrackCollectionName = "CombinedITkTracks"

    # In addition to the Run 3 tier0 environments, also run in the
    # offline MC reconstruction path (DQ.Environment == 'tier0ESD' or
    # 'AOD') used to develop and validate the ITk monitoring, since no
    # ITk data exists yet.
    _envs = ('online', 'tier0', 'tier0Raw', 'tier0ESD', 'AOD')
    if flags.DQ.Environment in _envs:

        ########### here begins ITkAlignMonGenericTracksAlg ###########
        kwargsITkAlignMonGenericTracksAlg = {
            'vxPrimContainerName' : 'PrimaryVertices',
            'TrackName'  : TrackCollectionName,
            'TrackName2' : TrackCollectionName,
        }

        from ITkAlignmentMonitoring.ITkAlignMonGenericTracksAlgCfg import ITkAlignMonGenericTracksAlgCfg
        itkAlignMonGenericTracksAlg = helper.addAlgorithm(CompFactory.ITkAlignMonGenericTracksAlg, 'ITkAlignMonGenericTracksAlg'+'_'+kwargsITkAlignMonGenericTracksAlg["TrackName"],
                                                          useExtendedPlots    = True,
                                                          # The Run 3 track selection is not yet usable on ITk tracks
                                                          ApplyTrackSelection = False)
        for k, v in kwargsITkAlignMonGenericTracksAlg.items():
            setattr(itkAlignMonGenericTracksAlg, k, v)

        itkAlignMonGenericTracksAlg.TrackSelectionTool = acc.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))

        ITkAlignMonGenericTracksAlgCfg(helper, itkAlignMonGenericTracksAlg, flags=flags, **kwargsITkAlignMonGenericTracksAlg)

        ########### here ends ITkAlignMonGenericTracksAlg ###########

        ########### here starts ITkAlignMonResidualsAlg ###########
        kwargsITkAlignMonResidualsAlg = {
            'TrackName'  : TrackCollectionName,
            'TrackName2' : TrackCollectionName,
        }

        from ITkAlignmentMonitoring.ITkAlignMonResidualsAlgCfg import ITkAlignMonResidualsAlgCfg
        itkAlignMonResidualsAlg = helper.addAlgorithm(CompFactory.ITkAlignMonResidualsAlg, 'ITkAlignMonResidualsAlg',
                                                      addFilterTools = [FilledBunchFilterToolCfg(flags)])

        for k, v in kwargsITkAlignMonResidualsAlg.items():
            setattr(itkAlignMonResidualsAlg, k, v)

        itkAlignMonResidualsAlg.TrackSelectionTool = acc.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))

        # The Run 3 track selection is not yet usable on ITk tracks
        itkAlignMonResidualsAlg.ApplyTrackSelection = False

        ITkAlignMonResidualsAlgCfg(helper, itkAlignMonResidualsAlg, flags=flags, **kwargsITkAlignMonResidualsAlg)
        ########### here ends ITkAlignMonResidualsAlg ###########

        ########### here starts ITkAlignMonPVBiasesAlg ###########
        if flags.Beam.Type is not BeamType.Cosmics:
            from ITkAlignmentMonitoring.ITkAlignMonPVBiasesAlgCfg import ITkAlignMonPVBiasesAlgCfg
            itkAlignMonPVBiasesAlg = helper.addAlgorithm(CompFactory.ITkAlignMonPVBiasesAlg, 'ITkAlignMonPVBiasesAlg',
                                                         addFilterTools = [FilledBunchFilterToolCfg(flags)])
            itkAlignMonPVBiasesAlg.vxContainerName = 'PrimaryVertices'

            from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
            TrackToVertexIPEstimator = acc.popToolsAndMerge(
                TrackToVertexIPEstimatorCfg(flags, name='TrackToVertexIPEstimator'))

            itkAlignMonPVBiasesAlg.TrackToVertexIPEstimator = TrackToVertexIPEstimator

            ITkAlignMonPVBiasesAlgCfg(helper, itkAlignMonPVBiasesAlg,
                                      TrackCollectionName=TrackCollectionName)
        ########### here ends ITkAlignMonPVBiasesAlg ###########

    acc.merge(helper.result())

    return acc
