# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FCalDigitModifierToolCfg(flags, name="FCalDigitModifierTool", **kwargs):
    """Private tool modelling FCal signal degradation, to be given to LArHitEMapToDigitAlg."""
    acc = ComponentAccumulator()
    kwargs.setdefault("ScaleFactor", 1.05)
    acc.setPrivateTools(CompFactory.FCalDigitModifierTool(name, **kwargs))
    return acc
