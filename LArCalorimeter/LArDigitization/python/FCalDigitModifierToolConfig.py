# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FCalDigitModifierToolCfg(flags, name="FCalDigitModifierTool", **kwargs):
    """Private tool: mu-dependent FCal1 amplitude degradation, to be given to LArHitEMapToDigitAlg.
    Table from the conditions database (LArFcalDegCondAlg); mu from EventInfo or LAr.FCalDegradation.MuOverride."""
    acc = ComponentAccumulator()
    # FCal1 degradation table: conditions object LArFcalDegFactors, made by LArFcalDegCondAlg
    from LArRecUtils.LArRecUtilsConfig import LArFcalDegCondAlgCfg
    acc.merge(LArFcalDegCondAlgCfg(flags))
    kwargs.setdefault("MuOverride", flags.LAr.FCalDegradation.MuOverride)
    acc.setPrivateTools(CompFactory.FCalDigitModifierTool(name, **kwargs))
    return acc
