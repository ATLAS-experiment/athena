# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

__doc__ = """Tool configuration to instantiate all
 isolationTools with default configuration"""

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

def TrackIsolationToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
        
    if 'TrackSelectionTool' not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import isoTrackSelectionToolCfg
        kwargs['TrackSelectionTool'] = acc.popToolsAndMerge(isoTrackSelectionToolCfg(flags))
    if 'TTVATool' not in kwargs:
        from InDetConfig.TrackVertexAssociationToolConfig import isoTTVAToolCfg
        kwargs['TTVATool'] = acc.popToolsAndMerge(isoTTVAToolCfg(flags))
    if flags.Beam.Type is BeamType.Cosmics:
        kwargs['VertexLocation'] = ''

    acc.setPrivateTools(CompFactory.xAOD.TrackIsolationTool(**kwargs))
    return acc

def CaloIsolationToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()
        
    if 'IsoLeakCorrectionTool' not in kwargs:
        kwargs['IsoLeakCorrectionTool'] = CompFactory.CP.IsolationCorrectionTool(
            name = 'LeakageCorrTool')

    if 'ClustersInConeTool' not in kwargs:
        kwargs['ClustersInConeTool'] = CompFactory.xAOD.CaloClustersInConeTool(
            name = 'MyCaloClustersInConeTool',
            CaloClusterLocation = "CaloCalTopoClusters")

    if 'ParticleCaloExtensionTool' not in kwargs:
        from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
        kwargs['ParticleCaloExtensionTool'] = acc.popToolsAndMerge(
            ParticleCaloExtensionToolCfg(flags))

    kwargs.setdefault('name','CaloIsolationTool')
    kwargs.setdefault('ParticleCaloExtensionTool',None)
    kwargs.setdefault('ParticleCaloCellAssociationTool',None)
    kwargs.setdefault('isMC',flags.Input.isMC)
        
    acc.setPrivateTools(CompFactory.xAOD.CaloIsolationTool(**kwargs))
    return acc

def ElectronTrackIsolationToolCfg(flags, **kwargs):
    kwargs.setdefault('name','ElectronTrackIsolationTool')
    kwargs.setdefault('CoreTrackEtaRange',0.01)
    return TrackIsolationToolCfg(flags,**kwargs)
        
def EGammaCaloIsolationToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    if 'IsoLeakCorrectionTool' not in kwargs:
        kwargs['IsoLeakCorrectionTool'] = CompFactory.CP.IsolationCorrectionTool(
            name = 'LeakageCorrTool',
            LogLogFitForLeakage = True)

    if 'CaloFillRectangularClusterTool' not in kwargs:
        kwargs['CaloFillRectangularClusterTool'] = CompFactory.CaloFillRectangularCluster(
            name="egamma_CaloFillRectangularCluster",
            eta_size=5,
            phi_size=7,
            cells_name=flags.Egamma.Keys.Input.CaloCells)

    # default is to read calocaltopoclusters.
    # In HI, if subtracted clusters, use them instead, and do not do pu correction
    if flags.HeavyIon.Egamma.doSubtractedClusters:
        kwargs['ClustersInConeTool'] = CompFactory.xAOD.CaloClustersInConeTool(
            name="topoiso_CaloClustersInConeTool",
            CaloClusterLocation=flags.Egamma.Keys.Input.TopoClusters)
        # No pileup correction, and do not save it
        kwargs['InitializeReadHandles'] = False
        kwargs['saveOnlyRequestedCorrections'] = True
        
    kwargs.setdefault('name','egCaloIsolationTool')
    kwargs.setdefault('ParticleCaloExtensionTool',None)
    kwargs.setdefault('ParticleCaloCellAssociationTool',None)
    kwargs.setdefault('isMC',flags.Input.isMC)
        
    acc.setPrivateTools(CompFactory.xAOD.CaloIsolationTool(**kwargs))
    return acc

def MuonCaloIsolationToolCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    if 'ParticleCaloExtensionTool' not in kwargs:
        from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
        kwargs['ParticleCaloExtensionTool'] = acc.popToolsAndMerge(
            ParticleCaloExtensionToolCfg(flags))
    if 'FlowElementsInConeTool' not in kwargs and flags.Reco.EnablePFlow:
        kwargs['FlowElementsInConeTool'] = CompFactory.xAOD.FlowElementsInConeTool(
            name='FlowElementsInConeTool')

    # default is to read calocaltopoclusters.
    # In HI, if subtracted clusters, use them instead, and do not do pu correction
    if flags.HeavyIon.Egamma.doSubtractedClusters:
        ccict = CompFactory.xAOD.CaloClustersInConeTool(
            name="topoiso_CaloClustersInConeTool",
            CaloClusterLocation=flags.Egamma.Keys.Input.TopoClusters)
        kwargs['ClustersInConeTool'] = ccict
        # No pileup correction, and do not save it
        kwargs['InitializeReadHandles'] = False
        kwargs['saveOnlyRequestedCorrections'] = True

    kwargs.setdefault('ParticleCaloCellAssociationTool',None)
    kwargs.setdefault('UseEtaDepPUCorr',False)
    kwargs.setdefault('name','muonCaloIsolationTool')

    acc.setPrivateTools(CompFactory.xAOD.CaloIsolationTool(**kwargs))
    return acc
