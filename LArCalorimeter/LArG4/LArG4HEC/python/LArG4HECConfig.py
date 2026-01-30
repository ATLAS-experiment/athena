# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArG4HEC import HECGeometryType


# Not used???
def LArHECLocalCalculatorCfg(flags, name="LArHECLocalCalculator", **kwargs):
    kwargs.setdefault("Birksk",0.0486) # this constant was not tuned for G4 10.6, so resetting back to the MC21 default value
    return CompFactory.LArHECLocalCalculator(name, **kwargs)


def LocalCalibrationCalculatorCfg(flags, name="LocalCalibrationCalculator", **kwargs):
    return CompFactory.LArG4.HEC.LocalCalibrationCalculator(name, **kwargs)


def LocalHECGeometry(name="LocalHECGeometry", **kwargs):
    return CompFactory.LArG4.HEC.LocalGeometry(name, **kwargs)


def HECWheelCalculatorCfg(flags, name="HECWheelCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("GeometryCalculator", result.getPrimaryAndMerge(HECGeometryCfg(flags)))
    kwargs.setdefault("Birksk",0.0486) # this constant was not tuned for G4 10.6, so resetting back to the MC21 default value
    result.addService(CompFactory.LArHECWheelCalculator(name, **kwargs), primary = True)
    return result


def LArHECCalibrationWheelCalculatorCfg(flags, name="LArHECCalibrationWheelCalculator", **kwargs):
    result = ComponentAccumulator()
    result.addService( CompFactory.LArG4.HEC.LArHECCalibrationWheelCalculator(name, **kwargs), primary = True )
    return result


def HECCalibrationWheelActiveCalculatorCfg(flags, name="HECCalibrationWheelActiveCalculator", **kwargs):
    kwargs.setdefault("GeometryType", HECGeometryType.kWheelActive)
    return LArHECCalibrationWheelCalculatorCfg(flags, name, **kwargs)


def HECCalibrationWheelInactiveCalculatorCfg(flags, name="HECCalibrationWheelInactiveCalculator", **kwargs):
    kwargs.setdefault("GeometryType", HECGeometryType.kWheelInactive)
    return LArHECCalibrationWheelCalculatorCfg(flags, name, **kwargs)


def HECCalibrationWheelDeadCalculatorCfg(flags, name="HECCalibrationWheelDeadCalculator", **kwargs):
    kwargs.setdefault("GeometryType", HECGeometryType.kWheelDead)
    return LArHECCalibrationWheelCalculatorCfg(flags, name, **kwargs)


def HECGeometryCfg(flags, name="HECGeometry", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.LArG4.HEC.HECGeometry(name, **kwargs), primary = True)
    return result
