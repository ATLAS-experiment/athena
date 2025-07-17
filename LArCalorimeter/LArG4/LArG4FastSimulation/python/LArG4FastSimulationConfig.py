# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def LArG4ShowerLibSvcCfg(flags, **kwargs):
    result = ComponentAccumulator()
    fileNameList = []
    #TODO make this configurable based on flags?
    # FCAL1 frozen shower libraries
    fileNameList += [ "LArG4ShowerLibData/MC23_v2/LArG4ShowerLib.FCAL1.11.root",
                      "LArG4ShowerLibData/MC23_v2/LArG4ShowerLib.FCAL1.22.root",
                      "LArG4ShowerLibData/MC23_v2/LArG4ShowerLib.FCAL1.2112.root"]
    # FCAL2 frozen shower libraries
    fileNameList += [ "LArG4ShowerLibData/MC23_v2/LArG4ShowerLib.FCAL2.11.root",
                      "LArG4ShowerLibData/MC23_v2/LArG4ShowerLib.FCAL2.22.root",
                      "LArG4ShowerLibData/MC23_v2/LArG4ShowerLib.FCAL2.2112.root"]
    kwargs.setdefault("FileNameList", fileNameList)
    result.addService(CompFactory.LArG4ShowerLibSvc(name="LArG4ShowerLibSvc", **kwargs))
    return result


def EMBFastShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    result.merge(LArG4ShowerLibSvcCfg(flags))
    kwargs.setdefault("RegionName",        "EMB")
    kwargs.setdefault("EFlagToShowerLib",   False)
    kwargs.setdefault("GFlagToShowerLib",   False)
    kwargs.setdefault("NeutFlagToShowerLib",False)
    kwargs.setdefault("PionFlagToShowerLib",False)
    kwargs.setdefault("ContainLow",         True)
    kwargs.setdefault("AbsLowEta",          0.3)
    kwargs.setdefault("ContainHigh",        True)
    kwargs.setdefault("AbsHighEta",         1.1)
    kwargs.setdefault("ContainCrack",       True)
    kwargs.setdefault("AbsCrackEta1",       0.5)
    kwargs.setdefault("AbsCrackEta2",       1.1)
    kwargs.setdefault("DetectorTag",        100000)
    kwargs.setdefault("SensitiveDetector",  "BarrelFastSimDedicatedSD")
    kwargs.setdefault("EMinEneShowerLib",   0.51)
    result.setPrivateTools(CompFactory.LArFastShowerTool(name="EMBFastShower", **kwargs))
    return result


def EMECFastShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    result.merge(LArG4ShowerLibSvcCfg(flags))
    kwargs.setdefault("RegionName",        "EMECPara")
    kwargs.setdefault("EFlagToShowerLib",   False)
    kwargs.setdefault("GFlagToShowerLib",   False)
    kwargs.setdefault("NeutFlagToShowerLib",False)
    kwargs.setdefault("PionFlagToShowerLib",False)
    kwargs.setdefault("ContainLow",         True)
    kwargs.setdefault("AbsLowEta",          1.8)
    kwargs.setdefault("ContainHigh",        True)
    kwargs.setdefault("AbsHighEta",         2.9)
    kwargs.setdefault("ContainCrack",       True)
    kwargs.setdefault("AbsCrackEta1",       2.2)
    kwargs.setdefault("AbsCrackEta2",       2.8)
    kwargs.setdefault("DetectorTag",        200000)
    kwargs.setdefault("SensitiveDetector", "EndcapFastSimDedicatedSD")
    kwargs.setdefault("EMinEneShowerLib",   0.51)
    result.setPrivateTools(CompFactory.LArFastShowerTool(name="EMECFastShower", **kwargs))
    return result


def FCALFastShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    result.merge(LArG4ShowerLibSvcCfg(flags))
    kwargs.setdefault("RegionName",        "FCALPara")
    kwargs.setdefault("EFlagToShowerLib",   True)
    kwargs.setdefault("GFlagToShowerLib",   True)
    kwargs.setdefault("NeutFlagToShowerLib",True)
    kwargs.setdefault("PionFlagToShowerLib",False)
    kwargs.setdefault("ContainLow",         False)
    kwargs.setdefault("AbsLowEta",          4.0)
    kwargs.setdefault("ContainHigh",        False)
    kwargs.setdefault("AbsHighEta",         4.6)
    kwargs.setdefault("DetectorTag",        300000)
    kwargs.setdefault("SensitiveDetector", "FCALFastSimDedicatedSD")
    kwargs.setdefault("EMinEneShowerLib",   3.0)
    result.setPrivateTools(CompFactory.LArFastShowerTool(name="FCALFastShower", **kwargs))
    return result


def FCAL2FastShowerCfg(flags, **kwargs):
    result = ComponentAccumulator()
    result.merge(LArG4ShowerLibSvcCfg(flags))
    kwargs.setdefault("RegionName",        "FCAL2Para")
    kwargs.setdefault("EFlagToShowerLib",   True)
    kwargs.setdefault("GFlagToShowerLib",   True)
    kwargs.setdefault("NeutFlagToShowerLib",True)
    kwargs.setdefault("PionFlagToShowerLib",False)
    kwargs.setdefault("ContainLow",         True)
    kwargs.setdefault("AbsLowEta",          3.8)
    kwargs.setdefault("ContainHigh",        True)
    kwargs.setdefault("AbsHighEta",         4.4)
    kwargs.setdefault("DetectorTag",        400000)
    kwargs.setdefault("SensitiveDetector", "FCALFastSimDedicatedSD")
    kwargs.setdefault("EMinEneShowerLib",   1.0)
    result.setPrivateTools(CompFactory.LArFastShowerTool(name="FCAL2FastShower", **kwargs))
    return result
