# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TestHepMCCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.TestHepMC(
            "TestHepMC",
            **kwargs
        )
    )   
    return acc


def FixHepMCCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.FixHepMC(
            "FixHepMC",
            **kwargs
        )
    )    
    return acc


def CountHepMCCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.CountHepMC(
            "CountHepMC",
            **kwargs
        )
    )    
    return acc
    

def CopyEventWeightCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.CopyEventWeight(
            "CopyEventWeight",
            **kwargs
        )
    )
    return acc


def FillFilterValuesCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.FillFilterValues(
            "FillFilterValues",
            **kwargs
        )
    )
    return acc


def SimTimeEstimateCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.SimTimeEstimate(
            "SimTimeEstimate",
            **kwargs
        )
    )
    return acc
