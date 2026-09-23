# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""CA rewrite of mcjoboptions DSID 801918."""

from functools import partial

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig


class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = (
            "Exclusive Bs->mu3p5mu3p5 decay production with Photos"
        )
        self.process = "Bs -> mumu"
        self.keywords = ["bottom", "Bs", "2muon", "exclusive"]
        self.contact = ["pavel.reznicek@cern.ch"]
        self.nEventsPerJob = 500

    def setupProcess(self, flags):
        from Pythia8B_i.Pythia8BConfig import (
            Pythia8B_A14_CTEQ6L1_Common_Cfg,
            Pythia8B_exclusiveB_Common_Cfg,
            Pythia8B_Photospp_Cfg,
        )

        shower_cfg = partial(
            Pythia8B_Photospp_Cfg,
            ShowerCfg=Pythia8B_A14_CTEQ6L1_Common_Cfg,
        )
        return Pythia8B_exclusiveB_Common_Cfg(
            flags,
            ShowerCfg=shower_cfg,
            Commands=[
                "PhaseSpace:pTHatMin = 7.",
                "531:addChannel = 2 1.0 0 -13 13",
            ],
            QuarkPtCut=0.0,
            AntiQuarkPtCut=7.0,
            QuarkEtaCut=102.5,
            AntiQuarkEtaCut=2.6,
            RequireBothQuarksPassCuts=True,
            VetoDoubleBEvents=True,
            SignalPDGCodes=[531, -13, 13],
            NHadronizationLoops=2,
            TriggerPDGCode=13,
            TriggerStatePtCut=[3.5],
            TriggerStateEtaCut=2.6,
            MinimumCountPerCut=[2],
        )
