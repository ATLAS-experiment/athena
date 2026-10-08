# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Starlight+Pythia8+Tauolapp+Photospp gamma + gamma UPC collisions to continuum -> 2 tau, breakup mode 5 (no selection) with pT>3 GeV charged tracks filter"
        self.keywords = ["2photon","2lepton","2tau"]
        self.contact = ["pawel.rybczynski@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):

        slightIn = [
            "maxW 200", #Max value of w
            "minW 4", #Min value of w
            "nmbWBins 400", #Bins n w
            "maxRapidity 9.", #max y
            "nmbRapidityBins 1000", #Bins n y
            "accCutPt 1", #Cut in pT? 0 = (no, 1 = yes)
            "minPt 2.0", #Minimum pT in GeV
            "maxPt 100.0", #Maximum pT in GeV
            "accCutEta 1", #Cut in pseudorapidity? (0 = no, 1 = yes)
            "minEta -5", #Minimum pseudorapidity
            "maxEta 5", #Maximum pseudorapidity
            "productionMode 1", #(1=2-phot,2=vmeson(narrow),3=vmeson(wide))
            "prodParticleId 15", #Channel of interest
            "beamBreakupMode 5", #Controls the nuclear breakup
        ]

        from Starlight_i.StarlightConfig import Starlight_Pythia8_Common_Cfg
        sampleConfig = Starlight_Pythia8_Common_Cfg(
            flags,
            Initialize=slightIn,
            Commands=[
                    'TimeShower:QEDshowerByOther = off', # for Photos
                    '15:onMode = off' # for tau decay with Taola
                    ],
            safety=20,
            doTauolappLheFormat=True
        )

        # Add Tauola on top of Pythia8
        from Tauolapp_i.TauolappConfig import TauolappCfg
        sampleConfig.merge(TauolappCfg(flags))

        # Add Photos on top of Pythia8
        from Photospp_i.PhotosppConfig import PhotosppCfg
        sampleConfig.merge(PhotosppCfg(flags))

        # Add filter
        from GeneratorFilters.GeneratorFiltersConfig import xAODChargedTracksFilterCommonCfg
        sampleConfig.merge(xAODChargedTracksFilterCommonCfg(
            flags,
            NTracks=1,
            Ptcut=3000.,
            Etacut=2.6,
        ))

        return sampleConfig
