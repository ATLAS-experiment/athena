# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def ITkAlignAlgCfg(flags, name="ITkAlignAlgSolve", **kwargs):
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

    if "AlignTrackCreator" not in kwargs:
        from InDetAlignConfig.ITkAlignToolsConfig import ITkAlignTrackCreatorCfg
        kwargs.setdefault("AlignTrackCreator", cfg.popToolsAndMerge(
            ITkAlignTrackCreatorCfg(flags)))

    kwargs.setdefault("AlignTrackPreProcessor", None)
    kwargs.setdefault("WriteNtuple", False)
    kwargs.setdefault("SolveOnly", True)
    
    cfg.addEventAlgo(CompFactory.Trk.AlignAlg(name, **kwargs))
    return cfg
    
    

def ITkSolveCfg(flags, **kwargs):
    cfg = ITkAlignAlgCfg(flags)
    
    ##----- Setup of OutputConditionsAlg and its tools -----##
    
    if flags.ITk.Align.writeConstantsToPool:
        objectList = []
        tagList = []

        if flags.ITk.Align.writeSilicon:
            if flags.ITk.Align.writeDynamicDB:
                objectList.extend(["CondAttrListCollection#/Indet/AlignL1/ID",
                                   "CondAttrListCollection#/Indet/AlignL2/PIX", 
                                   "CondAttrListCollection#/Indet/AlignL2/SCT",
                                   "AlignableTransformContainer#/Indet/AlignL3"])
                tagList.extend(["IndetL1Test", "IndetL2PIXTest", "IndetL2SCTTest",
                                flags.ITk.Align.tagSi])
            else:
                objectList.extend(["AlignableTransformContainer#/Indet/Align"])
                tagList.extend([flags.ITk.Align.tagSi])


        from RegistrationServices.OutputConditionsAlgConfig import OutputConditionsAlgCfg
        cfg.merge(OutputConditionsAlgCfg(
            flags, 
            outputFile = f"{flags.ITk.Align.baseDir}/Solve/{flags.ITk.Align.outputConditionFile}",
            ObjectList = objectList, IOVTagList = tagList, WriteIOV = False)) ##TODO: Set this to false to avoid errors

    cfg.addEventAlgo(CompFactory.Trk.AlignTrackCollSplitter())
    return cfg
