#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Generators providing input events via the LHEF format 
# (used to determine the input file dummy-naming strategy for C++ generators)
LHEFGenerators = ["Lhef", # generic name: prefer to use the names below
                  "aMcAtNlo", "McAtNlo", "Powheg",  "MadGraph", "CompHep", "Geneva",
                  "MCFM", "JHU", "MEtop", "BCVEGPY", "Dire4Pythia8", 
                  "BlackMax", "QBH", "gg2ww", "gg2zz", "gg2vv", "HvyN", 
                  "VBFNLO", "FPMC", "ProtosLHEF",
                  "BCVEGPY", "STRINGS", "Phantom"]

# "Main" generators which typically model QCD showers, hadronisation, decays, etc.
# Herwig family
MainGenerators = ["Herwig7"]
# Pythia family
MainGenerators += ["Pythia8", "Pythia8B"]
# Sherpa family
MainGenerators += ["Sherpa"]
# Soft QCD generators
MainGenerators += ["Epos"]
# ATLAS-specific generators
MainGenerators += ["ParticleGun"]
MainGenerators += ["CosmicGenerator", "BeamHaloGenerator"]
# Heavy ion generators
MainGenerators += ["AMPT","Superchic","Starlight", "Hijing", "Hydjet"]
# Reading in fully-formed events
MainGenerators += ["HepMCAscii"]

# Special QED and decay afterburners
# note: we have to use TauolaPP, because Tauolapp is used as a namespace in the external Tauolapp code
AfterburnerGenerators = ["Photospp", "TauolaPP", "EvtGen", "ParticleDecayer"]

# Set up list of allowed generators. The evgenConfig.generators list will be used
# to set random seeds, determine input config and event files, and report used generators to AMI.
KnownGenerators = LHEFGenerators + MainGenerators + AfterburnerGenerators

# Note which generators should NOT be sanity tested by the TestHepMC alg
NoTestHepMCGenerators = ["Superchic","ParticleDecayer", "ParticleGun", "CosmicGenerator", 
                         "BeamHaloGenerator", "FPMC", "Hijing", "Hydjet", "Starlight"]

# Generators with no flexibility/concept of a tune or PDF choice
NoTuneGenerators = ["ParticleGun", "CosmicGenerator", "BeamHaloGenerator", "HepMCAscii"]


def gen_require_steering(gennames):
    """Return a boolean of whether this set of generators requires the steering command line flag"""
    if "EvtGen" not in gennames: return False
    if any(("Pythia" in gen and "Pythia8" not in gen) for gen in gennames): return True
    if any(("Herwig" in gen and "Herwig7" not in gen) for gen in gennames): return True
    return False

def gen_known(genname):
    """Return whether a generator name is known"""
    return genname in KnownGenerators

def gens_known(gennames):
    """Return whether all generator names are known"""
    return all(gen_known(g) for g in gennames)

def gen_lhef(genname):
    """Return whether a generator uses LHEF input files"""
    return genname in LHEFGenerators

def gens_lhef(gennames):
    """Return whether any of the generators uses LHEF input files"""
    return any(gen_lhef(g) for g in gennames)

def gen_testhepmc(genname):
    """Return whether a generator should be sanity tested with TestHepMC"""
    return genname not in NoTestHepMCGenerators

def gens_testhepmc(gennames):
    """Return whether all of the generators should be sanity tested with TestHepMC"""
    return all(gen_testhepmc(g) for g in gennames)

def gen_notune(genname):
    """Return whether a generator is allowed to not provide PDF and tune information"""
    return genname not in NoTuneGenerators

def gens_notune(gennames):
    """Return whether all of the generators are allowed to not provide PDF and tune information"""
    return all(gen_notune(g) for g in gennames)

def gen_sortkey(genname):
    """Return a key suitable for sorting a generator name by stage, then alphabetically"""
    
    # Sort mainly in order of generator stage
    genstage = None
    for istage, gens in enumerate([LHEFGenerators, MainGenerators, AfterburnerGenerators]):
        if genname in gens:
            genstage = istage
            break

    # Return a tuple
    return (genstage,  genname)
