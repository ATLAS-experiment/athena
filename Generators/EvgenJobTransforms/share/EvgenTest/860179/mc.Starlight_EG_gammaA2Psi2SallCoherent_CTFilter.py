# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Starlight+EvtGen coherent gamma + A UPC collisions at 5360 GeV to Psi(2S) -> all decays, breakup mode 5, pT(trk)>0.9GeV, |eta(trk)|<2.6"
        self.keywords = ["2photon","2lepton"]
        self.contact = ["pawel.rybczynski@cern.ch"]
        self.nEventsPerJob = 10000

    def setupProcess(self, flags):

        slightIn = [
            "nmbWBins 200", #Bins n w
            "maxRapidity 3.", #max y
            "nmbRapidityBins 200", #Bins n y
            "productionMode 2", #(1=2-phot,2=vmeson(narrow),3=vmeson(wide))
            "prodParticleId 444011", #Channel of interest
            "beamBreakupMode 5", #Controls the nuclear breakup
        ]

        # Set up Starlight with vector meson decays from EvtGen
        from Starlight_i.StarlightConfig import Starlight_EvtGen_Common_Cfg
        sampleConfig = Starlight_EvtGen_Common_Cfg(flags, Initialize=slightIn)

        # Add filter
        from GeneratorFilters.GeneratorFiltersConfig import xAODChargedTracksFilterCommonCfg
        sampleConfig.merge(xAODChargedTracksFilterCommonCfg(
            flags,
            NTracks=1,
            Ptcut=900.,
            Etacut=2.6,
        ))

        return sampleConfig

