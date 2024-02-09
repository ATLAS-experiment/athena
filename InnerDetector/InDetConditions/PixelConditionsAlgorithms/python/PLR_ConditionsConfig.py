"""Define functions to configure PLR conditions algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFoldersSplitOnline


def PLR_ConfigCondAlgCfg(flags, name="PLR_ConfigCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelConfigCondAlg for PLR"""
    kwargs.setdefault("WriteKey", "PLR_ModuleData")
    # This one has the highest chance of premature divergence so just take the default Pixel for now
    from PixelConditionsAlgorithms.PixelConditionsConfig import PixelConfigCondAlgCfg
    return PixelConfigCondAlgCfg(flags, name, **kwargs)


def PLR_AlignCondAlgCfg(flags, name="PLR_AlignCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelAlignCondAlg for PLR"""
    acc = ComponentAccumulator()

    if flags.GeoModel.Align.Dynamic:
        raise RuntimeError("Dynamic alignment not supported for PLR yet")
    else:
        acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", "/Indet/Align", className="AlignableTransformContainer"))

    kwargs.setdefault("DetManagerName", "PLR")
    kwargs.setdefault("UseDynamicAlignFolders", flags.GeoModel.Align.Dynamic)
    kwargs.setdefault("ReadKeyStatic", "/Indet/Align")
    # kwargs.setdefault("ReadKeyDynamicL1", "/Indet/AlignL1/ID")
    # kwargs.setdefault("ReadKeyDynamicL2", "/Indet/AlignL2/PIX")
    # kwargs.setdefault("ReadKeyDynamicL3", "/Indet/AlignL3")
    kwargs.setdefault("ReadKeyIBLDist", "")
    kwargs.setdefault("WriteKey", "PLR_AlignmentStore")

    acc.addCondAlgo(CompFactory.PixelAlignCondAlg(name, **kwargs))
    return acc


def PLR_ChargeCalibCondAlgCfg(flags, name="PLR_ChargeCalibCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelChargeCalibCondAlg for PLR"""
    acc = ComponentAccumulator()
    acc.merge(PLR_ConfigCondAlgCfg(flags))

    folderName = ""  # disabled for now

    from PLRGeoModelXml.PLR_GeoModelConfig import PLR_ReadoutGeometryCfg
    acc.merge(PLR_ReadoutGeometryCfg(flags))

    kwargs.setdefault("PixelIDName", "PLR_ID")
    kwargs.setdefault("PixelDetEleCollKey", "PLR_DetectorElementCollection")
    kwargs.setdefault("PixelModuleData", "PLR_ModuleData")
    kwargs.setdefault("ReadKey", folderName)
    kwargs.setdefault("WriteKey", "PLR_ChargeCalibCondData")
    acc.addCondAlgo(CompFactory.PixelChargeLUTCalibCondAlg(name, **kwargs))
    return acc


def PLR_DCSCondHVAlgCfg(flags, name="PLR_DCSCondHVAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDCSCondHVAlg for PLR"""
    acc = ComponentAccumulator()
    acc.merge(PLR_ConfigCondAlgCfg(flags))
    kwargs.setdefault("ReadKey", "")  # disabled for now
    kwargs.setdefault("PixelModuleData", "PLR_ModuleData")
    kwargs.setdefault("WriteKey", "PLR_DCSHVCondData")
    acc.addCondAlgo(CompFactory.PixelDCSCondHVAlg(name, **kwargs))
    return acc


def PLR_DCSCondTempAlgCfg(flags, name="PLR_DCSCondTempAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDCSCondTempAlg for PLR"""
    acc = ComponentAccumulator()
    acc.merge(PLR_ConfigCondAlgCfg(flags))
    kwargs.setdefault("ReadKey", "")  # disabled for now
    kwargs.setdefault("WriteKey", "PLR_DCSTempCondData")
    acc.addCondAlgo(CompFactory.PixelDCSCondTempAlg(name, **kwargs))
    return acc


def PLR_DetectorElementCondAlgCfg(flags, name="PLR_DetectorElementCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDetectorElementCondAlg for PLR"""
    acc = ComponentAccumulator()
    acc.merge(PLR_AlignCondAlgCfg(flags))
    kwargs.setdefault("DetManagerName", "PLR")
    kwargs.setdefault("PixelAlignmentStore", "PLR_AlignmentStore")
    kwargs.setdefault("WriteKey", "PLR_DetectorElementCollection")
    acc.addCondAlgo(CompFactory.PixelDetectorElementCondAlg(name, **kwargs))
    return acc


def PLR_DistortionAlgCfg(flags, name="PLR_DistortionAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDistortionAlg for PLR"""
    acc = ComponentAccumulator()
    acc.merge(PLR_ConfigCondAlgCfg(flags))
    acc.merge(addFoldersSplitOnline(flags,"INDET", "/Indet/Onl/PixelDist", "/Indet/PixelDist", className="DetCondCFloat"))
    kwargs.setdefault("ReadKey", "/Indet/PixelDist")
    kwargs.setdefault("WriteKey", "PLR_DistortionData")
    acc.addCondAlgo(CompFactory.PixelDistortionAlg(name, **kwargs))
    return acc
