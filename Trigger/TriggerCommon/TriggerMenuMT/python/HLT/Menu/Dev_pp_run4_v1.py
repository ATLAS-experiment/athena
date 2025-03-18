# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Dev_pp_run4_v1.py menu for Phase-II developments 
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
# ['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],

import TriggerMenuMT.HLT.Menu.MC_pp_run4_v1 as mc_menu


DevGroup = ['Development']

def setupMenu():

    chains = mc_menu.setupMenu()

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the Dev menu chains now')

    chains['Muon'] += []
    chains['Egamma'] += []
    chains['Tau'] += []
    chains['Jet'] += []
    chains['Bjet'] += []
    chains['MET'] += []
    chains['Bphysics'] += []
    chains['UnconventionalTracking'] += []
    chains['Combined'] += []
    chains['Beamspot'] += []
    chains['MinBias'] += []
    chains['Calib'] += []
    chains['Streaming'] += []
    chains['Monitor'] += []
    return chains
