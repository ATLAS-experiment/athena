# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def BarrelCryostatCalibrationCalculatorCfg(flags, name="BarrelCryostatCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.BarrelCryostat.CalibrationCalculator(name, **kwargs), primary=True )
    return result


def BarrelCryostatCalibrationLArCalculatorCfg(flags, name="BarrelCryostatCalibrationLArCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.BarrelCryostat.CalibrationLArCalculator(name, **kwargs), primary=True )
    return result


def BarrelCryostatCalibrationMixedCalculatorCfg(flags, name="BarrelCryostatCalibrationMixedCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.BarrelCryostat.CalibrationMixedCalculator(name, **kwargs), primary=True )
    return result


def DMCalibrationCalculatorCfg(flags, name="DMCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.DM.CalibrationCalculator(name, **kwargs), primary=True )
    return result


def BarrelCalibrationCalculatorCfg(flags, name="BarrelCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.Barrel.CalibrationCalculator(name, **kwargs), primary=True )
    return result


def BarrelPresamplerCalibrationCalculatorCfg(flags, name="BarrelPresamplerCalibrationCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.BarrelPresampler.CalibrationCalculator(name, **kwargs), primary=True )
    return result


def EMBCalculatorCfg(flags, name="EMBCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator", result.getPrimaryAndMerge(LArBarrelGeometryCfg(flags)))
    result.addService(CompFactory.LArBarrelCalculator(name, **kwargs), primary=True)
    return result


def EMBPresamplerCalculatorCfg(flags, name="EMBPresamplerCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator", result.getPrimaryAndMerge(LArBarrelPresamplerGeometryCfg(flags)))

    result.addService(CompFactory.LArBarrelPresamplerCalculator(name, **kwargs), primary=True)
    return result


def LArBarrelGeometryCfg(flags, name="LArBarrelGeometry", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.LArG4.Barrel.Geometry(name, **kwargs), primary=True)
    return result


def LArBarrelPresamplerGeometryCfg(flags, name="LArBarrelPresamplerGeometry", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.LArG4.BarrelPresampler.Geometry(name, **kwargs), primary=True)
    return result
