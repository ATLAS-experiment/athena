# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Sherpa 3.0 example JO, Z+0,1-jet production."
        self.keywords = ["2lepton"]
        self.contact = ["atlas-generators-sherpa@cern.ch", "spyros.argyropoulos@cern.ch"]
        self.nEventsPerJob = 10000
        
    def setupProcess(self, flags):
        from Sherpa_i.SherpaConfig import (
            Sherpa3_PDF4LHC21_Cfg
        )

        # Define Sherpa Process and selectors
        run_card = """
            PROCESSES:
            - 93 93 -> 11 -11 93{0}:
                Order: {QCD: 0, EW: 2}
                CKKW: 20
    
            SELECTORS:
            - [Mass, 11, -11, 40, E_CMS]
            """
        

        # Set up Sherpa configuration: Sherpa 3 with PDF4LHC21 NNLO PDF tune,
        # no OpenLoops, and default Sherpa settings for hadronization, MPI, etc
        sampleConfig = Sherpa3_PDF4LHC21_Cfg(
            flags,
            RunCard=run_card
        )

        # Non-default settings can be passed as kwargs, e.g.
        #sampleConfig = Sherpa3_PDF4LHC21_Cfg(
        #    flags,
        #    RunCard=run_card,
        #    NCores=16,
        #    CleanupGeneratedFiles=False
        #)

        return sampleConfig
