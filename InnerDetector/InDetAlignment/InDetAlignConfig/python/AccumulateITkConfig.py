# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

##----- Setup of Tools for Trk::AlignAlg -----##
    
def ITkConstrainedTrackProviderCfg(flags, name="ITkConstrainedTrackProvider", **kwargs):
    cfg = ComponentAccumulator()

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import ITkStandaloneTrackFitterCfg
        kwargs.setdefault("TrackFitter", cfg.popToolsAndMerge(
            ITkStandaloneTrackFitterCfg(flags, FillDerivativeMatrix = True)))

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
    kwargs.setdefault("InputTracksCollection", flags.ConstrainedTrackProvider.InputTracksCollection)
    #kwargs.setdefault("OutputLevel", 1)
                
                
    cfg.setPrivateTools(CompFactory.Trk.ConstrainedTrackProvider(name, **kwargs))
    return cfg


def ITkAnalyticalDerivCalcToolCfg(flags, name="ITkAnalyticalDerivCalcTool", **kwargs):
    cfg = ComponentAccumulator()

    if "AlignModuleTool" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkAlignModuleToolCfg
        kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(cfg.popToolsAndMerge(
            ITkAlignModuleToolCfg(flags))))

    kwargs.setdefault("UseIntrinsicPixelError", True)
    kwargs.setdefault("UseIntrinsicSCTError", True)
    kwargs.setdefault("UseIntrinsicTRTError", True)
        
    cfg.setPrivateTools(CompFactory.Trk.AnalyticalDerivCalcTool(name, **kwargs))
    return cfg


def ITkAlignTrackDresserCfg(flags, name="ITkAlignTrackDresser", **kwargs):
    cfg = ComponentAccumulator()

    if "DerivCalcTool" not in kwargs:
        kwargs.setdefault("DerivCalcTool", cfg.popToolsAndMerge(
            ITkAnalyticalDerivCalcToolCfg(flags)))
        
    cfg.setPrivateTools(CompFactory.Trk.AlignTrackDresser(name, **kwargs))
    return cfg


def SimpleITkNtupleToolCfg(flags, name="SimpleITkNtupleTool", **kwargs):
    cfg = ComponentAccumulator()

    if "AlignModuleTool" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkAlignModuleToolCfg
        kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(cfg.popToolsAndMerge(
            ITkAlignModuleToolCfg(flags))))

    if "TrackParticleCreatorTool" not in kwargs:
        from TrkConfig.TrkParticleCreatorConfig import TrackParticleCreatorToolCfg
        kwargs.setdefault("TrackParticleCreatorTool", cfg.popToolsAndMerge(
            TrackParticleCreatorToolCfg(flags)))
        
    cfg.setPrivateTools(CompFactory.InDet.SimpleIDNtupleTool(name, **kwargs))
    return cfg


def ITkAlignAlgCfg(flags, name="ITkAlignAlgAccumulate", **kwargs):
    cfg = ComponentAccumulator()

    if "GeometryManagerTool" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkGeometryManagerToolCfg
        kwargs.setdefault("GeometryManagerTool", cfg.addPublicTool(cfg.popToolsAndMerge(
            ITkGeometryManagerToolCfg(flags))))

    if "AlignTool" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkGlobalChi2AlignToolCfg
        kwargs.setdefault("AlignTool", cfg.popToolsAndMerge(ITkGlobalChi2AlignToolCfg(flags)))

    if "AlignDBTool" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkTrkAlignDBToolCfg
        kwargs.setdefault("AlignDBTool", cfg.popToolsAndMerge(ITkTrkAlignDBToolCfg(flags)))

    kwargs.setdefault("TrackCollectionProvider", cfg.popToolsAndMerge(
        ITkConstrainedTrackProviderCfg(flags)))

    if "AlignTrackCreator" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkAlignTrackCreatorCfg
        kwargs.setdefault("AlignTrackCreator", cfg.popToolsAndMerge(
            ITkAlignTrackCreatorCfg(flags)))

    kwargs.setdefault("AlignTrackDresser", cfg.popToolsAndMerge(ITkAlignTrackDresserCfg(flags)))

    if "AlignTrackPreProcessor" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkBeamspotVertexPreProcessorCfg
        kwargs.setdefault("AlignTrackPreProcessor", cfg.popToolsAndMerge(
            ITkBeamspotVertexPreProcessorCfg(flags)))

    kwargs.setdefault("WriteNtuple", flags.ITk.Align.writeAlignNtuple)
    if kwargs["WriteNtuple"]:
        kwargs.setdefault("FillNtupleTool", cfg.popToolsAndMerge(SimpleITkNtupleToolCfg(flags)))
        kwargs.setdefault("FilePath", "{flags.ITk.Align.baseDir}/Accumulate")
        kwargs.setdefault("FileName", "newIDalign.root")

    cfg.addEventAlgo(CompFactory.Trk.AlignAlg(name, **kwargs))
    return cfg


def ITkAlignTrackCollSplitterCfg(flags, name="ITkAlignTrackCollSplitter", **kwargs):
    cfg = ComponentAccumulator()
    cfg.addEventAlgo(CompFactory.Trk.AlignTrackCollSplitter(name, **kwargs))
    return cfg


def ITkAccumulateCfg(flags, **kwargs):
    cfg = ITkAlignAlgCfg(flags)
    cfg.merge(ITkAlignTrackCollSplitterCfg(flags))
    
    if flags.ITk.Align.doMonitoring:
        from InDetAlignmentMonitoringRun3.InDetAlignmentMonitoringRun3Config import (
            InDetAlignmentMonitoringRun3Config)
        cfg.merge(InDetAlignmentMonitoringRun3Config(flags))
    
    return cfg
