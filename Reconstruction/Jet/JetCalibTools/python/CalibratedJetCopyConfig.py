"""
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""

import re

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def sanitizeName(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_]", "_", name)


def getJetCalibrationTool(
    jetCollection: str,
    configFile: str,
    calibSequence: str,
    calibArea: str,
    calibrationScale: str,
    isData: bool,
    calibJetCollection: str | None,
    rhoKey: str | None,
    pvKey: str | None,
):
    """Build the JetCalibrationTool and return (tool, jet_container)."""

    if jetCollection.endswith("Jets"):
        jet_container = jetCollection
        jet_collection_nosuffix = jetCollection[:-4]
    else:
        jet_container = f"{jetCollection}Jets"
        jet_collection_nosuffix = jetCollection

    safe_collection = sanitizeName(jet_collection_nosuffix)
    safe_scale = sanitizeName(calibrationScale)

    calib_collection = calibJetCollection if calibJetCollection is not None else jet_collection_nosuffix

    tool_kwargs = dict(
        JetCollection=calib_collection,
        ConfigFile=configFile,
        CalibSequence=calibSequence,
        CalibArea=calibArea,
        IsData=isData,
    )
    if rhoKey is not None:
        tool_kwargs['RhoKey'] = rhoKey
    if pvKey is not None:
        tool_kwargs['PrimaryVerticesContainerName'] = pvKey

    jet_calib_tool = CompFactory.JetCalibrationTool(
        f"JetCalibrationTool_{safe_collection}_{safe_scale}",
        **tool_kwargs,
    )

    return jet_calib_tool, jet_container


def CalibratedJetCopyCfg(
    cfgFlags,
    jetCollection: str,
    outputCollection: str,
    configFile: str,
    calibSequence: str,
    calibArea: str,
    calibrationScale: str,
    isData: bool = False,
    calibJetCollection: str | None = None,
    rhoKey: str | None = None,
    pvKey: str | None = None,
) -> ComponentAccumulator:
    """Record a calibrated shallow copy of a jet collection.

    The copy is written to *outputCollection* and each input jet is
    decorated with an element link ('calibratedJetLink') to its copy.
    """

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

    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.CalibratedJetCopyAlg(
            f"CalibratedJetCopyAlg_{sanitizeName(outputCollection)}",
            JetCalibrationTool=jet_calib_tool,
            JetContainer=jet_container,
            OutputContainer=outputCollection,
        )
    )

    return acc
