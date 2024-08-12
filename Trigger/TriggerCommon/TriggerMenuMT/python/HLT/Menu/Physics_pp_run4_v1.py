# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Physics_pp_run4_v1.py menu for Phase-II development
#
# ALL CHANGES TO THE RUN 4 MENU SHOULD BE MADE TO RELEASE 25 OR GREATER
#
#------------------------------------------------------------------------#

from .SignatureDicts import ChainStore

def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Physics menu chains now')

    chains = ChainStore()
    chains['Muon'] = []

    chains['Egamma'] = []

    chains['Tau'] = []

    chains['Jet'] = []

    chains['Bjet'] = []

    chains['MET'] = []

    chains['Bphysics'] = []

    chains['Combined'] = []

    chains['Monitor'] = []

    chains['Calib'] += []

    chains['UnconventionalTracking'] += []

    chains['EnhancedBias'] += []

    chains['Streaming'] += []

    chains['Beamspot'] += []

    return chains
