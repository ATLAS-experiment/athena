# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig


class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "SuperChic yy->WW"
        self.keywords = ["photonFusion","WW","dissociation"]
        self.contact = ["pawel.rybczynski@cern.ch"]

    def setupProcess(self, flags):

        from Analysis_Common_yyWW import GetSuperChicConfig
        scConfig = GetSuperChicConfig()

        safFactor = 16

        scConfig['proc'] = 55
        scConfig['sfaci'] = False
        scConfig['diff'] = 'el'
        scConfig['mmin'] = 0
        scConfig['mmax'] = 100000
        scConfig['wlp'] = 'tau'
        scConfig['wlm'] = 'tau'

        MLFil = True
        NchFil = False

        # Set up SuperChic
        from Superchic_i.SuperChicConfig import SuperChicBaseCfg
        sampleConfig = SuperChicBaseCfg(
            flags,
            Config=scConfig,
            safety=safFactor
        )

        # Get Pythia8 commands
        from Analysis_Common_yyWW import GetPythia8DissociationCommands
        Commands = GetPythia8DissociationCommands(scConfig)

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

        # Set up filters
        from Analysis_Common_yyWW import SetUpFilters
        SetUpFilters(flags, sampleConfig, MLFil, NchFil)

        return sampleConfig
