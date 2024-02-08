# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PrintMCCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.PrintMC(
            "PrintMC",
            **kwargs
        )
    )   
    return acc
