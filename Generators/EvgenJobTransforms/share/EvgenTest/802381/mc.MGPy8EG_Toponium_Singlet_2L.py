# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Toponium production with 2L, singlet part, showering only, m_t = 173 GeV"
        self.keywords = ["SM", "top"]
        self.contact = ["spyros.argyropoulos@cern.ch"]
        self.nEventsPerJob = 1000
        # This uses LHE produced by MG
        self.MEgenerator = "MadGraph"
        self.inputFilesPerJob = 1
        

    def setupProcess(self, flags):
        from Pythia8_i.Pythia8Config import (
            Pythia8_MadGraph_Cfg,
            Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg,
        )

        # Set up MadGraph + Pythia8
        sampleConfig = Pythia8_MadGraph_Cfg(
            flags,
            # Specify tune and EvtGen
            ShowerCfg=Pythia8_A14_NNPDF23LO_EvtGen_Common_Cfg,
        )

        return sampleConfig
