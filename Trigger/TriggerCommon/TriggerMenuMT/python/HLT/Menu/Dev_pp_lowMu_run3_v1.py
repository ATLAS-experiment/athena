# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_lowMu_run3_v1.py menu for Run 3 development
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from .SignatureDicts import ChainStore

import TriggerMenuMT.HLT.Menu.PhysicsP1_pp_lowMu_run3_v1 as physics_menu


def getDevLowMuSignatures():
    chains = ChainStore()

    chains['Muon'] += [
    ]

    chains['Egamma'] += [
    ]

    chains['Jet'] += [
    ]

    chains['Combined'] += [
    ]

    chains['MinBias'] += [
    ]

    chains['HeavyIon'] += [
    ]

    chains['Streaming'] += [
    ]

    return chains

def setupMenu():
    chains = physics_menu.setupMenu()

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    for sig,chainsInSig in getDevLowMuSignatures().items():
        chains[sig] += chainsInSig

    return chains
