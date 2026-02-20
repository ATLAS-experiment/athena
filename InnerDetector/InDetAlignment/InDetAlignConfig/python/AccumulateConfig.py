# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/python/AccumulateConfig.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

##----- Setup of Tools for Trk::AlignAlg -----##
    
def TrackCollectionProviderCfg(flags, name = "TrackCollectionProvider", **kwargs):
    cfg = ComponentAccumulator()
    
    kwargs.setdefault("InputTrkCol", flags.InDet.Align.inputTracksCollection)
    kwargs.setdefault("MaxRetrievalErrors", 10)
    
    cfg.setPrivateTools(CompFactory.Trk.TrackCollectionProvider(name, **kwargs))
    return cfg
    
def ConstrainedTrackProviderCfg(flags, name = "ConstrainedTrackProvider", **kwargs):
    cfg = ComponentAccumulator()

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetStandaloneTrackFitterCfg
        kwargs.setdefault("TrackFitter", cfg.popToolsAndMerge(
            InDetStandaloneTrackFitterCfg(flags, FillDerivativeMatrix = True)))

    kwargs.setdefault("MinPt", 0.)

    from PathResolver import PathResolver

    kwargs.setdefault("MomentumConstraintFileName", PathResolver.FindCalibFile("InDetAlign/nullmap.root"))
    kwargs.setdefault("MomentumConstraintHistName", "LambdaCorrectionVsEtaPhi")
    kwargs.setdefault("ScalePMapToGeV", True)
    kwargs.setdefault("ReduceConstraintUncertainty", 100.)
    kwargs.setdefault("z0ConstraintFileName", "")
    kwargs.setdefault("z0ConstraintHistName", "z0CorrectionVsEtaPhi")
    kwargs.setdefault("d0ConstraintFileName", "")
    kwargs.setdefault("d0ConstraintHistName", "d0CorrectionVsEtaPhi")
    kwargs.setdefault("UseConstraintError", False)
    kwargs.setdefault("UseConstrainedTrkOnly", True)
    kwargs.setdefault("InputTracksCollection", flags.InDet.Align.inputTracksCollection)
      
    cfg.setPrivateTools(CompFactory.Trk.ConstrainedTrackProvider(name, **kwargs))
    return cfg


def AnalyticalDerivCalcToolCfg(flags, name="AnalyticalDerivCalcTool", **kwargs):
    cfg = ComponentAccumulator()

    if "AlignModuleTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import InDetAlignModuleToolCfg
        kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(cfg.popToolsAndMerge(
            InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("UseIntrinsicPixelError", True)
    kwargs.setdefault("UseIntrinsicSCTError", True)
    kwargs.setdefault("UseIntrinsicTRTError", True)
        
    cfg.setPrivateTools(CompFactory.Trk.AnalyticalDerivCalcTool(name, **kwargs))
    return cfg


def AlignTrackDresserCfg(flags, name="AlignTrackDresser", **kwargs):
    cfg = ComponentAccumulator()

    if "DerivCalcTool" not in kwargs:
        kwargs.setdefault("DerivCalcTool", cfg.popToolsAndMerge(
            AnalyticalDerivCalcToolCfg(flags)))
        
    cfg.setPrivateTools(CompFactory.Trk.AlignTrackDresser(name, **kwargs))
    return cfg


def SimpleIDNtupleToolCfg(flags, name="SimpleIDNtupleTool", **kwargs):
    cfg = ComponentAccumulator()

    if "AlignModuleTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import InDetAlignModuleToolCfg
        kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(cfg.popToolsAndMerge(
            InDetAlignModuleToolCfg(flags))))

    if "TrackParticleCreatorTool" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import TrackParticleCreatorToolCfg
        kwargs.setdefault("TrackParticleCreatorTool", cfg.popToolsAndMerge(
            TrackParticleCreatorToolCfg(flags)))
        
    cfg.setPrivateTools(CompFactory.InDet.SimpleIDNtupleTool(name, **kwargs))
    return cfg


def AlignAlgCfg(flags, name="AlignAlgAccumulate", **kwargs):
    cfg = ComponentAccumulator()

    if "GeometryManagerTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import InDetGeometryManagerToolCfg
        kwargs.setdefault("GeometryManagerTool", cfg.addPublicTool(cfg.popToolsAndMerge(
            InDetGeometryManagerToolCfg(flags))))

    if "AlignTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import GlobalChi2AlignToolCfg
        kwargs.setdefault("AlignTool", cfg.popToolsAndMerge(GlobalChi2AlignToolCfg(flags)))

    if "AlignDBTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import InDetTrkAlignDBToolCfg
        kwargs.setdefault("AlignDBTool", cfg.popToolsAndMerge(InDetTrkAlignDBToolCfg(flags)))

    kwargs.setdefault("TrackCollectionProvider", cfg.popToolsAndMerge(
        TrackCollectionProviderCfg(flags)))

    if "AlignTrackCreator" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import AlignTrackCreatorCfg
        kwargs.setdefault("AlignTrackCreator", cfg.popToolsAndMerge(
            AlignTrackCreatorCfg(flags)))

    kwargs.setdefault("AlignTrackDresser", cfg.popToolsAndMerge(AlignTrackDresserCfg(flags)))

    if "AlignTrackPreProcessor" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import BeamspotVertexPreProcessorCfg
        kwargs.setdefault("AlignTrackPreProcessor", cfg.popToolsAndMerge(
            BeamspotVertexPreProcessorCfg(flags)))

    kwargs.setdefault("WriteNtuple", flags.InDet.Align.writeAlignNtuple)
    
    if flags.InDet.Align.writeAlignNtuple:
        kwargs.setdefault("FillNtupleTool", cfg.popToolsAndMerge(SimpleIDNtupleToolCfg(flags)))
        kwargs.setdefault("FilePath", "{flags.InDet.Align.baseDir}/Accumulate")
        kwargs.setdefault("FileName", "newIDalign.root")

    cfg.addEventAlgo(CompFactory.Trk.AlignAlg(name, **kwargs))
    
    return cfg
    
def AccumulateCfg(flags):
    cfg = AlignAlgCfg(flags)
    
    if flags.InDet.Align.doMonitoring:
        from InDetAlignmentMonitoringRun3.InDetAlignmentMonitoringRun3Config import (
            InDetAlignmentMonitoringRun3Config)
            
        cfg.merge(InDetAlignmentMonitoringRun3Config(flags, TrackCollectionName = flags.InDet.Align.inputTracksCollection))
    
    return cfg
