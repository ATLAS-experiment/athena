# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: InDetAlignConfig/python/SolveConfig.py
# Author: David Brunner (david.brunner@cern.ch), Thomas Strebler (thomas.strebler@cern.ch)

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def AlignAlgCfg(flags, name="AlignAlgSolve", **kwargs):
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

    if "AlignTrackCreator" not in kwargs:
        from InDetAlignConfig.IDAlignToolsConfig import AlignTrackCreatorCfg
        kwargs.setdefault("AlignTrackCreator", cfg.popToolsAndMerge(
            AlignTrackCreatorCfg(flags)))

    kwargs.setdefault("AlignTrackPreProcessor", None)
    kwargs.setdefault("WriteNtuple", False)
    kwargs.setdefault("SolveOnly", True)
    
    cfg.addEventAlgo(CompFactory.Trk.AlignAlg(name, **kwargs))
    return cfg
    
def WriteConstCfg(flags, name = "WriteConst", **kwargs):
    cfg = ComponentAccumulator()

    objectList = [
        "CondAttrListCollection#/Indet/AlignL1/ID",
        "CondAttrListCollection#/Indet/AlignL2/PIX", 
        "CondAttrListCollection#/Indet/AlignL2/SCT",
        "AlignableTransformContainer#/Indet/AlignL3",
        "CondAttrListCollection#/TRT/AlignL1/TRT",
        "AlignableTransformContainer#/TRT/AlignL2",
        # Disable TRT L3 until ATLIDTRKCP-745 is solved
        #"TRTCond::StrawDxContainer#/TRT/Calib/DX",
        "CondAttrListCollection#/Indet/IBLDist"
    ]
    
    tagList = [
        "InDetAlignL1-T0-Alignment", 
        "InDetAlignL2PIX-T0-Alignment", 
        "InDetAlignL2SCT-T0-Alignment", 
        "InDetAlignL3-T0-Alignment",
        "InDetAlignL1TRT-T0-Alignment", 
        "InDetAlignL2TRT-T0-Alignment",
        # Disable TRT L3 until ATLIDTRKCP-745 is solved
        #"InDetAlignL3TRT-T0-Alignment",
        "InDetAlignIBLDIST-T0-Alignment"
    ]

    from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
    cfg.merge(OutputConditionsAlgCfg(
        flags, 
        outputFile = flags.InDet.Align.outputConditionFile,
        ObjectList = objectList, IOVTagList = tagList, WriteIOV = True))
            
    return cfg

def SolveCfg(flags, **kwargs):
    cfg = AlignAlgCfg(flags)
    cfg.merge(WriteConstCfg(flags))
    
    return cfg
