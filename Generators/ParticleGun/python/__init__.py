# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from GeneratorModules.EvgenAlg import EvgenAlg
from ParticleGun.samplers import ParticleSampler
from ParticleGun.samplers import * # noqa: F401, F403 (import into our namespace)
from AthenaPython.PyAthena import StatusCode
import ROOT,random
from cppyy.gbl import std as std
try:
  from AthenaPython.PyAthena import HepMC3  as HepMC
  HepMCVersion=3
except ImportError:
  from AthenaPython.PyAthena import HepMC   as HepMC        
  HepMCVersion=2
__author__ = "Andy Buckley <andy.buckley@cern.ch>, Andrii Verbytskyi <andrii.verbytskyi@cern.ch>"

class ParticleGun(EvgenAlg):
    """
    A simple but flexible algorithm for generating events from simple distributions.
    """

    def __init__(self, name="ParticleGun", randomStream="ParticleGun", randomSeed=None):
        super(ParticleGun, self).__init__(name=name)
        self.samplers = [ParticleSampler()]
        self.randomStream = randomStream
        self.randomSeed = randomSeed
          
    @property
    def sampler(self):
        "Get the first (and presumed only) sampler"
        return self.samplers[0] if self.samplers else None
    @sampler.setter
    def sampler(self, s):
        "Set the samplers list to include only a single sampler, s"
        self.samplers = [s]


    def initialize(self):
        return StatusCode.Success


    def fillEvent(self, evt):
        """
        Sample a list of particle properties, which are then used to create a new GenEvent in StoreGate.
        """
        # set the random seed
        offset = self.randomSeed if self.randomSeed is not None else 0
        seed = ROOT.ATHRNG.calculateSeedsPython(self.randomStream, self._ctx.eventID().event_number(), self._ctx.eventID().run_number(), offset)

        if seed is None:
            self.msg.warning("Failed to find a seed for the random stream named '%s'.", self.randomStream)
            seed = self.randomSeed
        if seed is not None:
            self.msg.debug("Set random seed to %s.", str(seed))
            random.seed(seed)
        else:
            self.msg.error("Failed to set random seed.")
            return StatusCode.Failure

        if HepMCVersion == 2:
          evt.weights().push_back(1.0)
        ## Make and fill particles
          for s in self.samplers:
            particles = s.shoot()
            for p in particles:
                ## Make particle-creation vertex
                pos = HepMC.FourVector(p.pos.X(), p.pos.Y(), p.pos.Z(), p.pos.T())
                gv = HepMC.GenVertex(pos)
                ROOT.SetOwnership(gv, False)
                evt.add_vertex(gv)

                ## Make particle with status == 1
                mom = HepMC.FourVector(p.mom.Px(), p.mom.Py(), p.mom.Pz(), p.mom.E())
                gp = HepMC.GenParticle()
                gp.set_status(1)
                gp.set_pdg_id(p.pid)
                gp.set_momentum(mom)
                if p.mass is not None:
                    gp.set_generated_mass(p.mass)
                ROOT.SetOwnership(gp, False)
                gv.add_particle_out(gp)

        if HepMCVersion == 3:
          evt.set_units(HepMC.Units.MEV, HepMC.Units.MM)
          evt.weights().push_back(1.0)
          evt.set_event_number(self._ctx.eventID().event_number())
          beamparticle1 = std.shared_ptr['HepMC3::GenParticle'](HepMC.GenParticle(HepMC.FourVector(0,0,-7000,7000),2212,4))
          ROOT.SetOwnership(beamparticle1, False)
          beamparticle2 = std.shared_ptr['HepMC3::GenParticle'](HepMC.GenParticle(HepMC.FourVector(0,0,7000,7000),2212,4))
          ROOT.SetOwnership(beamparticle2, False)
          primary = std.shared_ptr['HepMC3::GenVertex'](HepMC.GenVertex())
          ROOT.SetOwnership(primary, False)
          primary.add_particle_in(beamparticle1)
          primary.add_particle_in(beamparticle2)
          evt.add_vertex(primary)
          evt.add_beam_particle(beamparticle1)
          evt.add_beam_particle(beamparticle2)
          #Create all the needed particles
          for s in self.samplers:
            particles = s.shoot()
            for p in particles:
                # Create the production vertex of the particle
                gv = std.shared_ptr['HepMC3::GenVertex'](HepMC.GenVertex(HepMC.FourVector(p.pos.X(), p.pos.Y(), p.pos.Z(), p.pos.T())))
                ROOT.SetOwnership(gv, False)
                evt.add_vertex(gv)
                # Create a fake particle to connect the production vertex of the particle of interest to the primary
                fakeparticle = std.shared_ptr['HepMC3::GenParticle'](HepMC.GenParticle(HepMC.FourVector(p.mom.Px(), p.mom.Py(), p.mom.Pz(), p.mom.E()),p.pid,11))
                ROOT.SetOwnership(fakeparticle, False)
                gv.add_particle_in(fakeparticle)
                primary.add_particle_out(fakeparticle)
                # Create the particle
                gp = std.shared_ptr['HepMC3::GenParticle'](HepMC.GenParticle(HepMC.FourVector(p.mom.Px(), p.mom.Py(), p.mom.Pz(), p.mom.E()),p.pid,1))
                ROOT.SetOwnership(gp, False)
                if p.mass is not None:
                    gp.set_generated_mass(p.mass)
                gv.add_particle_out(gp)
          for p in evt.particles():
            att = std.shared_ptr['HepMC3::IntAttribute'](HepMC.IntAttribute(p.id()))
            p.add_attribute("barcode",att)
          for v in evt.vertices():
            att = std.shared_ptr['HepMC3::IntAttribute'](HepMC.IntAttribute(v.id()))
            v.add_attribute("barcode",att)
        return StatusCode.Success
