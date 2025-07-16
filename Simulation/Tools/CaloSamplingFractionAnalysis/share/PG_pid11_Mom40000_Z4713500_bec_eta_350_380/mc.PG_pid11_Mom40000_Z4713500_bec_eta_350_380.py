#! -*- python -*-
evgenConfig.description = "Single particle gun for Sampling Fraction event generation"
evgenConfig.keywords = ["singleParticle",]
evgenConfig.generators = ["ParticleGun"]
evgenConfig.contact = ["michael.duehrssen@cern.ch"]

## Common parameters
module = "fcal1"   # Choose from "fcal1", "fcal2" or "fcal3"

## Distance from IP to face of each FCal module [mm]

## Values based on Table 3 in JINST 3 P02010 (2008), "The ATLAS Forward Calorimeter"
## These numbers do not necessarily reflect the geometry and location of the FCal in the Geant4 model!
# fcal1_z = 4708.90  # Distance from IP to FCal1 face [mm]; = 4683.5 + 26.4 - 1.0
# fcal2_z = 5166.10  # Distance from IP to FCal2 face [mm]; = 4683.5 + 483.6 - 1.0
# fcal3_z = 5648.20  # Distance from IP to FCal3 face [mm]; = 4683.5 + 965.7 - 1.0

## Values for the Geant4 module
fcal1_z = 4713.5
fcal2_z = 5173.3
fcal3_z = 5647.8

params = {
    'n_event': 200,          # Number of events to simulate
    'pg_E': 40000,           # Particle gun energy [MeV]
    'pg_x': [212.5, 277.5],  # Particle gun x-coordinate; constant or range
    'pg_y': [7.5, 72.5],     # Particle gun y-coordinate; constant or range
    'pg_z': None,            # Particle gun z-coordinate (distance to IP); should be constant
    'pg_eta': None,          # Particle gun eta; constant or range
}

if module.lower() == "fcal1":
    params['pg_z'] = fcal1_z
    params['pg_eta'] = [3.5, 3.8]
elif module.lower() == "fcal2":
    params['pg_z'] = fcal2_z
    params['pg_eta'] = [3.5, 3.8]
elif module.lower() == "fcal3":
    params['pg_z'] = fcal3_z
    params['pg_eta'] = [3.5, 3.8]


## Set particle gun parameters
import AthenaCommon.AtlasUnixGeneratorJob
import ParticleGun as PG
pg = PG.ParticleGun()
pg.sampler.pid = 11
pg.sampler.mom = PG.EEtaMPhiSampler(energy=params['pg_E'], eta=params['pg_eta'])
pg.sampler.pos = PG.PosSampler(x=params['pg_x'], y=params['pg_y'], z=params['pg_z'], t=params['pg_z'])
genSeq += pg
