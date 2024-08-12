# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# MC_pp_run4_v1.py menu for Phase-II developments 
#
# ALL CHANGES TO THE RUN 4 MENU SHOULD BE MADE TO RELEASE 25 OR GREATER
#
#------------------------------------------------------------------------#

import TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 as physics_menu 

def setupMenu():

    from AthenaCommon.Logging import logging
    log = logging.getLogger( __name__ )
    log.info('[setupMenu] going to add the MC menu chains now')
    
    chains = physics_menu.setupMenu()
    return chains
