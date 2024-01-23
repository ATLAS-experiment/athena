# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# Configuration of TrkResidualPullCalculator package
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ResidualPullCalculatorCfg(flags, name='ResidualPullCalculator', **kwargs):
    acc = ComponentAccumulator()

    if not flags.Detector.EnableSCT and not flags.Detector.EnableITkStrip:
        kwargs.setdefault("ResidualPullCalculatorForSCT", "")
    if not flags.Detector.EnableRPC:
        kwargs.setdefault("ResidualPullCalculatorForRPC", "")
    if not flags.Detector.EnableTGC:
        kwargs.setdefault("ResidualPullCalculatorForTGC", "")

    acc.setPrivateTools(CompFactory.Trk.ResidualPullCalculator(name, **kwargs))
    return acc
