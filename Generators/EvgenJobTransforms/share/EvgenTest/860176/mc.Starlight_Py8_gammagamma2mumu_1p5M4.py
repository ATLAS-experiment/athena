# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Starlight+Pythia8 gamma + gamma UPC collisions at 5360 GeV to continuum -> mumu, 1.5 < m < 4 GeV, 0.75 < pT(mu) < 2. GeV, |eta(mu)| < 2.6"
        self.keywords = ["2photon","2lepton"]
        self.contact = ["pawel.rybczynski@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):

        slightIn = [
            "maxW 4", #Max value of w
            "minW 1.5", #Min value of w
            "nmbWBins 400", #Bins n w
            "maxRapidity 3.", #max y
            "nmbRapidityBins 300", #Bins n y
            "accCutPt 1", #Cut in pT? 0 = (no, 1 = yes)
            "minPt 0.75", #Minimum pT in GeV
            "maxPt 2.", #Maximum pT in GeV
            "accCutEta 1", #Cut in pseudorapidity? (0 = no, 1 = yes)
            "minEta -2.6", #Minimum pseudorapidity
            "maxEta 2.6", #Maximum pseudorapidity
            "productionMode 1", #(1=2-phot,2=vmeson(narrow),3=vmeson(wide))
            "prodParticleId 13", #Channel of interest
            "beamBreakupMode 5", #Controls the nuclear breakup
        ]

        from Starlight_i.StarlightConfig import Starlight_Pythia8_Common_Cfg
        sampleConfig = Starlight_Pythia8_Common_Cfg(
            flags,
            Initialize=slightIn,
            Commands=['TauDecays:mode = 0']
        )

        return sampleConfig
