# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/python/IDAlignToolsConfig.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

##----- Geometry mangager tool config functions -----##


def InDetAlignModuleToolCfg(flags, name="InDetAlignModuleTool", **kwargs):
    cfg = ComponentAccumulator()
    cfg.setPrivateTools(CompFactory.InDet.InDetAlignModuleTool(name, **kwargs))
    return cfg


def PixelGeometryManagerToolCfg(flags, name="PixelGeometryManagerTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("AlignmentLevel", flags.InDet.Align.pixelAlignmentLevel)
    kwargs.setdefault("AlignmentLevelBarrel", flags.InDet.Align.pixelAlignmentLevelBarrel)
    kwargs.setdefault("AlignmentLevelEndcaps", flags.InDet.Align.pixelAlignmentLevelEndcaps)
    
    kwargs.setdefault("AlignBarrelBowX", True)
    kwargs.setdefault("AlignEndcapRotX", False)
    kwargs.setdefault("AlignEndcapRotY", False)
    kwargs.setdefault("AlignEndcapZ", False)

    if kwargs["AlignmentLevel"] == 16:
        kwargs.setdefault("AlignBarrelRotX", False)
        kwargs.setdefault("AlignBarrelRotY", False)
        kwargs.setdefault("AlignBarrelRotZ", False)
        kwargs.setdefault("AlignBarrelX", False)
        kwargs.setdefault("AlignBarrelY", False)
        kwargs.setdefault("AlignBarrelZ", False)
        kwargs.setdefault("AlignEndcaps", False)
    
    else:
        kwargs.setdefault("SetSoftCutEndcapX", 0.02)
        kwargs.setdefault("SetSoftCutEndcapY", 0.02)
        kwargs.setdefault("SetSoftCutEndcapZ", 0.02)
        kwargs.setdefault("SetSoftCutEndcapRotX", 0.05)
        kwargs.setdefault("SetSoftCutEndcapRotY", 0.05)
        kwargs.setdefault("SetSoftCutEndcapRotZ", 0.05)

    kwargs.setdefault("SetSoftCutBarrelX", 0.02)
    kwargs.setdefault("SetSoftCutBarrelY", 0.02)
    kwargs.setdefault("SetSoftCutBarrelZ", 0.02)
    kwargs.setdefault("SetSoftCutBarrelRotX", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotY", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotZ", 0.05)
            
    cfg.setPrivateTools(
        CompFactory.InDet.PixelGeometryManagerTool(name, **kwargs)) 
    return cfg
    

def SCTGeometryManagerToolCfg(flags, name="SCTGeometryManagerTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("AlignmentLevel", flags.InDet.Align.SCTAlignmentLevel)
    kwargs.setdefault("AlignmentLevelBarrel", flags.InDet.Align.SCTAlignmentLevelBarrel)
    kwargs.setdefault("AlignmentLevelEndcaps", flags.InDet.Align.SCTAlignmentLevelEndcaps)
    
    kwargs.setdefault("AlignBarrelRotX", False)
    kwargs.setdefault("AlignBarrelRotY", False)
    kwargs.setdefault("AlignBarrelRotZ", False)
    kwargs.setdefault("AlignBarrelX", False)
    kwargs.setdefault("AlignBarrelY", False)
    kwargs.setdefault("AlignBarrelZ", False)
    kwargs.setdefault("AlignEndcapZ", False)
    
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
    

def SiGeometryManagerToolCfg(flags, name="SiGeometryManagerTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))
        
    kwargs.setdefault("AlignPixel", flags.InDet.Align.alignPixel)
    kwargs.setdefault("AlignSCT", flags.InDet.Align.alignSCT)
    kwargs.setdefault("AlignmentLevel", -1)
    kwargs.setdefault("ModuleSelection", [])
    
    kwargs.setdefault("PixelGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(PixelGeometryManagerToolCfg(flags))))

    kwargs.setdefault("SCTGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(SCTGeometryManagerToolCfg(flags))))

    cfg.setPrivateTools(CompFactory.InDet.SiGeometryManagerTool(name, **kwargs))
    return cfg
    

def TRTGeometryManagerToolCfg(flags, name="TRTGeometryManagerTool", **kwargs):
    cfg = ComponentAccumulator()
    
    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("AlignmentLevel", flags.InDet.Align.TRTAlignmentLevel)
    kwargs.setdefault("AlignmentLevelBarrel", flags.InDet.Align.TRTAlignmentLevelBarrel)
    kwargs.setdefault("AlignmentLevelEndcaps", flags.InDet.Align.TRTAlignmentLevelEndcaps)

    kwargs.setdefault("AlignEndcapZ", False)

    kwargs.setdefault("SetSoftCutBarrelX", 0.1)
    kwargs.setdefault("SetSoftCutBarrelY", 0.1)
    kwargs.setdefault("SetSoftCutBarrelZ", 0.1)
    kwargs.setdefault("SetSoftCutBarrelRotX", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotY", 0.05)
    kwargs.setdefault("SetSoftCutBarrelRotZ", 0.05)
    kwargs.setdefault("SetSoftCutEndcapX", 0.1)
    kwargs.setdefault("SetSoftCutEndcapY", 0.1)
    kwargs.setdefault("SetSoftCutEndcapZ", 0.001)
    kwargs.setdefault("SetSoftCutEndcapRotX", 0.05)
    kwargs.setdefault("SetSoftCutEndcapRotY", 0.05)
    kwargs.setdefault("SetSoftCutEndcapRotZ", 0.05)

    cfg.setPrivateTools(
        CompFactory.InDet.TRTGeometryManagerTool(name, **kwargs))   
    return cfg
    

def InDetGeometryManagerToolCfg(
        flags, name="InDetGeometryManagerTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("AlignSilicon", flags.InDet.Align.alignPixel or flags.InDet.Align.alignSCT)
    kwargs.setdefault("AlignTRT", flags.InDet.Align.alignTRT)
    kwargs.setdefault("AlignmentLevel", -1)

    kwargs.setdefault("SiGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(SiGeometryManagerToolCfg(flags))))

    kwargs.setdefault("TRTGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(TRTGeometryManagerToolCfg(flags))))
            
    cfg.setPrivateTools(
        CompFactory.InDet.InDetGeometryManagerTool(name, **kwargs))
    
    return cfg


##----- Inner Detector DB I/O Setup -----##

def SiTrkAlignDBToolCfg(flags, name="SiTrkAlignDBTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))
        
    kwargs.setdefault("SiGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(SiGeometryManagerToolCfg(flags))))
    
    kwargs.setdefault("PixelGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(PixelGeometryManagerToolCfg(flags))))

    kwargs.setdefault("SCTGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(SCTGeometryManagerToolCfg(flags))))

    from InDetAlignGenTools.InDetAlignGenToolsConfig import InDetAlignDBTool
    kwargs.setdefault("IDAlignDBTool", cfg.addPublicTool(cfg.popToolsAndMerge(InDetAlignDBTool(flags))))

    kwargs.setdefault("WriteOldConstants", not flags.InDet.Align.accumulate)
    kwargs.setdefault("UpdateConstants", not flags.InDet.Align.accumulate)

    cfg.setPrivateTools(CompFactory.InDet.SiTrkAlignDBTool(name, **kwargs))
    return cfg
    
def TRTTrkAlignDBToolCfg(flags, name="TRTTrkAlignDBTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("TRTGeometryManager", cfg.addPublicTool(
        cfg.popToolsAndMerge(TRTGeometryManagerToolCfg(flags))))
    
    kwargs.setdefault("WriteOldConstants", not flags.InDet.Align.accumulate)
    kwargs.setdefault("UpdateConstants", not flags.InDet.Align.accumulate)
    
    cfg.setPrivateTools(CompFactory.InDet.TRTTrkAlignDBTool(name, **kwargs))
    return cfg
    

def InDetTrkAlignDBToolCfg(flags, name="InDetTrkAlignDBTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("SiTrkAlignDBTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(SiTrkAlignDBToolCfg(flags))))
    kwargs.setdefault("TRTTrkAlignDBTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(TRTTrkAlignDBToolCfg(flags))))

    cfg.setPrivateTools(CompFactory.InDet.InDetTrkAlignDBTool(name, **kwargs))
    return cfg

##----- GlobalChi2AlignTool Setup -----##

def MatrixToolCfg(flags, name="MatrixTool", **kwargs):
    cfg = ComponentAccumulator()
    
    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("InputTFiles", flags.InDet.Align.inputTFiles)
    kwargs.setdefault("TFileName", flags.InDet.Align.outputTFile)
    kwargs.setdefault("SolveOption", 0 if flags.InDet.Align.accumulate else 1)
    kwargs.setdefault("MinNumHitsPerModule", 10)
    kwargs.setdefault("AlignIBLbutNotPixel", flags.InDet.Align.pixelAlignmentLevel == 16)
    kwargs.setdefault("Remove_IBL_Rz", flags.InDet.Align.pixelAlignmentLevel == 11)
    kwargs.setdefault("RunLocalMethod", False)
    kwargs.setdefault("ReadTFile", not flags.InDet.Align.accumulate)
    kwargs.setdefault("ScaleMatrix", True)
    kwargs.setdefault("WriteEigenMat", False)
    kwargs.setdefault("WriteEigenMatTxt", False)
    kwargs.setdefault("WriteMat", False)
    kwargs.setdefault("WriteTFile", True)
    
    cfg.setPrivateTools(CompFactory.Trk.MatrixTool(name, **kwargs))
    return cfg


def GlobalChi2AlignToolCfg(flags, name="GlobalChi2AlignTool", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("MatrixTool", cfg.popToolsAndMerge(MatrixToolCfg(flags)))
    kwargs.setdefault("SecondDerivativeCut", 0)
        
    cfg.setPrivateTools(CompFactory.Trk.GlobalChi2AlignTool(name, **kwargs))
    return cfg


##----- AlignTrackCreator Setup -----##
    
def AlignResidualCalculatorCfg(flags, name="AlignResidualCalculator", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("ResidualType", 0)
        
    cfg.setPrivateTools(CompFactory.Trk.AlignResidualCalculator(name, **kwargs))
    return cfg


def AlignTrackCreatorCfg(flags, name="AlignTrackCreator", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    kwargs.setdefault("ResidualCalculator", cfg.popToolsAndMerge(
        AlignResidualCalculatorCfg(flags)))

    kwargs.setdefault("IncludeScatterers", False)
    kwargs.setdefault("RemoveATSOSNotInAlignModule", False)
        
    cfg.setPrivateTools(CompFactory.Trk.AlignTrackCreator(name, **kwargs))
    return cfg


##----- BeamspotVertexPreProcessor Setup -----##
    
def BeamspotVertexPreProcessorCfg(
        flags, name="BeamspotVertexPreProcessor", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("AlignModuleTool", cfg.addPublicTool(
        cfg.popToolsAndMerge(InDetAlignModuleToolCfg(flags))))

    if "TrackFitter" not in kwargs:
        from TrkConfig.CommonTrackFitterConfig import InDetStandaloneTrackFitterCfg
        kwargs.setdefault("TrackFitter", cfg.addPublicTool(
            cfg.popToolsAndMerge(InDetStandaloneTrackFitterCfg(
                flags, FillDerivativeMatrix = True))))

    if "TrackToVertexIPEstimatorTool" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            TrackToVertexIPEstimatorCfg)
        kwargs.setdefault("TrackToVertexIPEstimatorTool", cfg.addPublicTool(
            cfg.popToolsAndMerge(TrackToVertexIPEstimatorCfg(flags))))

    if "BSConstraintTrackSelector" not in kwargs:
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
            Align_InDetTrackSelectionToolCfg)
        kwargs.setdefault("BSConstraintTrackSelector", cfg.addPublicTool(
            cfg.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))))
            
    if "TrackSelector" not in kwargs:
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
            Align_InDetTrackSelectionToolCfg)
        kwargs.setdefault("TrackSelector", cfg.addPublicTool(
            cfg.popToolsAndMerge(Align_InDetTrackSelectionToolCfg(flags))))
            
    if "Extrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import InDetExtrapolatorCfg
        kwargs.setdefault("Extrapolator", cfg.addPublicTool(
            cfg.popToolsAndMerge(InDetExtrapolatorCfg(flags))))

    kwargs.setdefault("UseSingleFitter", True)
    kwargs.setdefault("RunOutlierRemoval", True)
    kwargs.setdefault("DoBSConstraint", False)
    kwargs.setdefault("DoAssociatedToPVSelection", False)
    kwargs.setdefault("RunOutlierRemoval", True)
                
    cfg.setPrivateTools(
        CompFactory.Trk.BeamspotVertexPreProcessor(name, **kwargs))
    return cfg
