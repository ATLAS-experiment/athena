"""ComponentAccumulator run dependent service configuration

Including features from old style DigitizationFlags.py

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Auxiliary
def maxNevtsPerXing(flags):
    """Return the largest minbias pileup value, for PileUpEvtLoopMgr caches"""
    # migrated from DigitizationConfig.lags.py
    pDicts = flags.Input.RunAndLumiOverrideList
    return max(element["mu"] for element in pDicts)


def runLumiListAndScaleFactorLists(flags):
    # migrated from DigitizationConfig.lags.py
    runLumiList = []
    scaleFactorList = []
    pDicts = flags.Input.RunAndLumiOverrideList
    MaxCollisionsPerXing = maxNevtsPerXing(flags)
    for element in pDicts:
        run = element["run"]
        lbk = element["lb"]
        mu = element["mu"]
        iovt = (run << 32) + lbk
        runLumiList += [iovt]
        if MaxCollisionsPerXing > 0:
            scaleFactorList += [mu/MaxCollisionsPerXing]
        else:
            scaleFactorList += [1.0]
    return runLumiList, scaleFactorList


# Config
def LumiProfileSvcCfg(flags, name="LumiProfileSvc", **kwargs):
    acc = ComponentAccumulator()
    runLumiList, scaleFactorList = runLumiListAndScaleFactorLists(flags)
    #clear svc properties?
    kwargs.setdefault("RunLumiList", runLumiList)
    kwargs.setdefault("ScaleFactorList", scaleFactorList)
    acc.addService(CompFactory.LumiProfileSvc(name, **kwargs), primary=True)
    return acc


def NoProfileSvcCfg(flags, name="NoProfileSvc", **kwargs):
    acc = ComponentAccumulator()
    acc.addService(CompFactory.NoProfileSvc(name, **kwargs), primary=True)
    return acc
