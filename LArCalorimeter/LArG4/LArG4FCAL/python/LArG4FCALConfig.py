# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import mm,deg,ns
import math


def FCALCalculatorBaseCfg(name="FCALCalculatorBase", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("OOTcut",2.5*ns)
    kwargs.setdefault("Birksk",0.0486) # this constant was not tuned for G4 10.6, so resetting back to the MC21 default value
    result.addService(CompFactory.LArFCALCalculatorBase(name, **kwargs), primary = True)
    return result


def FCAL1CalculatorCfg(flags, name="FCAL1Calculator", **kwargs):
    kwargs.setdefault("FCALSampling",1)
    return FCALCalculatorBaseCfg(name, **kwargs)


def FCAL2CalculatorCfg(flags, name="FCAL2Calculator", **kwargs):
    kwargs.setdefault("FCALSampling",2)
    return FCALCalculatorBaseCfg(name, **kwargs)


def FCAL3CalculatorCfg(flags, name="FCAL3Calculator", **kwargs):
    kwargs.setdefault("FCALSampling",3)
    return FCALCalculatorBaseCfg(name, **kwargs)


def FCAL1CalibCalculatorCfg(flags, name="FCAL1CalibCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FCALdeltaX",7.5*mm)
    kwargs.setdefault("FCALdeltaY",7.5*mm*math.sin(60*deg))
    kwargs.setdefault("FCALSampling",1)
    result.addService(CompFactory.LArG4.FCAL.LArFCALCalibCalculatorBase(name, **kwargs), primary = True)
    return result


def FCAL2CalibCalculatorCfg(flags, name="FCAL2CalibCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FCALdeltaX",8.179*mm)
    kwargs.setdefault("FCALdeltaY",8.179*mm*math.sin(60*deg))
    kwargs.setdefault("FCALSampling",2)
    result.addService(CompFactory.LArG4.FCAL.LArFCALCalibCalculatorBase(name, **kwargs), primary = True)
    return result


def FCAL3CalibCalculatorCfg(flags, name="FCAL3CalibCalculator", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FCALdeltaX",9.0*mm)
    kwargs.setdefault("FCALdeltaY",9.0*mm*math.sin(60*deg))
    kwargs.setdefault("FCALSampling",3)
    result.addService(CompFactory.LArG4.FCAL.LArFCALCalibCalculatorBase(name, **kwargs), primary = True)
    return result
