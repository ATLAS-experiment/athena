#!/usr/bin/env athena.py

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# This CA configuration script replaces the previously used legacy script for simulating sampling fractions
#

import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

# Common parameters
module = "fcal1"   # Choose from "fcal1", "fcal2" or "fcal3"


def FCalParticleGunCfg(flags):
    acc = ComponentAccumulator()

    # Import paticle gun
    import ParticleGun as PG

    pg = PG.ParticleGun(randomStream = "SINGLE", randomSeed = flags.Random.SeedOffset)

    # Setting particle gun specifications
    pg.sampler.pid = 11
    pg.sampler.mom = PG.EEtaMPhiSampler(energy=params['pg_E'], eta=params['pg_eta'])
    pg.sampler.pos = PG.PosSampler(x=params['pg_x'], y=params['pg_y'], z=params['pg_z'], t=params['pg_z'])

    acc.addEventAlgo(pg)

    return acc


def LArFCalSamplingFractionConfig(flags, name="LArFCalSamplingFraction", **kwargs):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc = LArGMCfg(flags)
    acc.addEventAlgo(CompFactory.LArFCalSamplingFraction(name=name, **kwargs))

    return acc


# Iniitialize flags
flags = initConfigFlags()

# FCal module distances
fcal1_z, fcal2_z, fcal3_z = 4713.5, 5173.3, 5647.8

# Seetting parameters
params = {
    'n_event': 200,          # Number of events to simulate
    'pg_E': 40000,           # Particle gun energy [MeV]
    'pg_x': [212.5, 277.5],  # Particle gun x-coordinate; constant or range
    'pg_y': [7.5, 72.5],     # Particle gun y-coordinate; constant or range
    'pg_z': None,            # Particle gun z-coordinate (distance to IP); should be constant
    'pg_eta': None,          # Particle gun eta; constant or range
}

# Check which module is chosen
if module.lower() == "fcal1":
    params['pg_z'] = fcal1_z
    params['pg_eta'] = [3.5, 3.8]
elif module.lower() == "fcal2":
    params['pg_z'] = fcal2_z
    params['pg_eta'] = [3.5, 3.8]
elif module.lower() == "fcal3":
    params['pg_z'] = fcal3_z
    params['pg_eta'] = [3.5, 3.8]

# Setting important  flags for the simulation
flags.Sim.WorldRRange = 15000.
flags.Sim.WorldZRange = 27000.
flags.IOVDb.GlobalTag = "OFLCOND-MC16-SDR-16"
flags.GeoModel.AtlasVersion = 'ATLAS-R2-2016-01-00-01'
flags.Output.HITSFileName = "atlasG4.hits.pool.root"

# Detector flags - disabling/enabling relevant subsystems
flags.Detector.GeometryCalo = True
flags.Detector.EnableCalo = True
flags.Detector.GeometryID = False
flags.Detector.EnableID = False
flags.Detector.GeometryID = False
flags.Detector.EnableID = False
flags.Detector.GeometryLucid = False
flags.Detector.EnableLucid = False

# Disable input file
flags.Input.Files = []

# Lock the flags to prevent further changes
flags.lock()

# Main CA
acc = MainServicesCfg(flags)

# Merge with particle gun
acc.merge(FCalParticleGunCfg(flags))

# Merge with sampling algorithm
acc.merge(LArFCalSamplingFractionConfig(flags))

# Output service for ROOT file
acc.merge(OutputStreamCfg(flags, "HITS"))

# Adding HIST output
acc.addService(CompFactory.THistSvc(name="THistSvc", Output=[ "AANT DATAFILE='LArFCalSamplingFraction.{}.{:g}GeV.aan.root' OPT='RECREATE'".format(module, params['pg_E']/1000) ]))

# Print and run the accumulator
acc.printConfig(withDetails=True)

# Finalize
acc.run()
sys.exit( acc.run().isFailure() )
