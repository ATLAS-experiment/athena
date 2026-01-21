# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from AthenaCommon.SystemOfUnits import GeV
import os

# Get logger
from AthenaCommon.Logging import logging
log = logging.getLogger("EvtGenConfig")

def EvtGenCfg(flags, 
              decayFile = None,
              whiteList = None,
              allowAllKnownDecays = False,
              auxfiles = None):

    # Set defaults
    if flags.Beam.Energy*2/GeV > 13001.:
        decayfile_str = "2022inclusive_BELLE.dec"
        log.info("Belle decay table updated Nov 2022")  
    elif "EVTGENVER" in os.environ:
        evtgenver = float(str(os.environ['EVTGENVER'])[:3])
        log.info(" ver of EvtGen ", evtgenver)
        decayfile_str =  "2014Inclusive_17.dec" if evtgenver == 1.7 else "2014Inclusive.dec"
    else:
        log.info("EVTGENVER not available !!! assuming version == 1.7")
        decayfile_str = "2014Inclusive_17.dec"
    default_auxfiles = [decayfile_str]

    default_whitelist = [-411, -421, -10411, -10421, -413, -423,
                        -10413, -10423, -20413, -20423, -415, -425, -431, -10431, -433, -10433, -20433,
                        -435, -511, -521, -10511, -10521, -513, -523, -10513, -10523, -20513, -20523,
                        -515, -525, -531, -10531, -533, -10533, -20533, -535, -541, -10541, -543,
                        -10543, -20543, -545, -441, -10441, -100441, -443, -10443, -20443, -100443,
                        -30443, -9000443, -9010443, -9020443, -445, -100445, -551, -10551, -100551,
                        -110551, -200551, -210551, -553, -10553, -20553, -30553, -100553, -110553,
                        -120553, -130553, -200553, -210553, -220553, -300553, -9000553, -9010553, -555,
                        -10555, -20555, -100555, -110555, -120555, -200555, -557, -100557, -4122, -4222,
                        -4212, -4112, -4224, -4214, -4114, -4232, -4132, -4322, -4312, -4324, -4314,
                        -4332, -4334, -4412, -4422, -4414, -4424, -4432, -4434, -4444, -5122, -5112,
                        -5212, -5222, -5114, -5214, -5224, -5132, -5232, -5312, -5322, -5314, -5324,
                        -5332, -5142, -5242, -5412, -5422, -5414, -5424, -5342, -5432, -5434, -5442,
                        -5444, -5512, -5522, -5514, -5524, -5532, -5534, -5542, -5544, -5554, -204126,
                        -104312, -104322, -105122, -105312, -105322, -104124, -104314, -104324, 411,
                        421, 10411, 10421, 413, 423, 10413, 10423, 20413, 20423, 415, 425, 431, 10431,
                        433, 10433, 20433, 435, 511, 521, 10511, 10521, 513, 523, 10513, 10523, 20513,
                        20523, 515, 525, 531, 10531, 533, 10533, 20533, 535, 541, 10541, 543, 10543,
                        20543, 545, 441, 10441, 100441, 443, 10443, 20443, 100443, 30443, 9000443,
                        9010443, 9020443, 445, 100445, 551, 10551, 100551, 110551, 200551, 210551, 553,
                        10553, 20553, 30553, 100553, 110553, 120553, 130553, 200553, 210553, 220553,
                        300553, 9000553, 9010553, 555, 10555, 20555, 100555, 110555, 120555, 200555,
                        557, 100557, 4122, 4222, 4212, 4112, 4224, 4214, 4114, 4232, 4132, 4322, 4312,
                        4324, 4314, 4332, 4334, 4412, 4422, 4414, 4424, 4432, 4434, 4444, 5122, 5112,
                        5212, 5222, 5114, 5214, 5224, 5132, 5232, 5312, 5322, 5314, 5324, 5332, 5142,
                        5242, 5412, 5422, 5414, 5424, 5342, 5432, 5434, 5442, 5444, 5512, 5522, 5514,
                        5524, 5532, 5534, 5542, 5544, 5554, 204126, 104312, 104322, 105122, 105312,
                        105322, 104124, 104314, 104324]

    # Merge custom settings with defaults
    whiteList = default_whitelist if whiteList is None else list(set(whiteList+default_whitelist))
    auxfiles = default_auxfiles if auxfiles is None else list(set(auxfiles+default_auxfiles))
    if decayFile is None:
        decayFile = decayfile_str

    # Define CA object
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator)) 
    ca.addEventAlgo(
        CompFactory.EvtInclusiveDecay(
          "EvtInclusiveDecay", 
          decayFile = decayFile,
          allowAllKnownDecays = allowAllKnownDecays,
          whiteList = whiteList
        )
    )

    # Announce generator to service
    from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg
    ca.merge(GeneratorInfoSvcCfg(flags, Generators=["EvtGen"]), sequenceName=EvgenSequence.Generator.value)

    # Copy necessary files
    from PyJobTransformsCore.trfutil import get_files
    get_files(auxfiles, keepDir=False, errorIfNotFound=True)

    return ca