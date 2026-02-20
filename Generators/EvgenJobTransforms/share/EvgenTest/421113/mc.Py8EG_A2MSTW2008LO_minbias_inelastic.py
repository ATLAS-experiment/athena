# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Inelastic minimum bias, with the A2 MSTW2008LO tune and EvtGen"
        self.keywords = ["QCD", "minBias", "SM"]
        self.contact = ["spyros.argyropoulos@cern.ch"]
        self.nEventsPerJob = 1000

    def setupProcess(self, flags):
        # Pythia8 process
        p8_process = ["SoftQCD:inelastic = on"]
        
        # Pythia with EvtGen
        from Pythia8_i.Pythia8Config import Pythia8_A2_MSTW2008LO_EvtGen_Common_Cfg
        sampleConfig = Pythia8_A2_MSTW2008LO_EvtGen_Common_Cfg(flags, Commands=p8_process)

        return sampleConfig