#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

####################################################
#                                                  #
# InDetAlignmentManager top algorithm              #
#                                                  #
####################################################
def InDetAlignmentMonitoringRun3Config(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, "InDetAlignmentMonitoringRun3")
        
    from AthenaConfiguration.ComponentFactory import CompFactory
    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (Align_InDetTrackSelectionToolCfg)

    from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg
    from AthenaConfiguration.Enums import BeamType
    
    UseITkGeometry = kwargs.pop("UseITkGeometry", False)
    trackCollectionName = "ExtendedTracks"
    trackCollectionName2 = "NONE"
    if ("TrackName" in kwargs):
        trackCollectionName = kwargs["TrackName"]
        print ("TrackName!!!!!!!!!", trackCollectionName)
    if ("TrackName2" in kwargs):
        trackCollectionName2 = kwargs["TrackName2"]
    
    if ( flags.DQ.Environment in ('online', 'tier0', 'tier0Raw', 'tier0ESD') ):

        ########### here begins InDetAlignMonGenericTracksAlg ###########
        kwargsIDAlignMonGenericTracksAlg = {}
        kwargsIDAlignMonGenericTracksAlg.update({'vxPrimContainerName' : 'PrimaryVertices'}) #InDetKeys.xAODVertexContainer())
        kwargsIDAlignMonGenericTracksAlg.update({'TrackName'  : trackCollectionName} )
        kwargsIDAlignMonGenericTracksAlg.update({'UseITkGeometry': UseITkGeometry})
        if ("NONE" not in trackCollectionName2): kwargsIDAlignMonGenericTracksAlg.update({'TrackName2' : trackCollectionName2})
            
        from InDetAlignmentMonitoringRun3.IDAlignMonGenericTracksAlgCfg import IDAlignMonGenericTracksAlgCfg
        inDetAlignMonGenericTracksAlg = helper.addAlgorithm(CompFactory.IDAlignMonGenericTracksAlg, 'IDAlignMonGenericTracksAlg'+'_'+kwargsIDAlignMonGenericTracksAlg["TrackName"],
                                                            useExtendedPlots    = True,
                                                            ApplyTrackSelection = False)
        for k, v in kwargsIDAlignMonGenericTracksAlg.items():
            setattr(inDetAlignMonGenericTracksAlg, k, v)

        inDetAlignMonGenericTracksAlg.TrackSelectionTool = acc.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))

        IDAlignMonGenericTracksAlgCfg(helper, inDetAlignMonGenericTracksAlg, **kwargsIDAlignMonGenericTracksAlg)
        
        ########### here ends InDetAlignMonGenericTracksAlg ###########
   

        ########### here starts InDetAlignMonResidualsAlgs ###########
        kwargsIDAlignMonResidualsAlg = { 'TrackName'  : kwargsIDAlignMonGenericTracksAlg["TrackName"], 'UseITkGeometry': UseITkGeometry,}  #for residuals, use the same track collections as for track monitoring
         
        if ("NONE" not in trackCollectionName2): kwargsIDAlignMonResidualsAlg.update({'TrackName2' : trackCollectionName2})
        
        from InDetAlignmentMonitoringRun3.IDAlignMonResidualsAlgCfg import IDAlignMonResidualsAlgCfg
        inDetAlignMonResidualsAlg = helper.addAlgorithm(CompFactory.IDAlignMonResidualsAlg, 'IDAlignMonResidualsAlg'+'_'+kwargsIDAlignMonResidualsAlg["TrackName"],
                                                        ApplyTrackSelection = False)
        
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
            
            for k, v in kwargsIDAlignMonPVBiasesAlg.items():
                setattr(inDetAlignMonPVBiasesAlg, k, v)
                
            from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
            TrackToVertexIPEstimator = acc.popToolsAndMerge(
                TrackToVertexIPEstimatorCfg(flags, name='TrackToVertexIPEstimator'))

            inDetAlignMonPVBiasesAlg.TrackToVertexIPEstimator = TrackToVertexIPEstimator

            #IDAlignMonPVBiasesAlgCfg(helper, inDetAlignMonPVBiasesAlg, **kwargsIDAlignMonPVBiasesAlg)
            IDAlignMonPVBiasesAlgCfg(helper, inDetAlignMonPVBiasesAlg )
        
        ########### here ends InDetAlignPVBiasesAlg ###########

    acc.merge(helper.result())

    return acc
#
