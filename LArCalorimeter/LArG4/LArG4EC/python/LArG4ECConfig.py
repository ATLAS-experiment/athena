# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArG4EC import LArWheelCalculatorEnum


def CalibrationCalculatorCfg(name="CalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.LArG4.EC.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EMECPosInnerWheelCalibrationCalculatorCfg(flags, name="EMECPosInnerWheelCalibrationCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.InnerAbsorberWheel)
    kwargs.setdefault("zSide", 1)
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator",result.getPrimaryAndMerge(EMECPosInnerWheelCorrOffCalculatorCfg(flags)))
    result.addService(CompFactory.LArG4.EC.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EMECNegInnerWheelCalibrationCalculatorCfg(flags, name="EMECNegInnerWheelCalibrationCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.InnerAbsorberWheel)
    kwargs.setdefault("zSide", -1)
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator",result.getPrimaryAndMerge(EMECNegInnerWheelCorrOffCalculatorCfg(flags)))
    result.addService(CompFactory.LArG4.EC.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EMECPosOuterWheelCalibrationCalculatorCfg(flags, name="EMECPosOuterWheelCalibrationCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.OuterAbsorberWheel)
    kwargs.setdefault("zSide", 1)
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator",result.getPrimaryAndMerge(EMECPosOuterWheelCorrOffCalculatorCfg(flags)))
    result.addService(CompFactory.LArG4.EC.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EMECNegOuterWheelCalibrationCalculatorCfg(flags, name="EMECNegOuterWheelCalibrationCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.OuterAbsorberWheel)
    kwargs.setdefault("zSide", -1)
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator",result.getPrimaryAndMerge(EMECNegOuterWheelCorrOffCalculatorCfg(flags)))
    result.addService(CompFactory.LArG4.EC.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EMECPosBackOuterBarretteCalibrationCalculatorCfg(flags, name="EMECPosBackOuterBarretteCalibrationCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.BackOuterBarretteWheelCalib)
    kwargs.setdefault("zSide", 1)
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator",result.getPrimaryAndMerge(EMECPosBackOuterBarretteCorrOffCalculatorCfg(flags)))
    result.addService(CompFactory.LArG4.EC.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EMECNegBackOuterBarretteCalibrationCalculatorCfg(flags, name="EMECNegBackOuterBarretteCalibrationCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.BackOuterBarretteWheelCalib)
    kwargs.setdefault("zSide", -1)
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator",result.getPrimaryAndMerge(EMECNegBackOuterBarretteCorrOffCalculatorCfg(flags)))
    result.addService(CompFactory.LArG4.EC.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EMECPresamplerCalibrationCalculatorCfg(flags, name="EMECPresamplerCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.LArG4.EC.PresamplerCalibrationCalculator(name, **kwargs), primary=True)
    return result

def EndcapCryostatCalibrationCalculatorCfg(flags, name="EndcapCryostatCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.EndcapCryostat.CalibrationCalculator(name, **kwargs), primary=True)
    return result

def EndcapCryostatCalibrationLArCalculatorCfg(flags, name="EndcapCryostatCalibrationLArCalculator", **kwargs):
    result = ComponentAccumulator()
    from LArG4SD.LArG4SDToolConfig import CalibrationDefaultCalculatorCfg
    kwargs.setdefault("CalibrationDefaultCalculator", result.getPrimaryAndMerge(CalibrationDefaultCalculatorCfg(flags)))
    result.addService( CompFactory.LArG4.EndcapCryostat.CalibrationLArCalculator(name, **kwargs), primary=True)
    return result

def EndcapCryostatCalibrationMixedCalculatorCfg(flags, name="EndcapCryostatCalibrationMixedCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.EndcapCryostat.CalibrationMixedCalculator(name, **kwargs), primary=True)
    return result

def EMECSupportCalibrationCalculatorCfg(flags, name="EMECSupportCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("BackupCalculator", result.getPrimaryAndMerge(EndcapCryostatCalibrationLArCalculatorCfg(flags)))
    result.addService( CompFactory.LArG4.EMECSupportCalibrationCalculator(name, **kwargs), primary=True)
    return result

def EnergyCalculatorCfg(flags, name="EnergyCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("SupportCalculator", result.getPrimaryAndMerge(EMECSupportCalibrationCalculatorCfg(flags)))
    from AthenaCommon.SystemOfUnits import ns
    kwargs.setdefault("OOTcut", 300.0*ns)

    result.addService(CompFactory.LArG4.EC.EnergyCalculator(name, **kwargs), primary=True)
    return result

def EMECPosInnerWheelCalculatorCfg(flags, name="EMECPosInnerWheelCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.InnerAbsorberWheel)
    #kwargs.setdefault("EnergyCorrection", 8) #LArG4::EMEC_ECOR_CHCL1
    kwargs.setdefault("zSide", 1)
    return EnergyCalculatorCfg(flags, name, **kwargs)

def EMECNegInnerWheelCalculatorCfg(flags, name="EMECNegInnerWheelCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.InnerAbsorberWheel)
    #kwargs.setdefault("EnergyCorrection", 8) #LArG4::EMEC_ECOR_CHCL1
    kwargs.setdefault("zSide", -1)
    return EnergyCalculatorCfg(flags, name, **kwargs)

def EMECPosOuterWheelCalculatorCfg(flags, name="EMECPosOuterWheelCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.OuterAbsorberWheel)
    #kwargs.setdefault("EnergyCorrection", 8) #LArG4::EMEC_ECOR_CHCL1
    kwargs.setdefault("zSide", 1)
    return EnergyCalculatorCfg(flags, name, **kwargs)

def EMECNegOuterWheelCalculatorCfg(flags, name="EMECNegOuterWheelCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.OuterAbsorberWheel)
    #kwargs.setdefault("EnergyCorrection", 8) #LArG4::EMEC_ECOR_CHCL1
    kwargs.setdefault("zSide", -1)
    return EnergyCalculatorCfg(flags, name, **kwargs)

def EMECPosBackOuterBarretteCalculatorCfg(flags, name="EMECPosBackOuterBarretteCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.BackOuterBarretteWheel)
    #kwargs.setdefault("EnergyCorrection", 8) #LArG4::EMEC_ECOR_CHCL1
    return EnergyCalculatorCfg(flags, name, **kwargs)

def EMECNegBackOuterBarretteCalculatorCfg(flags, name="EMECNegBackOuterBarretteCalculator", **kwargs):
    kwargs.setdefault("WheelType", LArWheelCalculatorEnum.BackOuterBarretteWheel)
    #kwargs.setdefault("EnergyCorrection", 8) #LArG4::EMEC_ECOR_CHCL1
    kwargs.setdefault("zSide", -1)
    return EnergyCalculatorCfg(flags, name, **kwargs)

def EMECPosInnerWheelCorrOffCalculatorCfg(flags, name="EMECPosInnerWheelCorrOffCalculator", **kwargs):
    kwargs.setdefault("EnergyCorrection", 1) #LArG4::EMEC_ECOR_OFF
    return EMECPosInnerWheelCalculatorCfg(flags, name, **kwargs)

def EMECNegInnerWheelCorrOffCalculatorCfg(flags, name="EMECNegInnerWheelCorrOffCalculator", **kwargs):
    kwargs.setdefault("EnergyCorrection", 1) #LArG4::EMEC_ECOR_OFF
    return EMECNegInnerWheelCalculatorCfg(flags, name, **kwargs)

def EMECPosOuterWheelCorrOffCalculatorCfg(flags, name="EMECPosOuterWheelCorrOffCalculator", **kwargs):
    kwargs.setdefault("EnergyCorrection", 1) #LArG4::EMEC_ECOR_OFF
    return EMECPosOuterWheelCalculatorCfg(flags, name, **kwargs)

def EMECNegOuterWheelCorrOffCalculatorCfg(flags, name="EMECNegOuterWheelCorrOffCalculator", **kwargs):
    kwargs.setdefault("EnergyCorrection", 1) #LArG4::EMEC_ECOR_OFF
    return EMECNegOuterWheelCalculatorCfg(flags, name, **kwargs)

def EMECPosBackOuterBarretteCorrOffCalculatorCfg(flags, name="EMECPosBackOuterBarretteCorrOffCalculator", **kwargs):
    kwargs.setdefault("EnergyCorrection", 1) #LArG4::EMEC_ECOR_OFF
    return EMECPosBackOuterBarretteCalculatorCfg(flags, name, **kwargs)

def EMECNegBackOuterBarretteCorrOffCalculatorCfg(flags, name="EMECNegBackOuterBarretteCorrOffCalculator", **kwargs):
    kwargs.setdefault("EnergyCorrection", 1) #LArG4::EMEC_ECOR_OFF
    return EMECNegBackOuterBarretteCalculatorCfg(flags, name, **kwargs)

def EMECPresamplerCalculatorCfg(flags, name="EMECPresamplerCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator",result.getPrimaryAndMerge(EMECPresamplerGeometryCfg(flags)))
    result.addService(CompFactory.LArEndcapPresamplerCalculator(name, **kwargs), primary=True)
    return result

def EMECPresamplerGeometryCfg(flags, name="EMECPresamplerGeometry", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.LArG4.EC.PresamplerGeometry(name, **kwargs), primary = True)
    return result
