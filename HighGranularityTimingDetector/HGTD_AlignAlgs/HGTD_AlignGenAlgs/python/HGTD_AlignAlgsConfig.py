# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HGTD_CreateMisalignAlgCfg(flags,
                             name="HGTD_MisalignAlg",
                             **kwargs):

    result = ComponentAccumulator()

    from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_GeoModelCfg
    result.merge(HGTD_GeoModelCfg(flags))

    # -----------------------------------------
    # ROOT output services
    # -----------------------------------------

    histoSvc = CompFactory.THistSvc(
        Output=[
            "IDENTIFIERTREE DATAFILE='HGTDIdentifierTree.root' TYPE='ROOT' OPT='RECREATE'"
        ]
    )
    result.addService(histoSvc)

    ntupSvc = CompFactory.NTupleSvc(
        Output=[
            "CREATEMISALIGN DATAFILE='CreateMisalignmentHGTD.root' TYPE='ROOT' OPT='NEW'"
        ]
    )
    result.addService(ntupSvc)

    result.setAppProperty("HistogramPersistency", "ROOT")

    alg = CompFactory.HGTD_MisalignAlg(name)

    dbTool = kwargs.pop("AlignDBTool", None)
    if dbTool is None:
        from HGTD_AlignGenTools.HGTD_AlignGenToolsConfig import HGTD_AlignDBToolCfg
        dbTool = result.popToolsAndMerge(HGTD_AlignDBToolCfg(flags))
        
    alg.AlignDBTool = dbTool

    # -------------------------
    # Tool
    # -------------------------

    alg.AlignDBTool = dbTool

    # -------------------------
    # Misalignment parameters
    # -------------------------

    alg.MisalignMode = int(kwargs.get("MisalignMode", 1))

    alg.ShiftX = float(kwargs.get("ShiftX", 0.05))
    alg.ShiftY = float(kwargs.get("ShiftY", 0.0))
    alg.ShiftZ = float(kwargs.get("ShiftZ", 0.0))

    alg.SigmaX = float(kwargs.get("SigmaX", 0.01))
    alg.SigmaY = float(kwargs.get("SigmaY", 0.01))
    alg.SigmaZ = float(kwargs.get("SigmaZ", 0.01))

    # -------------------------
    # Options
    # -------------------------

    alg.ApplyTranslation = kwargs.get("ApplyTranslation", True)
    alg.ApplyRotation = kwargs.get("ApplyRotation", False)

    alg.DoDebugPrint = kwargs.get("DoDebugPrint", True)
    alg.MaxModules = int(kwargs.get("MaxModules", -1))
    alg.ApplyToGeometry = kwargs.get("ApplyToGeometry", False)
    alg.TestHash = int(kwargs.get("TestHash", -1))

    alg.CreateFreshDB = kwargs.get("CreateFreshDB", True)
    alg.SQLiteTag = kwargs.get("SQLiteTag", "HGTD_AlignTag")
    alg.OutputFile = kwargs.get("OutputFile", "hgtd_misalignment.txt")
    alg.WriteToDB = kwargs.get("WriteToDB", False)
    alg.RandomStream = kwargs.get("RandomStream", "HGTDMisalignment")

    result.addEventAlgo(alg)

    return result