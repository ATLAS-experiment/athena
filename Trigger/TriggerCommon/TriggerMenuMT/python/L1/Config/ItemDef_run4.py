#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Disable flake8 checking due to the use of 'exec':
# flake8: noqa
#

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

# The trigger types
from ..Base.Items import MenuItem
from .TriggerTypeDef import TT

class ItemDef_run4:
    
    @staticmethod
    def registerItems(d, menuName):
        if "run4" not in menuName:
            return

        log.info("Adding extra run4 L1 items")
        physcond = d.BGRP0 & d.BGRP1
        
        MenuItem('L1_eEM10L_MU8F'   ).setLogic( d.eEM10L & d.MU8F     & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_cTAU30M' ).setLogic( d.MU5VF  & d.cTAU30M  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ40'         ).setLogic( d.jJ40.x(3)           & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM20M'         ).setLogic( d.eEM20M           & physcond).setTriggerType(TT.calo)     
        MenuItem('L1_eEM24M'         ).setLogic( d.eEM24M           & physcond).setTriggerType(TT.calo)

        # HL-LHC TDR inspired
        MenuItem('L1_eEM10L_MU5VF'   ).setLogic( d.eEM10L & d.MU5VF     & physcond).setTriggerType(TT.muon)
        MenuItem('L1_jJ70'          ).setLogic( d.jJ70                  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ70'         ).setLogic( d.jJ70.x(3)             & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2jJ70_jXE80'   ).setLogic( d.jJ70.x(2) & d.jXE80   & physcond).setTriggerType(TT.calo)
