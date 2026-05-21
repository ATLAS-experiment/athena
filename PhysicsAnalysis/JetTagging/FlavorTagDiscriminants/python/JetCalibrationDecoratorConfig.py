"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

import re

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def _sanitize(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_]", "_", name)


def JetCalibrationDecoratorCfg(
    cfgFlags,
    jetCollection: str,
    configFile: str,
    calibSequence: str,
    calibArea: str,
    calibrationScale: str,
    isData: bool = False,
) -> ComponentAccumulator:
    """Decorate jets with calibrated four-momentum components."""

    del cfgFlags

    if jetCollection.endswith("Jets"):
        jet_container = jetCollection
        jet_collection_nosuffix = jetCollection[:-4]
    else:
        jet_container = f"{jetCollection}Jets"
        jet_collection_nosuffix = jetCollection

    safe_collection = _sanitize(jet_collection_nosuffix)
    safe_scale = _sanitize(calibrationScale)

    jet_calib_tool = CompFactory.JetCalibrationTool(
        f"JetCalibrationTool_{safe_collection}_{safe_scale}",
        JetCollection=jet_collection_nosuffix,
        ConfigFile=configFile,
        CalibSequence=calibSequence,
        CalibArea=calibArea,
        IsData=isData,
    )

    acc = ComponentAccumulator()
    acc.addPublicTool(jet_calib_tool)
    acc.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.JetCalibrationDecoratorAlg(
            f"JetCalibrationDecoratorAlg_{_sanitize(jet_container)}_{safe_scale}",
            JetCalibrationTool=jet_calib_tool,
            JetContainer=jet_container,
            ptCalibratedKey=f"{jet_container}.{calibrationScale}_pt",
            etaCalibratedKey=f"{jet_container}.{calibrationScale}_eta",
            phiCalibratedKey=f"{jet_container}.{calibrationScale}_phi",
            massCalibratedKey=f"{jet_container}.{calibrationScale}_mass",
        )
    )

    return acc
