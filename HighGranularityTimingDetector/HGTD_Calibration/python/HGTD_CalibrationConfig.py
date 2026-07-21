"""Define calibration tools used on the digitization and reconstruction chains

Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HGTD_TdcCalibrationToolCfg(flags, name="HGTD_TdcCalibrationTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("PS_ActiveRange", 2.5)
    kwargs.setdefault("LHC_RiseEdge", 12.5)
    kwargs.setdefault("PS_LargeStep", 1.562)
    kwargs.setdefault("PS_SmallStep", 0.097)
    kwargs.setdefault("TOABinSize", 0.02)

    use_cond_db = kwargs.setdefault(
        "UseCondDB", flags.HGTD.Calibration.UseTdcConditions
    )

    if use_cond_db:
        acc.merge(HGTD_TdcCalibCondAlgCfg(flags))

    acc.setPrivateTools(CompFactory.HGTD_TdcCalibrationTool(name, **kwargs))
    return acc


def HGTD_TdcCalibCondAlgCfg(flags, name="HGTD_TdcCalibCondAlg", **kwargs):
    """Configure the TDC calibration conditions algorithm."""
    acc = ComponentAccumulator()

    folder = "/HGTD/Calibration/TdcBinSize"
    folder_db = flags.HGTD.Calibration.TdcCalibDb
    folder_tag = flags.HGTD.Calibration.TdcCalibTag

    kwargs.setdefault("ReadKey", folder)
    kwargs.setdefault("WriteKey", "HGTD_TdcCalibData")

    from IOVDbSvc.IOVDbSvcConfig import addFolders

    tag = folder_tag or None
    modifiers = ""
    if folder_db.startswith("crest_fs:"):
        if not folder_tag:
            raise ValueError(
                "HGTD.Calibration.TdcCalibTag must be set when using crest_fs"
            )
        # A per-folder CREST override uses <ctag> even when the global backend is COOL.
        modifiers = f"<ctag>{folder_tag}</ctag>"
        tag = None

    acc.merge(addFolders(flags, folder,
                         detDb=folder_db,
                         className="CondAttrListCollection",
                         tag=tag,
                         modifiers=modifiers))

    acc.addCondAlgo(CompFactory.HGTD_TdcCalibCondAlg(name, **kwargs))

    return acc


def HGTD_TimeResolutionToolCfg(flags, name="HGTD_TimeResolutionTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HGTD_TimeResolutionTool(name, **kwargs))
    return acc
