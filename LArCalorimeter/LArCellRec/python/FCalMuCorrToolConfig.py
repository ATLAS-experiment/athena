# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def FCalMuCorrToolCfg(flags, name="FCalMuCorrTool", **kwargs):
    """Private cell-maker tool: FCal1 energy correction E -> E / D(channel, mu).
    Same table as FCalDigitModifierTool in digitization (LAr.FCalDegradation flags)."""
    acc = ComponentAccumulator()
    # FCal1 degradation table: conditions object LArFcalDegFactors, made by LArFcalDegCondAlg
    from LArRecUtils.LArRecUtilsConfig import LArFcalDegCondAlgCfg
    acc.merge(LArFcalDegCondAlgCfg(flags))
    kwargs.setdefault("MuOverride", flags.LAr.FCalDegradation.MuOverride)
    acc.setPrivateTools(CompFactory.FCalMuCorrTool(name, **kwargs))
    return acc
