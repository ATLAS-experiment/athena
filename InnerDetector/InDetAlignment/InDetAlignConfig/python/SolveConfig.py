# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/python/SolveConfig.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def AlignAlgCfg(flags, name="AlignAlgSolve", **kwargs):
    cfg = ComponentAccumulator()

    if "GeometryManagerTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import GeometryManagerToolCfg
        kwargs.setdefault("GeometryManagerTool", cfg.addPublicTool(cfg.popToolsAndMerge(
            GeometryManagerToolCfg(flags))))

    if "AlignTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import GlobalChi2AlignToolCfg
        kwargs.setdefault("AlignTool", cfg.popToolsAndMerge(GlobalChi2AlignToolCfg(flags)))

    if "AlignDBTool" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import AlignDBToolCfg
        kwargs.setdefault("AlignDBTool", cfg.popToolsAndMerge(AlignDBToolCfg(flags)))

    if "AlignTrackCreator" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import AlignTrackCreatorCfg
        kwargs.setdefault("AlignTrackCreator", cfg.popToolsAndMerge(
            AlignTrackCreatorCfg(flags)))

    kwargs.setdefault("AlignTrackPreProcessor", None)
    kwargs.setdefault("WriteNtuple", False)
    kwargs.setdefault("SolveOnly", True)
    
    cfg.addEventAlgo(CompFactory.Trk.AlignAlg(name, **kwargs))
    return cfg
    
    

def SolveCfg(flags, **kwargs):
    cfg = AlignAlgCfg(flags)
    
    ##----- Setup of OutputConditionsAlg and its tools -----##
    
    if flags.InDet.Align.writeConstantsToPool:
        objectList = []
        tagList = []

        if flags.InDet.Align.writeSilicon:
            if flags.InDet.Align.writeDynamicDB:
                objectList.extend(["CondAttrListCollection#/Indet/AlignL1/ID",
                                   "CondAttrListCollection#/Indet/AlignL2/PIX", 
                                   "CondAttrListCollection#/Indet/AlignL2/SCT",
                                   "AlignableTransformContainer#/Indet/AlignL3"])
                tagList.extend(["IndetL1Test", "IndetL2PIXTest", "IndetL2SCTTest",
                                flags.InDet.Align.tagSi])
            else:
                objectList.extend(["AlignableTransformContainer#/Indet/Align"])
                tagList.extend([flags.InDet.Align.tagSi])
                
        if flags.InDet.Align.writeTRT:
            if flags.InDet.Align.writeDynamicDB:
                objectList.extend(["CondAttrListCollection#/TRT/AlignL1/TRT",
                                   "AlignableTransformContainer#/TRT/AlignL2"])
                tagList.extend(["IndetL1TRTTest", flags.InDet.Align.tagTRT])
            else:
                objectList.extend(["AlignableTransformContainer#/TRT/Align"])
                tagList.extend([flags.InDet.Align.tagTRT])
                
        if flags.InDet.Align.writeTRTL3:
            objectList.extend(["TRTCond::StrawDxContainer#/TRT/Calib/DX"])
            
        if flags.InDet.Align.writeIBLDistDB:
            objectList.extend(["CondAttrListCollection#/Indet/IBLDist"])
            tagList.extend([flags.InDet.Align.tagBow])

        from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
        cfg.merge(OutputConditionsAlgCfg(
            flags, 
            outputFile = f"{flags.InDet.Align.baseDir}/Solve/{flags.InDet.Align.outputConditionFile}",
            ObjectList = objectList, IOVTagList = tagList, WriteIOV = False)) ##TODO: Set this to false to avoid errors

    cfg.addEventAlgo(CompFactory.Trk.AlignTrackCollSplitter())
    return cfg
