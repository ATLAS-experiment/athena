#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Get logger
from AthenaCommon.Logging import logging
evgenLog = logging.getLogger('GenConfigHelpers')

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
MainGenerators += ["Epos4"]
# ATLAS-specific generators
MainGenerators += ["ParticleGun"]
MainGenerators += ["CosmicGenerator", "BeamHaloGenerator"]
# Heavy ion generators - as a special group to avoid problems in sorting
HIMainGenerators = ["AMPT","SuperChic","Starlight", "Hijing", "Hydjet"]
# Reading in fully-formed events

MainGenerators += ["HepMCAscii"]

# Special QED and decay afterburners
# note: we have to use TauolaPP, because Tauolapp is used as a namespace in the external Tauolapp code
AfterburnerGenerators = ["Photospp", "TauolaPP", "EvtGen", "ParticleDecayer"]

# Set up list of allowed generators. The sample.generators list will be used
# to set random seeds, determine input config and event files, and report used generators to AMI.
KnownGenerators = LHEFGenerators + HIMainGenerators +MainGenerators + AfterburnerGenerators

# Note which generators should NOT be sanity tested by the TestHepMC alg
NoTestHepMCGenerators = ["Superchic","ParticleDecayer", "ParticleGun", "CosmicGenerator", 
                         "BeamHaloGenerator", "FPMC", "Hijing", "Hydjet", "Starlight"]

# Generators with no flexibility/concept of a tune or PDF choice
NoTuneGenerators = ["ParticleGun", "CosmicGenerator", "BeamHaloGenerator", "HepMCAscii"]

# Generators whose unstable particles without end vertex have to be purged
# n.b. "Pythia8-Angantyr" is not a 'real' name, the real name would be just 'Pythia8'
PurgeNoEndVtxGenerators = ["Pythia8-Angantyr", "Herwig7", "Hijing"]

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

def gen_purgenoendvtx(genname):
    """Return whether a generator may produce unstable particles 
    without end vertex that have to be purged"""
    return genname in PurgeNoEndVtxGenerators

def gens_purgenoendvtx(gennames):
    """Return whether any of the generators may produce unstable particles 
    without end vertex that have to be purged"""
    return any(gen_purgenoendvtx(g) for g in gennames)

def gen_sortkey(genname):
    """Return a key suitable for sorting a generator name by stage, then alphabetically"""
    
    # Sort mainly in order of generator stage
    genstage = None
    for istage, gens in enumerate([LHEFGenerators, HIMainGenerators, MainGenerators,AfterburnerGenerators]):
        if genname in gens:
            genstage = istage
            break

    # Return a tuple
    return (genstage,  genname)

# Function to perform consistency check on jO
def checkJOConsistency(jofile):
    import os, sys, string
    officialJO = False
    joparts = (os.path.basename(jofile)).split(".")
    # Perform some consistency checks
    if joparts[0].startswith("mc") and all(c in string.digits for c in joparts[0][2:]):
        officialJO = True
        # Check that there are exactly 4 name parts separated by '.': MCxx, DSID, physicsShort, .py
        if len(joparts) != 3:
            evgenLog.error(jofile + " name format is wrong: must be of the form mc.<physicsShort>.py: please rename.")
            sys.exit(1)
        # Check the length limit on the physicsShort portion of the filename
        jo_physshortpart = joparts[1]
        if len(jo_physshortpart) > 50:
            evgenLog.error(jofile + " contains a physicsShort field of more than 60 characters: please rename.")
            sys.exit(1)
        # There must be at least 2 physicsShort sub-parts separated by '_': gens, (tune)+PDF, and process
        jo_physshortparts = jo_physshortpart.split("_")
        if len(jo_physshortparts) < 2:
            evgenLog.error(jofile + " has too few physicsShort fields separated by '_': should contain <generators>(_<tune+PDF_if_available>)_<process>. Please rename.")
            sys.exit(1)
        
        # NOTE: a further check on physicsShort consistency is done below, after fragment loading
        check_jofiles="/cvmfs/atlas.cern.ch/repo/sw/Generators/MC16JobOptions/scripts"
        sys.path.append(check_jofiles)
        from check_jo_consistency import check_naming       
        if os.path.exists(check_jofiles):
            check_naming(os.path.basename(jofile))
        else:
            evgenLog.error("check_jo_consistency.py not found")
            sys.exit(1)
    return officialJO
    
def checkNEventsPerJob(sample):
    if sample.nEventsPerJob < 1:
        raise RuntimeError("nEventsPerJob must be at least 1")
    elif sample.nEventsPerJob > 100000:
        raise RuntimeError("nEventsPerJob can be max. 100000")
    else:
        allowed_nEventsPerJob_lt1000 = [1, 2, 5, 10, 20, 25, 50, 100, 200, 500, 1000]
        if sample.nEventsPerJob >= 1000 and sample.nEventsPerJob <= 10000 and \
          (sample.nEventsPerJob % 1000 != 0 or 10000 % sample.nEventsPerJob != 0):
            raise RuntimeError("nEventsPerJob in range [1K, 10K] must be a multiple of 1K and a divisor of 10K")
        elif sample.nEventsPerJob > 10000  and sample.nEventsPerJob % 10000 != 0:
            raise RuntimeError("nEventsPerJob >10K must be a multiple of 10K")
        elif sample.nEventsPerJob < 1000 and sample.nEventsPerJob not in allowed_nEventsPerJob_lt1000:
            raise RuntimeError("nEventsPerJob in range <= 1000 must be one of %s" % allowed_nEventsPerJob_lt1000)
    
def checkKeywords(sample, evgenLog, officialJO):
    # Get file containing keywords
    from AthenaCommon.Utils.unixtools import find_datafile
    kwpath = find_datafile("evgenkeywords.txt")
    
    # Load the allowed keywords from the file
    allowed_keywords = []
    if kwpath: 
        evgenLog.info("evgenkeywords = %s", kwpath)
        kwf = open(kwpath, "r")
        for l in kwf:
            allowed_keywords += l.strip().lower().split()
        # Check the JO keywords against the allowed ones
        evil_keywords = []
        for k in sample.keywords:
            if k.lower() not in allowed_keywords:
                evil_keywords.append(k)
        if evil_keywords:
            msg = "keywords contains non-standard keywords: %s. " % ", ".join(evil_keywords)
            msg += "Please check the allowed keywords list and fix."
            evgenLog.error(msg)
            if officialJO:
                import sys
                sys.exit(1)
    else:
        evgenLog.warning("evgenkeywords.txt not found ")

def checkCategories(sample, evgenLog, officialJO):
    # Get file containing category names
    from AthenaCommon.Utils.unixtools import find_datafile
    lkwpath = find_datafile("CategoryList.txt")
    
    # Load the allowed categories names from the file
    allowed_cat = []
    if lkwpath:
        from ast import literal_eval
        with open(lkwpath, 'r') as catlist:
            for line in catlist:
               allowed_list = literal_eval(line)
               allowed_cat.append(allowed_list)

        # Check the JO categories against the allowed ones
        bad_cat =[]
        it = iter(sample.categories)
        for x in it:
           l1 = x
           l2 = next(it)
           if "L1:" in l2 and "L2:" in l1:
               l1, l2 = l2, l1
           print ("first",l1,"second",l2)
           bad_cat.extend([l1, l2])
           for a1,a2 in allowed_cat:
               if l1.strip().lower()==a1.strip().lower() and l2.strip().lower()==a2.strip().lower():
                 bad_cat=[]
           if bad_cat:
               msg = "categories contains non-standard category: %s. " % ", ".join(bad_cat)
               msg += "Please check the allowed categories list and fix."
               evgenLog.error(msg)
               if officialJO:
                   import sys
                   sys.exit(1)
    else:
        evgenLog.warning("Could not find CategoryList.txt file ", lkwpath, " in $DATAPATH")
