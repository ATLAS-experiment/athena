#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Disable flake8 checking due to the use of 'exec':
# flake8: noqa
#

from AthenaCommon.Logging import logging
from TriggerMenuMT.L1.Menu.MenuMapping import MenuMetaInfo
log = logging.getLogger(__name__)

# The trigger types
from ..Base.Items import MenuItem
from .TriggerTypeDef import TT
from .ItemDef import ItemDef as ItemDefRun3

class ItemDef:
    
    @staticmethod
    def threshold_conditions(tc) -> type:
        d = ItemDefRun3.threshold_conditions(tc)

        # bunchgroup-based conditions masks
        d.physcond = d.BGRP0 & d.BGRP1
        d.calibcond = d.BGRP0 & d.BGRP2
        d.cosmiccond = d.BGRP0 & d.BGRP3
        d.unpaired_isocond = d.BGRP0 & d.BGRP4 # unpaired isolated (satellite bunches)
        d.unpaired_nonisocond = d.BGRP0 & d.BGRP5 # unpaired non-isolated (parasitic bunches)
        d.firstempty = d.BGRP0 & d.BGRP6
        d.bgrp7cond = d.BGRP0 & d.BGRP7 # No unpaired anymore
        d.bgrp9cond = d.BGRP0 & d.BGRP9
        d.bgrp11cond = d.BGRP0 & d.BGRP11
        d.bgrp12cond = d.BGRP0 & d.BGRP12
        d.bgrp13cond = d.BGRP0 & d.BGRP13 #UNPAIREDB1
        d.bgrp14cond = d.BGRP0 & d.BGRP14 #UNPAIREDB2
        d.bgrp10cond = d.BGRP0 & d.BGRP10
        d.firstintrain = d.BGRP0 & d.BGRP8
        d.physcond_or_unpaired_isocond = d.BGRP0 & (d.BGRP1 | d.BGRP4)
        return d

    @staticmethod
    def registerItems(tc, menuInfo: MenuMetaInfo):
        if menuInfo.run != 4:
            return
        
        menuName = menuInfo.menuBaseName

        # Phase-II
        d = ItemDef.threshold_conditions(tc)

        ItemDefRun3.registerRequiredItems(d, menuName)

        log.info("Adding extra run4 L1 items")
        
        MenuItem('L1_eEM10L_MU8F'   ).setLogic( d.eEM10L & d.MU8F     & d.physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_cTAU30M' ).setLogic( d.MU5VF  & d.cTAU30M  & d.physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ40'         ).setLogic( d.jJ40.x(3)           & d.physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM20M'        ).setLogic( d.eEM20M              & d.physcond).setTriggerType(TT.calo)     
        MenuItem('L1_eEM24M'        ).setLogic( d.eEM24M              & d.physcond).setTriggerType(TT.calo)

        # HL-LHC TDR inspired
        MenuItem('L1_eEM10L_MU5VF').setLogic( d.eEM10L & d.MU5VF     & d.physcond).setTriggerType(TT.muon)
        MenuItem('L1_jJ70'        ).setLogic( d.jJ70                 & d.physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ70'       ).setLogic( d.jJ70.x(3)            & d.physcond).setTriggerType(TT.calo)
        MenuItem('L1_2jJ70_jXE80' ).setLogic( d.jJ70.x(2) & d.jXE80  & d.physcond).setTriggerType(TT.calo)
