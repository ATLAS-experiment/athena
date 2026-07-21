"""Define calibration tools used on the digitization and reconstruction chains

Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""
import os

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HGTD_TdcCalibrationToolCfg(flags, name="HGTD_TdcCalibrationTool", **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("PS_ActiveRange", 2.5)
    kwargs.setdefault("LHC_RiseEdge", 12.5)
    kwargs.setdefault("PS_LargeStep", 1.562)
    kwargs.setdefault("PS_SmallStep", 0.097)
    kwargs.setdefault("TOABinSize", 0.02)

    use_crest_api_config = kwargs.pop(
        "UseCrestApiConfig",
        os.environ.get("HGTD_USE_CREST_API_CONFIG", "0") == "1",
    )
    if use_crest_api_config:
        from HGTD_Calibration.HGTD_CrestFetcher import fetch_hgtd_toa_bin_size_from_crest

        crest_api_url = kwargs.pop("CrestApiUrl", os.environ.get("CREST_API_URL"))
        crest_tag_name = kwargs.pop(
            "CrestTagName",
            os.environ.get("HGTD_CREST_TAG_NAME", "TEST-yvolkotr-HGTD-TdcCalibBinSize-01"),
        )
        crest_toa_bin_size = fetch_hgtd_toa_bin_size_from_crest(
            crest_api_url=crest_api_url,
            tag_name=crest_tag_name,
        )
        kwargs["TOABinSize"] = crest_toa_bin_size
        print(
            "HGTD_TdcCalibrationToolCfg: fetched TOABinSize = "
            f"{crest_toa_bin_size} ns from CREST tag {crest_tag_name}"
        )

    # Phase 1: Enable conditions DB integration
    # Set UseCondDB=True to read toa_bin_size from CREST/COOL
    # instead of using the hardcoded TOABinSize property above.
    use_cond_db = kwargs.pop("UseCondDB", os.environ.get("HGTD_USE_CONDDB", "0") == "1")
    if use_cond_db and use_crest_api_config:
        raise RuntimeError(
            "HGTD_TdcCalibrationToolCfg: UseCondDB and UseCrestApiConfig cannot be enabled "
            "at the same time. Use UseCrestApiConfig as the interim CREST path, or UseCondDB "
            "for the full CondAlg/CDO conditions path."
        )
    kwargs["UseCondDB"] = use_cond_db

    if use_cond_db:
        # Schedule the conditions algorithm that reads from the DB
        acc.merge(HGTD_TdcCalibCondAlgCfg(flags))

    acc.setPrivateTools(CompFactory.HGTD_TdcCalibrationTool(name, **kwargs))
    return acc


def HGTD_TdcCalibCondAlgCfg(flags, name="HGTD_TdcCalibCondAlg", **kwargs):
    """Configure the conditions algorithm that reads TDC calibration
    data from CREST/COOL and produces HGTD_TdcCalibData."""
    acc = ComponentAccumulator()

    folder = "/HGTD/Calibration/TdcBinSize"
    kwargs.setdefault("ReadKey", folder)
    kwargs.setdefault("WriteKey", "HGTD_TdcCalibData")

    acc.addCondAlgo(CompFactory.HGTD_TdcCalibCondAlg(name, **kwargs))

    # ── Register the conditions folder with IOVDbSvc ──
    from IOVDbSvc.IOVDbSvcConfig import addFolders

    if flags.IOVDb.UseCREST:
        # Option B: CREST server backend.
        # The folder name maps to the CREST tag label.
        # IOVDbSvc.Source is set to 'CREST' globally when UseCREST=True.
        # No detDb needed — CREST resolves the folder via the global tag.
        acc.merge(addFolders(flags, folder,
                             className="AthenaAttributeList"))
    else:
        # Option A: Local SQLite file (for testing without CREST server).
        # Create the SQLite file with create_hgtd_calib_sqlite.py first.
        import os
        sqliteFile = kwargs.pop("SqliteFile",
                                os.path.join(os.getcwd(), "HGTD_TdcCalib.db"))
        acc.merge(addFolders(flags, folder,
                             detDb=sqliteFile,
                             db="OFLP200",
                             tag="HGTDCalibTdcBinSize-MC-01",
                             className="AthenaAttributeList"))

    return acc


def HGTD_TimeResolutionToolCfg(flags, name="HGTD_TimeResolutionTool", **kwargs):
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HGTD_TimeResolutionTool(name, **kwargs))
    return acc
