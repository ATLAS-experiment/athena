# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


def PrintMCCfg(flags, name="PrintMC", **kwargs):
    kwargs.setdefault("McEventKey", "GEN_EVENT")
    kwargs.setdefault("VerboseOutput", True)
    kwargs.setdefault("PrintStyle", "Barcode")
    kwargs.setdefault("FirstEvent", 1)
    kwargs.setdefault("LastEvent", 1)

    acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
    acc.addEventAlgo(CompFactory.PrintMC(name, **kwargs))
    return acc
