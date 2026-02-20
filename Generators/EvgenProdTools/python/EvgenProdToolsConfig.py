# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


def TestHepMCCfg(flags, name="TestHepMC", streamName="TestHepMCname", fileName="TestHepMC.root", **kwargs):
    kwargs.setdefault("CmEnergy", flags.Beam.Energy * 2)

    acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Test))
    acc.addService(CompFactory.THistSvc(name="THistSvc",
                   Output=[f"{streamName} DATAFILE='{fileName}' OPT='RECREATE'"]))

    acc.addEventAlgo(CompFactory.TestHepMC(name, **kwargs))
    return acc


def FixHepMCCfg(flags, name="FixHepMC", **kwargs):
    acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Fix))
    acc.addEventAlgo(CompFactory.FixHepMC(name, **kwargs))
    return acc


def CountHepMCCfg(flags, name="CountHepMC", **kwargs):
    kwargs.setdefault("InputEventInfo", "TMPEvtInfo")
    kwargs.setdefault("OutputEventInfo", "EventInfo")
    kwargs.setdefault("mcEventWeightsKey", "mcEventWeights")
    kwargs.setdefault("FirstEvent", flags.Exec.FirstEvent)
    kwargs.setdefault("CorrectHepMC", True)
    kwargs.setdefault("CorrectEventID", True)

    if flags.Input.Files:
        kwargs.setdefault("CorrectRunNumber", True)
        kwargs.setdefault("NewRunNumber", flags.Generator.DSID)

    acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
    acc.addEventAlgo(CompFactory.CountHepMC(name, **kwargs))
    return acc


def CopyEventWeightCfg(flags, name="CopyEventWeight", **kwargs):
    kwargs.setdefault("mcEventWeightsKey", "TMPEvtInfo.mcEventWeights")

    acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
    acc.addEventAlgo(CompFactory.CopyEventWeight(name, **kwargs))
    return acc


def FillFilterValuesCfg(flags, name="FillFilterValues", **kwargs):
    kwargs.setdefault("mcFilterHTKey", "TMPEvtInfo.mcFilterHT")

    acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
    acc.addEventAlgo(CompFactory.FillFilterValues(name, **kwargs))
    return acc


def SimTimeEstimateCfg(flags, name="SimTimeEstimate", **kwargs):
    acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
    acc.addEventAlgo(CompFactory.SimTimeEstimate(name, **kwargs))
    return acc
