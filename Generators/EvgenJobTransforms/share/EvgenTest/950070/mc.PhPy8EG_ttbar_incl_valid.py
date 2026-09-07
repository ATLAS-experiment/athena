# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig


class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "POWHEG ttbar production for validation purposes"
        self.keywords = ["SM", "top", "ttbar"]
        self.contact = ["bwynnyck@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):
        from PowhegControl.PowhegConfig import PowhegCfg
        from Pythia8_i.Pythia8Config import (
            Pythia8_Powheg_Main31_Cfg,
            Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg,
            Pythia8EvtGenBaseCfg,
        )

        # ATLAS defaults for the Powheg tt process
        ca, lhe_file = PowhegCfg(
            flags,
            process="tt",
            settings={
                "decay_mode": "t t~ > all",
                "hdamp": 258.75,                                        # 1.5 * mtop
                "mu_F": [1.0, 2.0, 0.5, 1.0, 1.0, 0.5, 2.0, 0.5, 2.0],  # factorisation scales, paired with the renormalisation scales below
                "mu_R": [1.0, 1.0, 1.0, 2.0, 0.5, 0.5, 2.0, 2.0, 0.5],  # List of renormalisation scales
                "PDF": [260000, 25200, 13165, 90900, 265000, 266000, 303400],   # NNPDF30_nlo_as_0118, MMHT2014nlo68clas118, CT14nlo_as_0118, PDF4LHC15_nlo_30, NNPDF30_nlo_as_0117, NNPDF30_nlo_as_0119
            },
        )

        # Pythia8 showering: A14 tune with Main31 POWHEG matching
        powheg_matching_commands = [
            "Powheg:pTHard    = 0",
            "Powheg:pTdef     = 2",
            "Powheg:vetoCount = 3",
            "Powheg:pTemt     = 0",
            "Powheg:emitted   = 0",
            "Powheg:MPIveto   = 0",
        ]
        ca.merge(Pythia8_Powheg_Main31_Cfg(
            flags,
            ShowerCfg=Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg,
            LHEFile=lhe_file,
            NFinal=2,
            Commands=powheg_matching_commands,
        ))

        # EvtGen afterburner (legacy uses the ..._EvtGen_Common fragment)
        ca.merge(Pythia8EvtGenBaseCfg(flags))
        return ca
