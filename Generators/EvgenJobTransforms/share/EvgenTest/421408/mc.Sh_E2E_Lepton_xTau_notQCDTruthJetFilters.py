# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import parAND
from AthenaCommon.SystemOfUnits import GeV
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from EvgenJobTransforms.EvgenCAConfig import EvgenConfig
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory


def xAODLeptonFilterCfg(flags, **kwargs):
    kwargs.setdefault("Ptcut", 13 * GeV)

    from GeneratorFilters.GeneratorFiltersConfig import xAODLeptonFilterCommonCfg
    return xAODLeptonFilterCommonCfg(flags, **kwargs)


def xAODTauFilterCfg(flags, **kwargs):
    kwargs.setdefault("Ntaus", 2)
    kwargs.setdefault("EtaMaxe", 2.7)
    kwargs.setdefault("EtaMaxmu", 2.7)
    kwargs.setdefault("EtaMaxhad", 2.7)  # no hadronic tau decays
    kwargs.setdefault("Ptcute", 31 * GeV)
    kwargs.setdefault("Ptcutmu", 33 * GeV)

    from GeneratorFilters.GeneratorFiltersConfig import xAODTauFilterCommonCfg
    return xAODTauFilterCommonCfg(flags, **kwargs)


def QCDTruthJetFilterCfg(flags, **kwargs):
    kwargs.setdefault("MinPt", 53 * GeV)

    from GeneratorFilters.GeneratorFiltersConfig import QCDTruthJetFilterCommonCfg
    return QCDTruthJetFilterCommonCfg(flags, jetR=0.6, **kwargs)


class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Sherpa Z/gamma* -> tau tau + 0,1,2j@NLO + 3,4,5j@LO with di-leptonic tau decays and b-jet filter taking input from existing unfiltered input file."
        self.keywords = ["SM", "Z", "2tau", "2lepton", "jets", "NLO"]
        self.contact = ["tadej@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):
        # we run no generators here so it's fine only to configure the filter sequence
        acc = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        acc.addSequence(parAND("InvertQCD", invert=True))

        # by default filters are joined with AND
        # add lepton filter
        acc.merge(xAODLeptonFilterCfg(flags))
        # add QCD jet filter
        acc.merge(QCDTruthJetFilterCfg(flags), sequenceName="InvertQCD")
        # add tau filter
        acc.merge(xAODTauFilterCfg(flags))

        # TODO:
        # postSeq.CountHepMC.CorrectRunNumber = True

        return acc
