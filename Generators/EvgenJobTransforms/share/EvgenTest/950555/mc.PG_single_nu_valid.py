# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from EvgenJobTransforms.EvgenCAConfig import EvgenConfig
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory

class Sample(EvgenConfig):

    def setupFlags(self, flags):
        self.description = "Single neutrinos with fixed eta and E: purely for pile-up/lumi testing"
        self.keywords = ["singleParticle", "neutrino"]
        self.contact = ["dhirsch@mail.cern.ch"]
        self.nEventsPerJob = 100
        self.generators += ["ParticleGun"]

    def setupProcess(self, flags):
        # TODO: update once we have a proper PG fragment
        sampleConfig = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))

        import ParticleGun as PG
        pg = PG.ParticleGun(randomStream="SINGLE", randomSeed=flags.Random.SeedOffset)
        pg.sampler.pid = 12
        pg.sampler.mom = PG.EEtaMPhiSampler(energy=50000, eta=0)

        sampleConfig.addEventAlgo(pg)

        return sampleConfig
