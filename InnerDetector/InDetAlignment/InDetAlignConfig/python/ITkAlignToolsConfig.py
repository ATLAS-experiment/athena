# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkAlignModuleToolCfg(flags, name="ITkAlignModuleTool", **kwargs):   
    cfg = ComponentAccumulator()
    cfg.setPrivateTools(CompFactory.InDet.InDetAlignModuleTool(name, **kwargs))
    return cfg

def ITkGeometryManagerToolCfg(flags, name="ITkGeometryManagerTool", **kwargs):

    cfg = ComponentAccumulator()
    
    kwargs.setdefault("PixelDetectorManager","ITkPixel")
    kwargs.setdefault("StripDetectorManager","ITkStrip")
    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))
        
    kwargs.setdefault("AlignPixel", flags.ITk.Align.alignITkPixel)
    kwargs.setdefault("AlignSCT", flags.ITk.Align.alignITkStrip)
    kwargs.setdefault("AlignmentLevel", -1) ## TODO: Figure out proper alignment levels (many leads to errors)
    kwargs.setdefault("ModuleSelection", [])
    
    if flags.ITk.Align.alignITkPixel: 
        kwargs.setdefault("PixelGeometryManager", cfg.addPublicTool(
            cfg.popToolsAndMerge(ITkPixelGeometryManagerToolCfg(flags))))

    if flags.ITk.Align.alignITkStrip:
        kwargs.setdefault("SCTGeometryManager", cfg.addPublicTool(
            cfg.popToolsAndMerge(ITkStripGeometryManagerToolCfg(flags))))

    cfg.setPrivateTools(CompFactory.InDet.SiGeometryManagerTool(name, **kwargs))
    return cfg

def ITkPixelGeometryManagerToolCfg(flags, name="ITkPixelManagerTool", **kwargs):
    
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))

    kwargs.setdefault("PixelDetectorManager","ITkPixel")
    kwargs.setdefault("EtaCorrection",False)
    
    ## TODO: Figure out proper alignment levels (many leads to errors)
    kwargs.setdefault("AlignmentLevel", 1)
   # kwargs.setdefault("AlignmentLevelBarrel", 12)
   # kwargs.setdefault("AlignmentLevelEndcaps", 12)

    kwargs.setdefault("SetSoftCutBarrelX", 0.02)
    kwargs.setdefault("SetSoftCutBarrelY", 0.02)
    kwargs.setdefault("SetSoftCutBarrelZ", 0.02)
    kwargs.setdefault("SetSoftCutBarrelRotX", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotY", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotZ", 0.05)
    kwargs.setdefault("SetSoftCutEndcapX", 0.02)
    kwargs.setdefault("SetSoftCutEndcapY", 0.02)
    kwargs.setdefault("SetSoftCutEndcapZ", 0.02)
    kwargs.setdefault("SetSoftCutEndcapRotX", 0.05)
    kwargs.setdefault("SetSoftCutEndcapRotY", 0.05)
    kwargs.setdefault("SetSoftCutEndcapRotZ", 0.05)
            
    cfg.setPrivateTools(
        CompFactory.InDet.PixelGeometryManagerTool(name, **kwargs)) 
    return cfg

def ITkStripGeometryManagerToolCfg(flags, name="ITkStripManagerTool", **kwargs):
    
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))

    kwargs.setdefault("StripDetectorManager","ITkStrip")
    
    ## TODO: Figure out proper alignment levels (many leads to errors)
    kwargs.setdefault("AlignmentLevel", 0)
   # kwargs.setdefault("AlignmentLevelBarrel", 2)
   # kwargs.setdefault("AlignmentLevelEndcaps", 2)

    kwargs.setdefault("SetSoftCutBarrelX", 0.05)
    kwargs.setdefault("SetSoftCutBarrelY", 0.05)
    kwargs.setdefault("SetSoftCutBarrelZ", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotX", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotY", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotZ", 0.05)
    kwargs.setdefault("SetSoftCutEndcapX", 0.05)
    kwargs.setdefault("SetSoftCutEndcapY", 0.05)
    kwargs.setdefault("SetSoftCutEndcapZ", 0.005)
    kwargs.setdefault("SetSoftCutEndcapRotX", 0.005)
    kwargs.setdefault("SetSoftCutEndcapRotY", 0.05)
    kwargs.setdefault("SetSoftCutEndcapRotZ", 0.05)
    
    cfg.setPrivateTools(
        CompFactory.InDet.SCTGeometryManagerTool(name, **kwargs))
    return cfg

def ITkTrkAlignDBToolCfg(flags, name="ITkTrkAlignDBTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))
        
    kwargs.setdefault("SiGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkGeometryManagerToolCfg(flags))))
            
    kwargs.setdefault("PixelGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkPixelGeometryManagerToolCfg(flags))))

    kwargs.setdefault("SCTGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkStripGeometryManagerToolCfg(flags))))

    from InDetAlignGenTools.InDetAlignGenToolsConfig import ITkAlignDBTool
    kwargs.setdefault("IDAlignDBTool", cfg.addPublicTool(cfg.popToolsAndMerge(ITkAlignDBTool(flags))))

    kwargs.setdefault("WriteOldConstants", not flags.ITk.Align.accumulate)
    kwargs.setdefault("UpdateConstants", not flags.ITk.Align.accumulate)

    #Set filenames empty to suppress writing
    kwargs.setdefault("OutputIBLDistFile","")
    kwargs.setdefault("OldIBLDistFile","")

    cfg.setPrivateTools(CompFactory.InDet.SiTrkAlignDBTool(name, **kwargs))
    return cfg

def ITkMatrixToolCfg(flags, name="ITkMatrixTool", **kwargs):
    cfg = ComponentAccumulator()
    
    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))

    kwargs.setdefault("PathBinName", f"{flags.ITk.Align.baseDir}/Accumulate/")
    kwargs.setdefault("PathTxtName", f"{flags.ITk.Align.baseDir}/Accumulate/")
    kwargs.setdefault("InputTFiles", [f"{flags.ITk.Align.baseDir}/Accumulate/{flags.ITk.Align.inputTFiles}"])
    kwargs.setdefault("SolveOption", 3)
    kwargs.setdefault("MinNumHitsPerModule", 10)

    kwargs.setdefault("WriteTFile", flags.ITk.Align.accumulate)
    kwargs.setdefault("ReadTFile", not flags.ITk.Align.accumulate)
    kwargs.setdefault("ScaleMatrix", True)
    kwargs.setdefault("WriteEigenMat", False)
    kwargs.setdefault("WriteEigenMatTxt", False)
           
    cfg.setPrivateTools(CompFactory.Trk.MatrixTool(name, **kwargs))
    return cfg

def ITkGlobalChi2AlignToolCfg(flags, name="ITkGlobalChi2AlignTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))

    kwargs.setdefault("MatrixTool", cfg.popToolsAndMerge(ITkMatrixToolCfg(flags)))

    kwargs.setdefault("StoreLocalDerivOnly", flags.ITk.Align.solveLocal)
    kwargs.setdefault("SecondDerivativeCut", 0)
    #kwargs.setdefault("OutputLevel", 1)
        
    cfg.setPrivateTools(CompFactory.Trk.GlobalChi2AlignTool(name, **kwargs))
    return cfg    

def ITkAlignResidualCalculatorCfg(flags, name="ITkAlignResidualCalculator", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("ResidualType", 0)
        
    cfg.setPrivateTools(CompFactory.Trk.AlignResidualCalculator(name, **kwargs))
    return cfg


def ITkAlignTrackCreatorCfg(flags, name="ITkAlignTrackCreator", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))

    kwargs.setdefault("ResidualCalculator", cfg.popToolsAndMerge(
        ITkAlignResidualCalculatorCfg(flags)))

    kwargs.setdefault("IncludeScatterers", False)
    kwargs.setdefault("RemoveATSOSNotInAlignModule", False)
        
    cfg.setPrivateTools(CompFactory.Trk.AlignTrackCreator(name, **kwargs))
    return cfg

def ITkBeamspotVertexPreProcessorCfg(flags, name="ITkBeamspotVertexPreProcessor", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(ITkAlignModuleToolCfg(flags))))

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import ITkStandaloneTrackFitterCfg
        kwargs.setdefault("TrackFitter", cfg.addPublicTool(
            cfg.popToolsAndMerge(ITkStandaloneTrackFitterCfg(flags, 
            FillDerivativeMatrix = True))))

    if "TrackToVertexIPEstimatorTool" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
        kwargs.setdefault("TrackToVertexIPEstimatorTool", cfg.addPublicTool(
            cfg.popToolsAndMerge(TrackToVertexIPEstimatorCfg(flags))))

    if "BSConstraintTrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import Align_InDetTrackSelectionToolCfg
        kwargs.setdefault("BSConstraintTrackSelector", cfg.addPublicTool(
            cfg.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))))

    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", cfg.addPublicTool(
            cfg.popToolsAndMerge(InDetExtrapolatorCfg(flags))))

    kwargs.setdefault("UseSingleFitter", True)
    kwargs.setdefault("RunOutlierRemoval", False)
    kwargs.setdefault("DoBSConstraint", False)
    kwargs.setdefault("DoAssociatedToPVSelection", False)
                
    cfg.setPrivateTools(
        CompFactory.Trk.BeamspotVertexPreProcessor(name, **kwargs))
    return cfg


    
