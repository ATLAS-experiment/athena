"""Define functions to configure Pixel conditions algorithms

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from IOVDbSvc.IOVDbSvcConfig import addFolders, addFoldersSplitOnline

def ITkPixelModuleConfigCondAlgCfg(flags, name="ITkPixelModuleConfigCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelConfigCondAlg for ITk"""
    acc = ComponentAccumulator()
    kwargs.setdefault("WriteKey", "ITkPixelModuleData")
    #These settings should be replaced with values taken from conditions
    CondArgs = {}
    CondArgs.update(
            BarrelToTThreshold       = [-1,-1,-1,-1,-1],
            BarrelCrossTalk          = [  0.06,  0.06,  0.06,  0.06,  0.06],
            BarrelNoiseOccupancy     = [  5e-8,  5e-8,  5e-8,  5e-8,  5e-8],
            BarrelDisableProbability = [  9e-3,  9e-3,  9e-3,  9e-3,  9e-3],
            BarrelLorentzAngleCorr   = [   1.0,   1.0,   1.0,   1.0,   1.0],
            DefaultBarrelBiasVoltage = [ 150.0, 150.0, 150.0, 150.0, 150.0],
            BarrelFluence            = [0.0e14,0.0e14,0.0e14,0.0e14,0.0e14],
            BarrelFluenceMap         = ["PixelDigitization/maps_IBL_PL_80V_fl0e14.root",
                                        "PixelDigitization/maps_IBL_PL_80V_fl0e14.root",
                                        "PixelDigitization/maps_IBL_PL_80V_fl0e14.root",
                                        "PixelDigitization/maps_IBL_PL_80V_fl0e14.root",
                                        "PixelDigitization/maps_IBL_PL_80V_fl0e14.root"],
            EndcapToTThreshold       = [-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1],
            EndcapCrossTalk          = [ 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06, 0.06],
            EndcapNoiseOccupancy     = [ 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8, 5e-8],
            EndcapDisableProbability = [ 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3, 9e-3],
            EndcapLorentzAngleCorr   = [  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0],
            InnermostNoiseShape      = [0.0, 1.0],
            NextInnermostNoiseShape  = [0.0, 1.0],
            PixelNoiseShape          = [0.0, 1.0],
            # charge calib
            DefaultBarrelAnalogThreshold = [900, 600, 600, 600, 600],
            DefaultEndcapAnalogThreshold = [600, 600, 600, 600, 600, 600, 600, 600, 600],
            DefaultBarrelAnalogThresholdSigma = [36, 24, 24, 24, 24],
            DefaultEndcapAnalogThresholdSigma = [24, 24, 24, 24, 24, 24, 24, 24, 24],
            DefaultBarrelAnalogThresholdNoise = [110, 75, 75, 75, 75],
            DefaultEndcapAnalogThresholdNoise = [75, 75, 75, 75, 75, 75, 75, 75, 75],
            DefaultBarrelInTimeThreshold = [1000, 1000, 1000, 1000, 1000],
            DefaultEndcapInTimeThreshold = [1500, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000],
            DefaultCalibrationParameterA = 14.0,
            DefaultCalibrationParameterE = -1000.0,
            DefaultCalibrationParameterC = 8000.0,
        )
    CondArgs.update(kwargs)
    acc.addCondAlgo(CompFactory.PixelModuleConfigCondAlg(name, **CondArgs))
    return acc

def ITkPixelAlignCondAlgCfg(flags, name="ITkPixelAlignCondAlg",setAlignmentFolderName="/Indet/Align", **kwargs):
    """Return a ComponentAccumulator with configured PixelAlignCondAlg for ITk"""
    acc = ComponentAccumulator()

    if flags.GeoModel.Align.Dynamic:
        raise RuntimeError("Dynamic alignment not supported for ITk yet")
    else:
        acc.merge(addFoldersSplitOnline(flags, "INDET", "/Indet/Onl/Align", setAlignmentFolderName, className="AlignableTransformContainer"))

    kwargs.setdefault("DetManagerName", "ITkPixel")
    kwargs.setdefault("UseDynamicAlignFolders", flags.GeoModel.Align.Dynamic)
    kwargs.setdefault("ReadKeyStatic", setAlignmentFolderName)
    # kwargs.setdefault("ReadKeyDynamicL1", "/Indet/AlignL1/ID")
    # kwargs.setdefault("ReadKeyDynamicL2", "/Indet/AlignL2/PIX")
    # kwargs.setdefault("ReadKeyDynamicL3", "/Indet/AlignL3")
    kwargs.setdefault("ReadKeyIBLDist", "")
    kwargs.setdefault("WriteKey", "ITkPixelAlignmentStore")

    acc.addCondAlgo(CompFactory.PixelAlignCondAlg(name, **kwargs))
    return acc

def ITkPixelChargeCalibCondAlgCfg(flags, name="ITkPixelChargeCalibCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelChargeCalibCondAlg for ITk"""
    acc = ComponentAccumulator()
    acc.merge(ITkPixelModuleConfigCondAlgCfg(flags))
    folderName = ""
    if flags.ITk.Conditions.PixelChargeCalibTag:
        folderName = "/ITk/PixelChargeCalib"
        if flags.ITk.Conditions.PixelChargeCalibFile:
            acc.merge(addFolders(flags, folderName, flags.ITk.Conditions.PixelChargeCalibFile, tag=flags.ITk.Conditions.PixelChargeCalibTag, db="OFLP200", className="CondAttrListCollection"))
        else:
            acc.merge(addFolders(flags, folderName, "INDET_OFL", tag=flags.ITk.Conditions.PixelChargeCalibTag, db="OFLP200", className="CondAttrListCollection"))
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    kwargs.setdefault("PixelDetEleCollKey", "ITkPixelDetectorElementCollection")
    kwargs.setdefault("PixelModuleData", "ITkPixelModuleData")
    kwargs.setdefault("ReadKey", folderName)
    kwargs.setdefault("WriteKey", "ITkPixelChargeCalibCondData")
    acc.addCondAlgo(CompFactory.PixelChargeLUTCalibCondAlg(name, **kwargs))
    return acc

def ITkPixelDCSCondHVAlgCfg(flags, name="ITkPixelDCSCondHVAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDCSCondHVAlg for ITk"""
    acc = ComponentAccumulator()
    acc.merge(ITkPixelModuleConfigCondAlgCfg(flags))
    kwargs.setdefault("ReadKey", "")  # disable for ITk for now
    kwargs.setdefault("PixelModuleData", "ITkPixelModuleData")
    kwargs.setdefault("WriteKey", "ITkPixelDCSHVCondData")
    acc.addCondAlgo(CompFactory.PixelDCSCondHVAlg(name, **kwargs))
    return acc

def ITkPixelDCSCondStateAlgCfg(flags, name="ITkPixelDCSCondStateAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDCSCondStateAlg for ITk"""
    acc = ComponentAccumulator()
    kwargs.setdefault("ReadKeyState", "") #To be configured when final DCS implementation for ITk becomes available
    kwargs.setdefault("WriteKeyState", "ITkPixelDCSStateCondData")
    acc.addCondAlgo(CompFactory.PixelDCSCondStateAlg(name, **kwargs))
    return acc

def ITkPixelDCSCondStatusAlgCfg(flags, name="ITkPixelDCSCondStatusAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDCSCondStatusAlg for ITk"""
    acc = ComponentAccumulator()
    kwargs.setdefault("ReadKeyStatus", "") #To be configured when final DCS implementation for ITk becomes available
    kwargs.setdefault("WriteKeyStatus", "ITkPixelDCSStatusCondData")
    acc.addCondAlgo(CompFactory.PixelDCSCondStatusAlg(name, **kwargs))
    return acc

def ITkPixelDCSCondTempAlgCfg(flags, name="ITkPixelDCSCondTempAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDCSCondTempAlg for ITk"""
    acc = ComponentAccumulator()
    acc.merge(ITkPixelModuleConfigCondAlgCfg(flags))
    kwargs.setdefault("ReadKey", "")  # disable for ITk for now
    kwargs.setdefault("WriteKey", "ITkPixelDCSTempCondData")
    acc.addCondAlgo(CompFactory.PixelDCSCondTempAlg(name, **kwargs))
    return acc

def ITkPixelDeadMapCondAlgCfg(flags, name="ITkPixelDeadMapCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDeadMapCondAlg for ITk"""
    acc = ComponentAccumulator()
    acc.merge(ITkPixelModuleConfigCondAlgCfg(flags))

    # TODO: not enabled for ITk for now
    kwargs.setdefault("ReadKey", "")
    # Use a json file to disable modules. If readKey!="" this option is ignored
    kwargs.setdefault("JsonPath",flags.ITk.JsonPathPixelModuleVeto)
    kwargs.setdefault("WriteKey", "ITkPixelDeadMapCondData")
    acc.addCondAlgo(CompFactory.PixelDeadMapCondAlg(name, **kwargs))
    return acc

def ITkPixelDetectorElementCondAlgCfg(flags, name="ITkPixelDetectorElementCondAlg",setAlignmentFolderName="/Indet/Align", **kwargs):
    """Return a ComponentAccumulator with configured PixelDetectorElementCondAlg for ITk"""
    acc = ComponentAccumulator()
    acc.merge(ITkPixelAlignCondAlgCfg(flags,setAlignmentFolderName=setAlignmentFolderName))
    kwargs.setdefault("DetManagerName", "ITkPixel")
    kwargs.setdefault("PixelAlignmentStore", "ITkPixelAlignmentStore")
    kwargs.setdefault("WriteKey", "ITkPixelDetectorElementCollection")
    acc.addCondAlgo(CompFactory.PixelDetectorElementCondAlg(name, **kwargs))
    return acc

def ITkPixelDistortionAlgCfg(flags, name="ITkPixelDistortionAlg", **kwargs):
    """Return a ComponentAccumulator with configured PixelDistortionAlg for ITk"""
    acc = ComponentAccumulator()
    acc.merge(ITkPixelModuleConfigCondAlgCfg(flags))
    acc.merge(addFoldersSplitOnline(flags,"INDET", "/Indet/Onl/PixelDist", "/Indet/PixelDist", className="DetCondCFloat"))
    kwargs.setdefault("ReadKey", "/Indet/PixelDist")
    kwargs.setdefault("WriteKey", "ITkPixelDistortionData")
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)).name)
    acc.addCondAlgo(CompFactory.PixelDistortionAlg(name, **kwargs))
    return acc

def ITkPixelOfflineCalibCondAlgCfg(flags, name="ITkPixelOfflineCalibCondAlg", **kwargs):
    """Return a ComponentAccumulator with configured ITkPixelOfflineCalibCondAlg"""
    acc = ComponentAccumulator()

    folderName = ""
    if flags.ITk.Conditions.PixelOfflineCalibTag:
        folderName = '/PIXEL/ITkClusterError'
        CalibTag = flags.ITk.Conditions.PixelOfflineCalibTag     
        if flags.ITk.Conditions.PixelOfflineCalibFile:
            acc.merge(addFolders(flags, folderName, flags.ITk.Conditions.PixelOfflineCalibFile, tag=CalibTag, db="OFLP200", className="CondAttrListCollection"))
        else:
            acc.merge(addFolders(flags, folderName, "PIXEL_OFL", tag=CalibTag, db="OFLP200", className="CondAttrListCollection"))


    # TODO: enable once in the DB
    # else:
    #    acc.merge(addFolders(flags, "/PIXEL/ITkClusterError", "PIXEL_OFL", db="OFLP200", className="CondAttrListCollection"))

    kwargs.setdefault("ReadKey", "/PIXEL/ITkClusterError")
    kwargs.setdefault("WriteKey", "ITkPixelOfflineCalibData")
    kwargs.setdefault("InputSource", 2)
    acc.addCondAlgo(CompFactory.ITk.PixelOfflineCalibCondAlg(name, **kwargs))
    return acc
