"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from JetCalibTools.CalibratedJetCopyConfig import getJetCalibrationTool, sanitizeName


def JetCalibrationDecoratorCfg(
    cfgFlags,
    jetCollection: str,
    configFile: str,
    calibSequence: str,
    calibArea: str,
    calibrationScale: str,
    isData: bool = False,
    calibJetCollection: str | None = None,
    rhoKey: str | None = None,
    pvKey: str | None = None,
) -> ComponentAccumulator:
    """Decorate jets with calibrated four-momentum components."""

    jet_calib_tool, jet_container = getJetCalibrationTool(
        jetCollection=jetCollection,
        configFile=configFile,
        calibSequence=calibSequence,
        calibArea=calibArea,
        calibrationScale=calibrationScale,
        isData=isData,
        calibJetCollection=calibJetCollection,
        rhoKey=rhoKey,
        pvKey=pvKey,
    )
    safe_scale = sanitizeName(calibrationScale)

    acc = ComponentAccumulator()
    acc.addPublicTool(jet_calib_tool)
    acc.addEventAlgo(
        CompFactory.JetCalibrationDecoratorAlg(
            f"JetCalibrationDecoratorAlg_{sanitizeName(jet_container)}_{safe_scale}",
            JetCalibrationTool=jet_calib_tool,
            JetContainer=jet_container,
            ptCalibratedKey=f"{calibrationScale}_pt",
            etaCalibratedKey=f"{calibrationScale}_eta",
            phiCalibratedKey=f"{calibrationScale}_phi",
            massCalibratedKey=f"{calibrationScale}_mass",
        )
    )

    return acc
