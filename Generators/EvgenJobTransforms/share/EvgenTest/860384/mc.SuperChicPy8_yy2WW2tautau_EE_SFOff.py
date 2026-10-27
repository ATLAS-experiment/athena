# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "SuperChic yy->WW"
        self.keywords = ["photonFusion","WW","dissociation"]
        self.contact = ["pawel.rybczynski@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):

        Config = {
            "isurv": 4,
            "PDFname": 'MSHT20qed_nnlo',
            "PDFmember": 0,
            "proc": 55,
            "beam": 'prot',
            "sfaci": False,
            "diff": 'el',
            "genunw": True,
            "ymin": -5.0,
            "ymax": 5.0,
            "mmin": 0,
            "mmax": 100000,
            "gencuts": True,
            "scorr": True,
            "fwidth": True,
            "ptxmax": 300,
            "ptamin": 3.0,
            "ptbmin": 3.0,
            "etaamin": -3.0,
            "etaamax": 3.0,
            "etabmin": -3.0,
            "etabmax": 3.0,
            "acoabmax": 100,
            "wlp": 'tau',
            "wlm": 'tau',
            "ncall": 100000,
            "ncall1": 100000
        }

        # Set up SuperChic
        from Superchic_i.SuperChicConfig import SuperChicBaseCfg
        sampleConfig = SuperChicBaseCfg(
            flags,
            Config=Config,
            safety=20
        )

        Commands = [
            "TimeShower:recoilStrategyRF = 1",
            "PartonLevel:MPI = off",
            "SpaceShower:pTmaxMatch = 2",
            "SpaceShower:pTdampMatch = 1",
            "LesHouches:matchInOut = off",
            "SpaceShower:dipoleRecoil = on",
            "PDF:BeamA2gamma = on",
            "PDF:BeamB2gamma = on",
            "Photon:ProcessType = 4",
            "PDF:Proton2gammaSet = 1",
            "Photon:sampleQ2 = off"
        ]

        # Set up base Pythia8 for showering
        from Superchic_i.SuperChicConfig import SuperChicPythia8BaseCfg
        sampleConfig.merge(SuperChicPythia8BaseCfg(
            flags,
            Commands=Commands
        ))

        # Add multi lepton filter
        from GeneratorFilters.GeneratorFiltersConfig import xAODMultiLeptonFilterCommonCfg
        sampleConfig.merge(xAODMultiLeptonFilterCommonCfg(
            flags,
            PtCut=3500.,
            EtaCut=2.6,
            NLeptons=2
        ))

        return sampleConfig
