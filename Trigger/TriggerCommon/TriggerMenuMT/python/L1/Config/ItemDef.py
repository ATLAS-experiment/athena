#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Disable flake8 checking due to the use of 'exec':
# flake8: noqa
#

from collections import defaultdict as ddict
import re, sys
import traceback

from ..Base.L1MenuFlags import L1MenuFlags
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

# The trigger types
from ..Base.Limits import Limits
from ..Base.Logic import Logic, Not
from ..Base.Items import MenuItem, meta_d
from ..Base.Thresholds import TopoThreshold
from ..Base.CTPCondition import ThrCondition, InternalTrigger  # noqa: F401
from .TriggerTypeDef import TT
from .ItemDef_run4 import ItemDef_run4


class ItemDef:
    """
    Defines any items that might be needed in Run2
    """

    otherTopoNames = ddict(list)

    @staticmethod
    def threshold_conditions(tc):
        class d(metaclass=meta_d): pass

        # ... and make them accessible by their name
        for thr in tc.getDefinedThresholds():
            setattr(d, thr.getVarName(), ThrCondition(thr))
            if isinstance(thr, TopoThreshold):
                ItemDef.otherTopoNames[thr.getVarName().split('_',1)[-1]].append( thr.getVarName() )

        # InternalTriggers
        for i in range( Limits.NumBunchgroups ):
            setattr (d, "BGRP%i"%i,  InternalTrigger('BGRP%i'%i))

        for i in range( Limits.NumRndmTriggers ):
            setattr (d, "RNDM%i"%i, InternalTrigger('RNDM%i'%i))

        return d

    @staticmethod
    def registerItems(tc, menuName):
        """Register L1 items for further use"""

        # dear Menu Experts, please note that it is only necessary to
        # check the menu name if a certain item needs to be defined
        # differently in different menus

        d = ItemDef.threshold_conditions(tc)

        # Setting up bunch group and BPTX conditions
        physcond            = d.BGRP0 & d.BGRP1
        calibcond           = d.BGRP0 & d.BGRP2
        cosmiccond          = d.BGRP0 & d.BGRP3
        unpaired_isocond    = d.BGRP0 & d.BGRP4 # unpaired isolated (satellite bunches)
        unpaired_nonisocond = d.BGRP0 & d.BGRP5 # unpaired non-isolated (parasitic bunches)
        firstempty          = d.BGRP0 & d.BGRP6
        bgrp7cond           = d.BGRP0 & d.BGRP7 # No unpaired anymore
        bgrp9cond           = d.BGRP0 & d.BGRP9
        bgrp11cond          = d.BGRP0 & d.BGRP11
        bgrp12cond          = d.BGRP0 & d.BGRP12
        bgrp13cond          = d.BGRP0 & d.BGRP13 #UNPAIREDB1
        bgrp14cond          = d.BGRP0 & d.BGRP14 #UNPAIREDB2
        bgrp10cond          = d.BGRP0 & d.BGRP10
        firstintrain        = d.BGRP0 & d.BGRP8
        physcond_or_unpaired_isocond    = d.BGRP0 & (d.BGRP1 | d.BGRP4)

        # partition 1
        #bgrpcond1           = d.BGRP0 & d.BGRP11
        #calibcond1          = d.BGRP0 & d.BGRP12

        # partition 2
        #bgrpcond2           = d.BGRP0 & d.BGRP14


        MenuItem.currentPartition = 1

        # ZDC

        # old Run-3 configurations 
        #ZDC_A_C = d.ZDC_A & d.ZDC_C
        #VZDC_A_C = Not(d.ZDC_A) & Not(d.ZDC_C)

        # new ZDC configuration for Run-3 (ATR-24734)
        ZDC_comb0 = Not(d.ZDC_2) & Not(d.ZDC_1) & Not(d.ZDC_0) # this means no signal! to be used ONLY in add with other inputs
        ZDC_comb1 = Not(d.ZDC_2) & Not(d.ZDC_1) & d.ZDC_0
        ZDC_comb2 = Not(d.ZDC_2) & d.ZDC_1      & Not(d.ZDC_0)
        ZDC_comb3 = Not(d.ZDC_2) & d.ZDC_1      & d.ZDC_0
        ZDC_comb4 = d.ZDC_2      & Not(d.ZDC_1) & Not(d.ZDC_0)
        ZDC_comb5 = d.ZDC_2      & Not(d.ZDC_1) & d.ZDC_0
        ZDC_comb6 = d.ZDC_2      & d.ZDC_1      & Not(d.ZDC_0)
        ZDC_comb7 = d.ZDC_2      & d.ZDC_1      & d.ZDC_0

        # combined signals for heavy ion runs
        PHYS_VZDC_A_VZDC_C         = ZDC_comb0
        PHYS_1TO4ZDC_A_VZDC_C      = ZDC_comb4
        PHYS_VZDC_A_1TO4ZDC_C      = ZDC_comb6
        PHYS_1TO4ZDC_A_1TO4ZDC_C   = ZDC_comb1
        PHYS_5ZDC_A_VZDC_C         = ZDC_comb5
        PHYS_VZDC_A_5ZDC_C         = ZDC_comb7
        PHYS_ZDC_1TO4XOR5          = ZDC_comb2
        PHYS_5ZDC_A_5ZDC_C         = ZDC_comb3

        PHYS_ZDC_A_VZDC_C          = d.ZDC_2 & Not(d.ZDC_1)
        PHYS_VZDC_A_ZDC_C          = d.ZDC_2 & d.ZDC_1
        PHYS_ZDC_XOR4              = d.ZDC_2 & Not(d.ZDC_0)
        PHYS_ZDC_5XOR              = d.ZDC_2 & d.ZDC_0
        PHYS_1ZDC_NZDC             = Not(d.ZDC_2) & ( d.ZDC_1 | d.ZDC_0 ) & Not( d.ZDC_1 & d.ZDC_0)

        #ATR-26984 refine ZDC_A and ZDC_C logic
        #ZDC_A, ZDC_C and ZDC_A_C redefined due to O+O configuration, ATR-30690
        #ZDC_A     = ( Not(d.ZDC_2) & ( d.ZDC_1 | d.ZDC_0 ) ) | ( d.ZDC_2 & Not(d.ZDC_1) )
        #ZDC_C     = d.ZDC_1 | (d.ZDC_0 & Not(d.ZDC_2) )

        #ZDC_A_C   = Not(d.ZDC_2) & ( d.ZDC_1 | d.ZDC_0 )
        #ZDC_AND   = ZDC_A_C
        VZDC_A_C  = ZDC_comb0
        ZDC_XOR   = d.ZDC_2
        #VZDC_AORC = Not(ZDC_A) | Not(ZDC_C)
        ZDCOR = Not(ZDC_comb0)

        # ZDC configuration for LHCf+ZDC special run in Sep. 2022
        # rename existing ZDC configuration to match request in ATR-26051
        ZDC_VETO          = ZDC_comb0
        ZDC_XOR_E1_E3     = ZDC_comb1
        ZDC_XOR_E2        = ZDC_comb2
        ZDC_E1_AND_E1     = ZDC_comb3
        ZDC_E1_AND_E2ORE3 = ZDC_comb4
        ZDC_E2_AND_E2     = ZDC_comb5
        ZDC_E2_AND_E3     = ZDC_comb6
        ZDC_E3_AND_E3     = ZDC_comb7
        # (additional) combined ZDC signals for LHCf+ZDC special run
        ZDC_OR = Not(ZDC_VETO)
        ZDC_A_AND_C = d.ZDC_2 | (d.ZDC_1 & d.ZDC_0)
        
        # new HG items for the upcoming O+O runs, ATR-30690
        ZDC_XnXn = ZDC_comb2 | ZDC_comb4 | ZDC_comb5 | ZDC_comb7
        ZDC_XnYn = ZDC_comb4 | ZDC_comb5 | ZDC_comb7
        ZDC_XnZn = ZDC_comb7
        ZDC_Xn_XOR = ZDC_comb1 | ZDC_comb3 | ZDC_comb6
        ZDC_Yn_XOR = ZDC_comb3 | ZDC_comb6
        ZDC_Zn_XOR = ZDC_comb6
        ZDC_Yn = Not(ZDC_comb0 | ZDC_comb1)
        ZDC_Zn = d.ZDC_2

        #new ZDC items defiition, ATR-29719
        ZDC_alt_comb0 = Not(d.ZDC_ALT_2) & Not(d.ZDC_ALT_1) & Not(d.ZDC_ALT_0) # this means no signal!
        ZDC_alt_comb1 = Not(d.ZDC_ALT_2) & Not(d.ZDC_ALT_1) & d.ZDC_ALT_0
        ZDC_alt_comb2 = Not(d.ZDC_ALT_2) & d.ZDC_ALT_1      & Not(d.ZDC_ALT_0)
        ZDC_alt_comb3 = Not(d.ZDC_ALT_2) & d.ZDC_ALT_1      & d.ZDC_ALT_0
        ZDC_alt_comb4 = d.ZDC_ALT_2      & Not(d.ZDC_ALT_1) & Not(d.ZDC_ALT_0)
        ZDC_alt_comb5 = d.ZDC_ALT_2      & Not(d.ZDC_ALT_1) & d.ZDC_ALT_0
        ZDC_alt_comb6 = d.ZDC_ALT_2      & d.ZDC_ALT_1      & Not(d.ZDC_ALT_0)
        ZDC_alt_comb7 = d.ZDC_ALT_2      & d.ZDC_ALT_1      & d.ZDC_ALT_0

        #new ZDC items in HI running
        ZDC_HELT15 = ZDC_alt_comb1 #hadronic energy sum < 15 TeV
        ZDC_HELT20 = ZDC_alt_comb2 #hadronic energy sum < 20 TeV
        ZDC_HELT25 = ZDC_alt_comb3 #hadronic energy sum < 25 TeV
        ZDC_HELT35 = ZDC_alt_comb4 #hadronic energy sum < 35 TeV
        ZDC_HELT50 = ZDC_alt_comb5 #hadronic energy sum < 50 TeV – supporting only
        ZDC_HELT100 = ZDC_alt_comb6 #hadronic energy sum < 100 TeV - for PU rejection

        #new ZDC items in pp running
        ZDC_PP_C = Not(d.ZDC_ALT_2) & d.ZDC_ALT_0
        ZDC_PP_A = Not(d.ZDC_ALT_2) & d.ZDC_ALT_1
        ZDC_PP_OR = ZDC_PP_A | ZDC_PP_C
        ZDC_PP_A_C = d.ZDC_ALT_1 & d.ZDC_ALT_0
        ZDC_PP_C2 = d.ZDC_ALT_2 & d.ZDC_ALT_0
        ZDC_PP_A2 = d.ZDC_ALT_2 & d.ZDC_ALT_1
        ZDC_PP_OR2 = ZDC_PP_C2 | ZDC_PP_A2
        
        # new LG items for the upcoming O+O runs, ATR-30690
        ZDC_LOR = Not(ZDC_alt_comb0)
        ZDC_A = d.ZDC_ALT_0
        ZDC_C = d.ZDC_ALT_1
        ZDC_A_C = d.ZDC_ALT_0 & d.ZDC_ALT_1
        ZDC_YnYn = ZDC_alt_comb7
        ZDC_AND   = ZDC_A_C
        VZDC_AORC = Not(ZDC_A) | Not(ZDC_C)

        MenuItem('L1_EM3'       ).setLogic( d.EM3        & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM12'      ).setLogic( d.EM12       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15'      ).setLogic( d.EM15       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM10VH'    ).setLogic( d.EM10VH     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM20VH'    ).setLogic( d.EM20VH     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM18VHI'   ).setLogic( d.EM18VHI    & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI'   ).setLogic( d.EM15VHI    & physcond).setTriggerType( TT.calo )
        #heavy-ions ATR-27791
        MenuItem('L1_EM10'      ).setLogic( d.EM10       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_2EM15'      ).setLogic( d.EM15.x(2)       & physcond).setTriggerType( TT.calo )

        # Phase-I
        MenuItem('L1_eEM1'      ).setLogic( d.eEM1       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM2'      ).setLogic( d.eEM2       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM5'      ).setLogic( d.eEM5       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM7'      ).setLogic( d.eEM7       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM9'      ).setLogic( d.eEM9       & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM10L'    ).setLogic( d.eEM10L     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM12'     ).setLogic( d.eEM12      & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM12L'    ).setLogic( d.eEM12L     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM15'     ).setLogic( d.eEM15      & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18'     ).setLogic( d.eEM18      & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18L'    ).setLogic( d.eEM18L     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18M'    ).setLogic( d.eEM18M     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM22M'    ).setLogic( d.eEM22M     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM24L'    ).setLogic( d.eEM24L     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM24VM'   ).setLogic( d.eEM24VM    & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM26'     ).setLogic( d.eEM26      & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM26L'    ).setLogic( d.eEM26L     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM26M'    ).setLogic( d.eEM26M     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM26T'    ).setLogic( d.eEM26T     & physcond).setTriggerType( TT.calo ) 
        MenuItem('L1_eEM28M'    ).setLogic( d.eEM28M     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM40L'    ).setLogic( d.eEM40L     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM1_EMPTY'        ).setLogic(d.eEM1 & cosmiccond      ).setTriggerType( TT.calo )
        MenuItem('L1_eEM5_EMPTY'        ).setLogic(d.eEM5 & cosmiccond      ).setTriggerType( TT.calo )
        MenuItem('L1_eEM2_EMPTY'        ).setLogic(d.eEM2 & cosmiccond      ).setTriggerType( TT.calo )
        MenuItem('L1_eEM9_EMPTY'        ).setLogic(d.eEM9 & cosmiccond      ).setTriggerType( TT.calo )
        MenuItem('L1_2eEM9_EMPTY'       ).setLogic(d.eEM9.x(2) & cosmiccond ).setTriggerType( TT.calo )
        MenuItem('L1_eEM9_UNPAIRED_ISO' ).setLogic(d.eEM9 & unpaired_isocond).setTriggerType( TT.calo )
        MenuItem('L1_eEM15_EMPTY'       ).setLogic(d.eEM15 & cosmiccond     ).setTriggerType( TT.calo )
        # ATR-29025
        MenuItem('L1_DPHI-2eEM1_VjTE200_EMPTY'  ).setLogic(d.TOPO_23DPHI32_2eEM1s  & Not(d.jTE200) & cosmiccond ).setTriggerType( TT.calo )
        MenuItem('L1_DPHI-2eTAU1_VjTE200_EMPTY' ).setLogic(d.TOPO_23DPHI32_2eTAU1s & Not(d.jTE200) & cosmiccond ).setTriggerType( TT.calo )

        MenuItem('L1_eEM22A'    ).setLogic( d.eEM22A     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM22C'    ).setLogic( d.eEM22C     & physcond).setTriggerType( TT.calo )

        # PhaseI 2xEM and 3xEM
        MenuItem('L1_2eEM10L').setLogic(d.eEM10L.x(2) & physcond).setTriggerType(TT.calo) # prospective Run 4 L1 item, ATR-30180
        MenuItem('L1_2eEM12').setLogic(d.eEM12.x(2) & physcond).setTriggerType(TT.calo) #heavy ions, ATR-26333
        MenuItem('L1_2eEM12L').setLogic(d.eEM12L.x(2) & physcond).setTriggerType(TT.calo) #heavy ions, ATR-26333
        MenuItem('L1_2eEM18').setLogic(d.eEM18.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM18L').setLogic(d.eEM18L.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM18M').setLogic(d.eEM18M.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM24L').setLogic(d.eEM24L.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3eEM12L').setLogic(d.eEM12L.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM24L_3eEM12L').setLogic(d.eEM24L & d.eEM12L.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM40L_2eEM18L').setLogic(d.eEM40L & d.eEM18L.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM9').setLogic(d.eEM9.x(2) & physcond).setTriggerType(TT.calo)

        # EM and jet
        MenuItem('L1_J15p23ETA49' ).setLogic( d.J1523ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J15p24ETA49' ).setLogic( d.J1524ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J15p31ETA49' ).setLogic( d.J1531ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J20p28ETA31' ).setLogic( d.J2028ETA31 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J20p31ETA49' ).setLogic( d.J2031ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J30p31ETA49' ).setLogic( d.J3031ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J40p0ETA25'  ).setLogic( d.J400ETA25  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J50p31ETA49' ).setLogic( d.J5031ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J70p31ETA49' ).setLogic( d.J7031ETA49 & physcond).setTriggerType(TT.calo)

        MenuItem('L1_J15p0ETA25'  ).setLogic( d.J150ETA25 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J20p0ETA28'  ).setLogic( d.J200ETA28 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J25p0ETA23'  ).setLogic( d.J250ETA23 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J35p0ETA23'  ).setLogic( d.J350ETA23 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J20p0ETA49'  ).setLogic( d.J200ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J30p0ETA49'  ).setLogic( d.J300ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J25p0ETA23_2J15p31ETA49'  ).setLogic( d.J250ETA23 & d.J1531ETA49.x(2) & physcond).setTriggerType(TT.calo)

        # HI
        MenuItem('L1_EM3_EMPTY'       ).setLogic(d.EM3 & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_EM7_EMPTY'          ).setLogic(d.EM7 & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_EM7_FIRSTEMPTY'     ).setLogic(d.EM7 & firstempty).setTriggerType( TT.calo )
        MenuItem('L1_eEM9_FIRSTEMPTY'     ).setLogic(d.eEM9 & firstempty).setTriggerType( TT.calo )

        MenuItem('L1_J10_VTE100'         ).setLogic( d.J10  & Not(d.TE100) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_J10_VTE200'         ).setLogic( d.J10  & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_J10_VTE300'         ).setLogic( d.J10  & Not(d.TE300) & physcond).setTriggerType( TT.calo )

        MenuItem('L1_J12_VTE100'         ).setLogic( d.J12  & Not(d.TE100) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_J12_VTE200'         ).setLogic( d.J12  & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        # L1 items for IDCalibPEB,  IDCalib in heavy ion collisions, ATR-26405
        MenuItem('L1_J100_VTE200'        ).setLogic( d.J100 & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_J30_VTE200'         ).setLogic( d.J30  & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_XE35_VTE200'        ).setLogic( d.XE35 & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_XE50_VTE200'        ).setLogic( d.XE50 & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jJ30_VjTE200'         ).setLogic( d.jJ30  & Not(d.jTE200) & physcond).setTriggerType( TT.calo )

        MenuItem('L1_VTE200'             ).setLogic( Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE20_VTE200'        ).setLogic( d.TE20 & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE50_VTE200'        ).setLogic( d.TE50 & Not(d.TE200) & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TE5_VTE200'         ).setLogic( d.TE5  & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE5_VTE100'         ).setLogic( d.TE5  & Not(d.TE100) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE5_VTE50'          ).setLogic( d.TE5  & Not(d.TE50)  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE5_VTE20'          ).setLogic( d.TE5  & Not(d.TE20)  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TE4_VTE200'         ).setLogic( d.TE4  & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE4_VTE100'         ).setLogic( d.TE4  & Not(d.TE100) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE4_VTE50'          ).setLogic( d.TE4  & Not(d.TE50)  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TE3_VTE50'          ).setLogic( d.TE3  & Not(d.TE50)  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TE3_VTE200'         ).setLogic( d.TE3  & Not(d.TE200)  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TE4_VTE200_EMPTY'   ).setLogic( d.TE4  & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_TE5_VTE200_EMPTY'   ).setLogic( d.TE5  & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )

        #HI phase-1
        # additional L1_GAP items defined using jTEFWDA or JTEFWDC

        GAPA = Not(d.jTEFWDA5)
        GAPC = Not(d.jTEFWDC5)
        GAPAC = Not(d.jTEFWDA5) & Not(d.jTEFWDC5)

        MenuItem('L1_GAP_A').setLogic( GAPA & physcond).setTriggerType(TT.calo)
        MenuItem('L1_GAP_C').setLogic( GAPC & physcond).setTriggerType(TT.calo)
        MenuItem('L1_GAP_AANDC').setLogic( GAPAC  & physcond).setTriggerType(TT.calo)

        MenuItem('L1_eEM1_VjTE200').setLogic( d.eEM1      & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_jTE3_VjTE200').setLogic( d.eEM1 & d.jTE3 &    Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU1_jTE3_VjTE200').setLogic( d.eTAU1 & d.jTE3 &    Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_jTE3_VjTE200_EMPTY').setLogic( d.eEM1 & d.jTE3   & Not(d.jTE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_jTE4_VjTE200').setLogic( d.eEM1 & d.jTE4 &    Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU1_jTE4_VjTE200').setLogic( d.eTAU1 & d.jTE4 &    Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_jTE4_VjTE200_GAP_AANDC').setLogic( d.eEM1 & d.jTE4 &    Not(d.jTE200) & GAPAC & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_jTE4_VjTE200_EMPTY').setLogic( d.eEM1 & d.jTE4   & Not(d.jTE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eEM2_VjTE200').setLogic( d.eEM2      & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM2_jTE3_VjTE200').setLogic( d.eEM2 & d.jTE3    & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM2_jTE4_VjTE200').setLogic( d.eEM2 & d.jTE4    & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM2_jTE4_VjTE200_GAP_AANDC').setLogic( d.eEM2 & d.jTE4    & Not(d.jTE200) & GAPAC  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM5_VjTE200').setLogic( d.eEM5      & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM5_VjTE200_GAP_AANDC').setLogic( d.eEM5      & Not(d.jTE200) & GAPAC & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM1_VjTE200').setLogic( d.eEM1.x(2)      & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM2_VjTE200').setLogic( d.eEM2.x(2)      & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM1_VjTE200_EMPTY').setLogic( d.eEM1.x(2)      & Not(d.jTE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM1_VjTE200_UNPAIRED_ISO').setLogic( d.eEM1.x(2)      & Not(d.jTE200) & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM1_VjTE200_UNPAIRED_NONISO').setLogic( d.eEM1.x(2)      & Not(d.jTE200) & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_2eTAU1_VjTE200').setLogic( d.eTAU1.x(2)      & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eTAU1_VjTE200_EMPTY').setLogic( d.eTAU1.x(2)      & Not(d.jTE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_2eTAU1_VjTE200_UNPAIRED_ISO').setLogic( d.eTAU1.x(2)      & Not(d.jTE200) & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_2eTAU1_VjTE200_UNPAIRED_NONISO').setLogic( d.eTAU1.x(2)      & Not(d.jTE200) & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU1_TRT_VjTE200_EMPTY').setLogic( d.eTAU1 & d.NIMTRT & Not(d.jTE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU1_TRT_VjTE200_UNPAIRED_ISO').setLogic( d.eTAU1 & d.NIMTRT  & Not(d.jTE200) & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU1_TRT_VjTE200_UNPAIRED_NONISO').setLogic( d.eTAU1 & d.NIMTRT   & Not(d.jTE200) & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM1_VjTE200_GAP_AANDC').setLogic( d.eEM1.x(2)      & Not(d.jTE200) & GAPAC  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eTAU1_VjTE200_GAP_AANDC').setLogic( d.eTAU1.x(2)      & Not(d.jTE200) & GAPAC  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM9_VjTE200').setLogic( d.eEM9      & Not(d.jTE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM9_VjTE200_EMPTY').setLogic( d.eEM9      & Not(d.jTE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_TRT_VjTE50').setLogic( d.eEM1 & d.NIMTRT & Not(d.jTE50) & physcond)
        MenuItem('L1_eTAU1_TRT_VjTE50').setLogic( d.eTAU1 & d.NIMTRT & Not(d.jTE50) & physcond)

        #mixed items: eEM + legacy TE (ATR-22067), HI run 2023
        MenuItem('L1_eEM1_TE4_VTE200').setLogic( d.eEM1 & d.TE4  &   Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM2_TE4_VTE200').setLogic( d.eEM2 & d.TE4  &   Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_TE4_VTE200_EMPTY').setLogic( d.eEM1 & d.TE4   & Not(d.TE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_VTE200').setLogic( d.eEM1      & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM1_VTE200').setLogic( d.eEM1.x(2)      & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2eEM2_VTE200').setLogic( d.eEM2.x(2)      & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM1_VZDC_A_VZDC_C_VTE100' ).setLogic( d.eEM1 & PHYS_VZDC_A_VZDC_C & Not(d.TE100)  & physcond)
        MenuItem('L1_eEM1_ZDC_XOR4_VTE100' ).setLogic( d.eEM1 & PHYS_ZDC_XOR4 & Not(d.TE100)   & physcond)
        MenuItem('L1_eEM2_VZDC_A_VZDC_C_VTE100' ).setLogic( d.eEM2 & PHYS_VZDC_A_VZDC_C & Not(d.TE100)  & physcond)
        MenuItem('L1_eEM2_ZDC_XOR4_VTE100' ).setLogic( d.eEM2 & PHYS_ZDC_XOR4 & Not(d.TE100) &  physcond)
        MenuItem('L1_eEM9_VTE200').setLogic( d.eEM9      & Not(d.TE200) & physcond).setTriggerType(TT.calo)

        MenuItem('L1_VjTE200_GAP_A'         ).setLogic(  Not(d.jTE200) & GAPA  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_VjTE200_GAP_C'         ).setLogic(  Not(d.jTE200) & GAPC  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTE3_VjTE200_GAP_A'         ).setLogic( d.jTE3  & Not(d.jTE200) & GAPA  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTE3_VjTE200_GAP_C'         ).setLogic( d.jTE3  & Not(d.jTE200) & GAPC  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTE5_VjTE200'         ).setLogic( d.jTE5  & Not(d.jTE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTE5_VjTE200_GAP_A'         ).setLogic( d.jTE5  & Not(d.jTE200) & GAPA  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTE5_VjTE200_GAP_C'         ).setLogic( d.jTE5  & Not(d.jTE200) & GAPC  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_gTE5_VjTE200'         ).setLogic( d.gTE5  & Not(d.jTE200) & physcond).setTriggerType( TT.calo )

        # NSW Monitoring
        MenuItem('L1_NSW_MONITOR').setLogic(d.NSWMon & physcond ).setTriggerType(TT.nsw)

        # MUON ctpid=[0x20;0x2f]
        # RUn3 thresholds
        MenuItem('L1_MU3V'       ).setLogic( d.MU3V       & physcond).setTriggerType(TT.muon) # ~ Run2 MU4 eff
        MenuItem('L1_MU5VF'      ).setLogic( d.MU5VF      & physcond).setTriggerType(TT.muon) # ~ Run2 MU6
        MenuItem('L1_MU8F'       ).setLogic( d.MU8F       & physcond).setTriggerType(TT.muon) # ~ Run2 MU10
        MenuItem('L1_MU8VF'      ).setLogic( d.MU8VF      & physcond).setTriggerType(TT.muon) # ~ Run2 MU11
        MenuItem('L1_MU14FCH'    ).setLogic( d.MU14FCH    & physcond).setTriggerType(TT.muon) # ~ Run2 MU20
        MenuItem('L1_MU14FCHR'   ).setLogic( d.MU14FCHR   & physcond).setTriggerType(TT.muon) # ~ Run2 MU21

        MenuItem('L1_MU3VF'      ).setLogic( d.MU3VF      & physcond).setTriggerType(TT.muon) # ~ Run2 MU4 rate
        MenuItem('L1_MU8FC'      ).setLogic( d.MU8FC      & physcond).setTriggerType(TT.muon) # Backup MU8F
        MenuItem('L1_MU8VFC'     ).setLogic( d.MU8VFC     & physcond).setTriggerType(TT.muon) # Backup MU8VF
        MenuItem('L1_MU15VFCH'   ).setLogic( d.MU15VFCH   & physcond).setTriggerType(TT.muon) # 

        MenuItem('L1_MU10BOM'    ).setLogic( d.MU10BOM    & physcond).setTriggerType(TT.muon) # Barrel-only close-by muons
        MenuItem('L1_MU20VFC'    ).setLogic( d.MU20VFC    & physcond).setTriggerType(TT.muon) # alignment with toroid off

        MenuItem('L1_MU10BO'     ).setLogic( d.MU10BO     & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU4BOM'     ).setLogic( d.MU4BOM     & physcond).setTriggerType(TT.muon) # Barrel-only close-by muons 
        MenuItem('L1_MU12BOM'    ).setLogic( d.MU12BOM    & physcond).setTriggerType(TT.muon) # Barrel-only close-by muons  

        # test items
        MenuItem('L1_MU3VC'      ).setLogic( d.MU3VC    & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU4BO'      ).setLogic( d.MU4BO    & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3EOF'     ).setLogic( d.MU3EOF   & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU8FH'      ).setLogic( d.MU8FH    & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU8EOF'     ).setLogic( d.MU8EOF   & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU9VF'      ).setLogic( d.MU9VF    & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU9VFC'     ).setLogic( d.MU9VFC   & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU12FCH'    ).setLogic( d.MU12FCH  & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU14EOF'    ).setLogic( d.MU14EOF  & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU15VFCHR'  ).setLogic( d.MU15VFCHR & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU18VFCH'   ).setLogic( d.MU18VFCH & physcond).setTriggerType(TT.muon)

        MenuItem('L1_2MU3V'        ).setLogic( d.MU3V.x(2)             & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3VF'       ).setLogic( d.MU3VF.x(2)            & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU5VF'       ).setLogic( d.MU5VF.x(2)            & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU8F'        ).setLogic( d.MU8F.x(2)             & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU8VF'       ).setLogic( d.MU8VF.x(2)            & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU14FCH'     ).setLogic( d.MU14FCH.x(2)          & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_2MU3V'  ).setLogic( d.MU5VF & d.MU3V.x(2)   & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_2MU3VF' ).setLogic( d.MU5VF & d.MU3VF.x(2)  & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU8F_2MU3V'   ).setLogic( d.MU8F & d.MU3V.x(2)    & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU8F_2MU5VF'  ).setLogic( d.MU8F & d.MU5VF.x(2)   & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU8VF_2MU5VF' ).setLogic( d.MU8VF & d.MU5VF.x(2)  & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_3MU3V'  ).setLogic( d.MU5VF & d.MU3V.x(3)   & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_3MU3VF' ).setLogic( d.MU5VF & d.MU3VF.x(3)  & physcond).setTriggerType(TT.muon)
        MenuItem('L1_3MU3V'        ).setLogic( d.MU3V.x(3)             & physcond).setTriggerType(TT.muon)
        MenuItem('L1_3MU3VF'       ).setLogic( d.MU3VF.x(3)            & physcond).setTriggerType(TT.muon)
        MenuItem('L1_3MU5VF'       ).setLogic( d.MU5VF.x(3)            & physcond).setTriggerType(TT.muon)
        MenuItem('L1_4MU3V'        ).setLogic( d.MU3V.x(4)             & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU5VF_3MU3V' ).setLogic( d.MU5VF.x(2) & d.MU3V.x(3)   & physcond).setTriggerType(TT.muon)

        MenuItem('L1_2MU14FCH_OVERLAY').setLogic( d.MU14FCH.x(2)       & physcond).setTriggerType(TT.zerobs)

        MenuItem('L1_LLPDPHI-jXE40-jJ40').setLogic( d.TOPO_0DPHI10_jXE40delay_jJ40s & physcond)
        MenuItem('L1_LLPNODPHI-jXE40-jJ40').setLogic( d.TOPO_0DPHI99_jXE40delay_jJ40s & physcond)
        
        # HI
        MenuItem('L1_MU3V_VTE10' ).setLogic( d.MU3V      & Not(d.TE10) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_VTE10').setLogic( d.MU3V.x(2) & Not(d.TE10) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_VTE50').setLogic( d.MU3V.x(2) & Not(d.TE50) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE10'  ).setLogic( d.MU3V      & d.TE10 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE20'  ).setLogic( d.MU3V      & d.TE20 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE30'  ).setLogic( d.MU3V      & d.TE30 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE40'  ).setLogic( d.MU3V      & d.TE40 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE50'  ).setLogic( d.MU3V      & d.TE50 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE70'  ).setLogic( d.MU3V      & d.TE70 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE90'  ).setLogic( d.MU3V      & d.TE90 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE120' ).setLogic( d.MU3V      & d.TE120 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE160' ).setLogic( d.MU3V      & d.TE160 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE10p24ETA49' ).setLogic( d.MU3V  & d.TE1024ETA49 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE15p24ETA49' ).setLogic( d.MU3V  & d.TE1524ETA49 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE20p24ETA49' ).setLogic( d.MU3V  & d.TE2024ETA49 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE25p24ETA49' ).setLogic( d.MU3V  & d.TE2524ETA49 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE30p24ETA49' ).setLogic( d.MU3V  & d.TE3024ETA49 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE35p24ETA49' ).setLogic( d.MU3V  & d.TE3524ETA49 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_TE40p24ETA49' ).setLogic( d.MU3V  & d.TE4024ETA49 & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_VTE50' ).setLogic( d.MU3V     & Not(d.TE50) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_VTE200' ).setLogic( d.MU3V    & Not(d.TE200) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_VTE50').setLogic( d.MU5VF    & Not(d.TE50) & physcond).setTriggerType(TT.muon)

        MenuItem('L1_MU3V_UNPAIRED_ISO'   ).setLogic(d.MU3V  & unpaired_isocond   ).setTriggerType( TT.muon )
        MenuItem('L1_MU3V_UNPAIRED_NONISO').setLogic(d.MU3V  & unpaired_nonisocond).setTriggerType( TT.muon )
        MenuItem('L1_MU3V_EMPTY'          ).setLogic(d.MU3V  & cosmiccond).setTriggerType( TT.muon )
        MenuItem('L1_MU5VF_EMPTY'         ).setLogic(d.MU5VF & cosmiccond).setTriggerType( TT.muon )
        MenuItem('L1_MU3V_FIRSTEMPTY'     ).setLogic(d.MU3V  & firstempty).setTriggerType( TT.muon )
        MenuItem('L1_MU5VF_FIRSTEMPTY'    ).setLogic(d.MU5VF & firstempty).setTriggerType( TT.muon )
        MenuItem('L1_MU8F_EMPTY'          ).setLogic(d.MU8F  & cosmiccond).setTriggerType( TT.muon )
        MenuItem('L1_MU8VF_EMPTY'         ).setLogic(d.MU8VF & cosmiccond).setTriggerType( TT.muon )
        MenuItem('L1_MU14FCH_FIRSTEMPTY'  ).setLogic(d.MU14FCH  & firstempty).setTriggerType(TT.muon)
        MenuItem('L1_MU14FCH_EMPTY'       ).setLogic(d.MU14FCH  & cosmiccond).setTriggerType(TT.muon)
        MenuItem('L1_MU14FCH_UNPAIRED_ISO').setLogic(d.MU14FCH  & unpaired_isocond).setTriggerType(TT.muon)
        MenuItem('L1_MU14FCHR_FIRSTEMPTY' ).setLogic(d.MU14FCHR & firstempty).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_EMPTY'           ).setLogic(d.MU3V.x(2)  & cosmiccond).setTriggerType( TT.muon )
        MenuItem('L1_2MU5VF_UNPAIRED_ISO'   ).setLogic(d.MU5VF.x(2) & unpaired_isocond).setTriggerType( TT.muon )
        MenuItem('L1_2MU5VF_UNPAIRED_NONISO').setLogic(d.MU5VF.x(2) & unpaired_nonisocond).setTriggerType( TT.muon )
        MenuItem('L1_2MU5VF_EMPTY'          ).setLogic(d.MU5VF.x(2) & cosmiccond).setTriggerType( TT.muon )
        MenuItem('L1_2MU5VF_FIRSTEMPTY'     ).setLogic(d.MU5VF.x(2) & firstempty).setTriggerType( TT.muon )

        # HI - phase-1
        MenuItem('L1_MU3V_VjTE50'  ).setLogic( d.MU3V      & Not(d.jTE50) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_VjTE200' ).setLogic( d.MU3V      & Not(d.jTE200) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_VjTE50').setLogic( d.MU5VF    & Not(d.jTE50) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_VjTE50').setLogic( d.MU3V.x(2) & Not(d.jTE50) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_VjTE50_GAP_AANDC'  ).setLogic( d.MU3V      &  Not(d.jTE50) & GAPAC  & physcond).setTriggerType(TT.muon)
        

        # EM and MU
        MenuItem('L1_EM3_MU14FCH'    ).setLogic( d.EM3        & d.MU14FCH  & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2eEM10L_MU8F'   ).setLogic( d.eEM10L.x(2) & d.MU8F     & physcond).setTriggerType(TT.muon)
        MenuItem('L1_eEM18L_MU8F'    ).setLogic( d.eEM18L     & d.MU8F     & physcond).setTriggerType(TT.muon)

        # TAU ctpid=[0x40:0x4f]
        MenuItem('L1_TAU2'  ).setLogic( d.HA2   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU3'  ).setLogic( d.HA3   & physcond).setTriggerType( TT.calo )
        # ATR-19359
        MenuItem('L1_TAU5'  ).setLogic( d.HA5   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_2TAU5' ).setLogic( d.HA5.x(2)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TAU6'  ).setLogic( d.HA6   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU8'  ).setLogic( d.HA8   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_2TAU8' ).setLogic( d.HA8.x(2)  & physcond).setTriggerType(TT.calo)

        MenuItem('L1_TAU12'  ).setLogic( d.HA12  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU12IL').setLogic( d.HA12IL & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU12IM').setLogic( d.HA12IM & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU12IT').setLogic( d.HA12IT & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU12I' ).setLogic( d.HA12I & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU15'  ).setLogic( d.HA15  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TAU20'  ).setLogic( d.HA20  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IL').setLogic( d.HA20IL  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM').setLogic( d.HA20IM  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IT').setLogic( d.HA20IT  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TAU20A' ).setLogic( d.HA20A  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20C' ).setLogic( d.HA20C  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TAU25'  ).setLogic( d.HA25  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU25IT').setLogic( d.HA25IT  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU25IM').setLogic( d.HA25IM  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU30'  ).setLogic( d.HA30  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU40'  ).setLogic( d.HA40  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU60'  ).setLogic( d.HA60  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU100' ).setLogic( d.HA100  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU90'  ).setLogic( d.HA90  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TAU8_UNPAIRED_ISO'   ).setLogic( d.HA8   & unpaired_isocond ).setTriggerType( TT.calo )

        MenuItem('L1_TAU8_UNPAIRED_NONISO').setLogic( d.HA8   & unpaired_nonisocond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU8_EMPTY'          ).setLogic( d.HA8   & cosmiccond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU8_FIRSTEMPTY'     ).setLogic( d.HA8   & firstempty ).setTriggerType( TT.calo )

        MenuItem('L1_TAU12_UNPAIRED_ISO'   ).setLogic( d.HA12   & unpaired_isocond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU12_UNPAIRED_NONISO').setLogic( d.HA12   & unpaired_nonisocond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU12_EMPTY'          ).setLogic( d.HA12  & cosmiccond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU12_FIRSTEMPTY'     ).setLogic( d.HA12   & firstempty ).setTriggerType( TT.calo )
        MenuItem('L1_TAU30_EMPTY'          ).setLogic( d.HA30   & cosmiccond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU30_UNPAIRED_ISO'   ).setLogic( d.HA30  & unpaired_isocond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU40_EMPTY'          ).setLogic( d.HA40   & cosmiccond ).setTriggerType( TT.calo )
        MenuItem('L1_TAU40_UNPAIRED_ISO'   ).setLogic( d.HA40  & unpaired_isocond ).setTriggerType( TT.calo )

        #Phase-I
        MenuItem('L1_eTAU1'  ).setLogic( d.eTAU1   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU12'  ).setLogic( d.eTAU12   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU20'  ).setLogic( d.eTAU20   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU20L' ).setLogic( d.eTAU20L  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU20M' ).setLogic( d.eTAU20M  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTAU1'  ).setLogic( d.jTAU1   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTAU20'  ).setLogic( d.jTAU20   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTAU30'  ).setLogic( d.jTAU30   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_jTAU30M' ).setLogic( d.jTAU30M  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_cTAU12M' ).setLogic( d.cTAU12M  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_cTAU20M' ).setLogic( d.cTAU20M  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU30'  ).setLogic( d.eTAU30   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_cTAU30M' ).setLogic( d.cTAU30M  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU35'  ).setLogic( d.eTAU35   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_cTAU35M' ).setLogic( d.cTAU35M  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_cTAU50M' ).setLogic( d.cTAU50M  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU40HM').setLogic( d.eTAU40HM & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU40HT').setLogic( d.eTAU40HT & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU60HM').setLogic( d.eTAU60HM & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU80HL').setLogic( d.eTAU80HL & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU60HL').setLogic( d.eTAU60HL & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU60'  ).setLogic( d.eTAU60   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU70'  ).setLogic( d.eTAU70   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU80'  ).setLogic( d.eTAU80   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU140' ).setLogic( d.eTAU140  & physcond).setTriggerType( TT.calo )

        MenuItem('L1_eTAU1_EMPTY').setLogic(d.eTAU1 & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU12_EMPTY').setLogic(d.eTAU12 & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU12_FIRSTEMPTY').setLogic(d.eTAU12 & firstempty).setTriggerType(TT.calo)
        MenuItem('L1_eTAU12_UNPAIRED_ISO').setLogic(d.eTAU12 & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU60_EMPTY').setLogic(d.eTAU60 & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU60_UNPAIRED_ISO').setLogic(d.eTAU60 & unpaired_isocond).setTriggerType(TT.calo)

        # Phase-I 2xTAU
        MenuItem('L1_cTAU30M_2cTAU20M').setLogic(d.cTAU30M & d.cTAU20M.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_cTAU35M_2cTAU30M').setLogic(d.cTAU35M & d.cTAU30M.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eTAU80_2eTAU60').setLogic(d.eTAU80 & d.eTAU60.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_cTAU30M_2cTAU20M_4jJ30p0ETA25').setLogic(d.cTAU30M & d.cTAU20M.x(2) & d.jJ300ETA25.x(4) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_cTAU35M_2cTAU30M_2jJ55_3jJ50').setLogic(d.cTAU35M & d.cTAU30M.x(2) & d.jJ55.x(2) & d.jJ50.x(3) & physcond).setTriggerType(TT.calo)
        # ATR-29439
        MenuItem('L1_cTAU30M_2cTAU20M_3jJ30p0ETA25').setLogic(d.cTAU30M & d.cTAU20M.x(2) & d.jJ300ETA25.x(3) & physcond).setTriggerType(TT.calo)
        
        #UPC TAU
        MenuItem('L1_2TAU1_VTE50' ).setLogic( d.HA1.x(2)      & Not(d.TE50) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU2_VTE50' ).setLogic( d.HA2.x(2)      & Not(d.TE50) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU3_VTE50' ).setLogic( d.HA3.x(2)      & Not(d.TE50) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU2_VTE100').setLogic( d.HA2.x(2)      & Not(d.TE100) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU3_VTE100').setLogic( d.HA3.x(2)      & Not(d.TE100) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU1_VTE200').setLogic( d.HA1.x(2)      & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU2_VTE200').setLogic( d.HA2.x(2)      & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU3_VTE200').setLogic( d.HA3.x(2)      & Not(d.TE200) & physcond).setTriggerType(TT.calo)

        MenuItem('L1_TAU1_VTE200'    ).setLogic( d.HA1  & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU1_TE3_VTE200').setLogic( d.HA1  & d.TE3    & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TAU1_TE4_VTE200').setLogic( d.HA1  & d.TE4    & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TAU2_TE3_VTE200').setLogic( d.HA2  & d.TE3    & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TAU2_TE4_VTE200').setLogic( d.HA2  & d.TE4    & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TAU1_TE5_VTE200').setLogic( d.HA1  & d.TE5    & Not(d.TE200) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TAU1_VTE200_EMPTY'   ).setLogic( d.HA1  & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_TAU1_TE3_VTE200_EMPTY'   ).setLogic( d.HA1 & d.TE3  & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_TAU2_TE3_VTE200_EMPTY'   ).setLogic( d.HA2 & d.TE3 & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_TAU1_TE4_VTE200_EMPTY'   ).setLogic( d.HA1 & d.TE4  & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_TAU2_TE4_VTE200_EMPTY'   ).setLogic( d.HA2 & d.TE4 & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )
        MenuItem('L1_2TAU1_VTE200_EMPTY').setLogic( d.HA1.x(2)      & Not(d.TE200) & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_2TAU1_VTE200_UNPAIRED_ISO').setLogic( d.HA1.x(2)      & Not(d.TE200) & unpaired_isocond ).setTriggerType(TT.calo)
        MenuItem('L1_2TAU1_VTE200_UNPAIRED_NONISO').setLogic( d.HA1.x(2)      & Not(d.TE200) & unpaired_nonisocond ).setTriggerType(TT.calo)
        MenuItem('L1_TAU8_VTE200'    ).setLogic( d.HA8  & Not(d.TE200) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU8_VTE200_EMPTY'    ).setLogic( d.HA8  & Not(d.TE200) & cosmiccond).setTriggerType( TT.calo )

        # 3xTAU
        MenuItem('L1_TAU20_2TAU12'  ).setLogic( d.HA20 & d.HA12.x(2)  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20_2TAU12I' ).setLogic( d.HA20 & d.HA12I.x(2) &  physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU40_2TAU20IM' ).setLogic( d.HA40 & d.HA20IM.x(2) &  physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2TAU12IM' ).setLogic( d.HA20IM & d.HA12IM.x(2) &  physcond).setTriggerType( TT.calo )

        # mixed tau
        MenuItem('L1_EM15VHI_2TAU12'               ).setLogic( d.EM15VHI & d.HA12.x(2)  	& physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12IL'             ).setLogic( d.EM15VHI & d.HA12IL.x(2)	& physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12IM'             ).setLogic( d.EM15VHI & d.HA12IM.x(2)	& physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_TAU40_2TAU15'         ).setLogic( d.EM15VHI & d.HA40 & d.HA15.x(2)   & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12I_J25_2J15_3J12').setLogic( d.EM15VHI & d.HA12I.x(2)  & d.J25 & d.J15.x(2) & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12IM_J25_3J12'    ).setLogic( d.EM15VHI & d.HA12IM.x(2)  & d.J25 & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12IL_J25_3J12'    ).setLogic( d.EM15VHI & d.HA12IL.x(2)  & d.J25 & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12_J25_3J12'      ).setLogic( d.EM15VHI & d.HA12.x(2)  & d.J25 & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12IM_4J12'        ).setLogic( d.EM15VHI & d.HA12IM.x(2) & d.J12.x(4) & physcond).setTriggerType( TT.calo )

        #Phase-I mixed tau
        MenuItem('L1_eEM18M_2eTAU20M'             ).setLogic( d.eEM18M & d.eTAU20M.x(2)	& physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_eTAU20M'                ).setLogic( d.MU8F   & d.eTAU20M	& physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_cTAU20M'                ).setLogic( d.MU8F   & d.cTAU20M	& physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_eTAU20M_jJ55_2jJ30'     ).setLogic( d.MU8F   & d.eTAU20M      & d.jJ55      & d.jJ30.x(2) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_cTAU20M_jJ55_2jJ30'     ).setLogic( d.MU8F   & d.cTAU20M      & d.jJ55      & d.jJ30.x(2) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_eTAU20M_3jJ30'          ).setLogic( d.MU8F   & d.eTAU20M	& d.jJ30.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_cTAU20M_3jJ30'          ).setLogic( d.MU8F   & d.eTAU20M	& d.jJ30.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18M_2eTAU20M_jJ55_3jJ30'  ).setLogic( d.eEM18M & d.eTAU20M.x(2) & d.jJ55      & d.jJ30.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_eTAU30M'                ).setLogic( d.MU8F   & d.eTAU30M      & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_cTAU30M'                ).setLogic( d.MU8F   & d.cTAU30M	& physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU30M_2eTAU20M_jJ55_2jJ50_3jJ30').setLogic( d.eTAU30M & d.eTAU20M.x(2) & d.jJ55 & d.jJ50.x(2) & d.jJ30.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_cTAU30M_2cTAU20M_jJ55_2jJ50_3jJ30').setLogic( d.cTAU30M & d.cTAU20M.x(2) & d.jJ55.x(1) & d.jJ50.x(2) & d.jJ30.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU35M_2eTAU30M'            ).setLogic( d.eTAU35M & d.eTAU30M.x(2) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU30M_2jJ50_jXE90'         ).setLogic( d.eTAU30M & d.jJ50.x(2)    & d.jXE90     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18M_2eTAU20M_jXE70'       ).setLogic( d.eEM18M  & d.eTAU20M.x(2) & d.jXE70     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18M_2cTAU20M_jXE70'       ).setLogic( d.eEM18M  & d.cTAU20M.x(2) & d.jXE70     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18M_2eTAU20M_4jJ30'       ).setLogic( d.eEM18M  & d.eTAU20M.x(2) & d.jJ30.x(4) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eEM18M_2cTAU20M_4jJ30'       ).setLogic( d.eEM18M  & d.cTAU20M.x(2) & d.jJ30.x(4) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_eTAU20M_jXE70'          ).setLogic( d.MU8F    & d.eTAU20M      & d.jXE70     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_cTAU20M_jXE70'          ).setLogic( d.MU8F    & d.cTAU20M      & d.jXE70     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU30M_2eTAU20M_jXE70'      ).setLogic( d.eTAU30M & d.eTAU20M.x(2) & d.jXE70     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_cTAU30M_2cTAU20M_jXE70'      ).setLogic( d.cTAU30M & d.cTAU20M.x(2) & d.jXE70     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU60_2eTAU20M_jXE80'       ).setLogic( d.eTAU60  & d.eTAU20M.x(2) & d.jXE80     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_eTAU60_2cTAU20M_jXE80'       ).setLogic( d.eTAU60  & d.cTAU20M.x(2) & d.jXE80     & physcond).setTriggerType( TT.calo )
        
        MenuItem('L1_MU8F_TAU12'       ).setLogic( d.MU8F  & d.HA12          & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12IM'     ).setLogic( d.MU8F  & d.HA12IM        & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU20'       ).setLogic( d.MU8F  & d.HA20          & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU20IM'     ).setLogic( d.MU8F  & d.HA20IM        & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8VF_TAU20IM'    ).setLogic( d.MU8VF & d.HA20IM        & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU14FCH_TAU12IM'      ).setLogic( d.MU14FCH & d.HA12IM & physcond).setTriggerType( TT.calo)
        MenuItem('L1_MU8F_TAU12_J25_2J12'  ).setLogic( d.MU8F & d.HA12 & d.J25 & d.J12.x(2)     & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12IM_J25_2J12').setLogic( d.MU8F & d.HA12IM & d.J25 & d.J12.x(2)    & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12IM_3J12'    ).setLogic( d.MU8F & d.HA12IM & d.J12.x(3)    & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU25IM_2TAU20IM_2J25_3J20').setLogic( d.HA25IM & d.HA20IM.x(2)     & d.J25.x(2)  & d.J20.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20_2TAU12I_J25_2J15_3J12'   ).setLogic( d.HA20 & d.HA12I.x(2)     & d.J25 & d.J15.x(2) & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20_2TAU12_J25_2J20_3J12'    ).setLogic( d.HA20 & d.HA12.x(2)     & d.J25 & d.J20.x(2) & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2TAU12IM_J25_2J20_3J12').setLogic( d.HA20IM & d.HA12IM.x(2)     & d.J25 & d.J20.x(2) & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IL_2TAU12IL_J25_2J20_3J12').setLogic( d.HA20IL & d.HA12IL.x(2)     & d.J25 & d.J20.x(2) & d.J12.x(3) & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TAU25IT_2TAU12IT_2J25_3J12').setLogic( d.HA25IT & d.HA12IT.x(2)     & d.J25.x(2)  & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU25IM_2TAU12IM_J25_3J12' ).setLogic( d.HA25IM & d.HA12IM.x(2) & d.J25 & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU25IM_2TAU20IM').setLogic( d.HA25IM & d.HA20IM.x(2)  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2TAU12IM_4J12'     ).setLogic( d.HA20IM & d.HA12IM.x(2)  & d.J12.x(4) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2TAU12IM_4J12p0ETA25').setLogic( d.HA20IM & d.HA12IM.x(2)  & d.J120ETA25.x(4) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2TAU12IM_4J12p0ETA28').setLogic( d.HA20IM & d.HA12IM.x(2)  & d.J120ETA28.x(4) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU60_2TAU40'                ).setLogic( d.HA60 & d.HA40.x(2)  & physcond).setTriggerType( TT.calo )
        MenuItem('L1_2TAU12I_TAU20_J25_2J15_3J12' ).setLogic( d.HA12I.x(2)   & d.HA20  & d.J25 & d.J15.x(2) & d.J12.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_2TAU20IM_3J20'  ).setLogic(  d.HA20IM.x(2) & d.J20.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_2TAU20IM_J25_3J20'  ).setLogic(  d.HA20IM.x(2) & d.J25 & d.J20.x(3) & physcond).setTriggerType( TT.calo )

        MenuItem('L1_TAU20_2J20_XE45'              ).setLogic( d.HA20    & d.J20.x(2)   & d.XE45 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2J20_XE45'            ).setLogic( d.HA20IM    & d.J20.x(2)   & d.XE45 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU25_2J20_XE45'              ).setLogic( d.HA25      & d.J20.x(2)   & d.XE45 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20_2TAU12_XE35'            ).setLogic( d.HA20    & d.HA12.x(2)  & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2TAU12IM_XE35'        ).setLogic( d.HA20IM  & d.HA12IM.x(2)  & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IL_2TAU12IL_XE35'        ).setLogic( d.HA20IL  & d.HA12IL.x(2)  & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IT_2TAU12IT_XE35'        ).setLogic( d.HA20IT  & d.HA12IT.x(2)  & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20IM_2TAU12IM_XE40'        ).setLogic( d.HA20IM  & d.HA12IM.x(2)  & d.XE40 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU40_2TAU12IM_XE40'          ).setLogic( d.HA40  & d.HA12IM.x(2)  & d.XE40 & physcond).setTriggerType( TT.calo )

        MenuItem('L1_MU8F_TAU12I_XE35'             ).setLogic( d.MU8F    & d.HA12I      & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12_XE35'              ).setLogic( d.MU8F    & d.HA12       & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12IL_XE35'            ).setLogic( d.MU8F    & d.HA12IL     & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12IM_XE35'            ).setLogic( d.MU8F    & d.HA12IM     & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12IT_XE35'            ).setLogic( d.MU8F    & d.HA12IT     & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_MU8F_TAU12IM_XE40'            ).setLogic( d.MU8F    & d.HA12IM     & d.XE40 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_TAU20I_2TAU12I_XE35'          ).setLogic( d.HA20I   & d.HA12I.x(2) & d.XE35 & physcond).setTriggerType( TT.calo )

        MenuItem('L1_EM15VHI_TAU20IM_2TAU15_J25_2J20_3J15').setLogic( d.EM15VHI  &  d.HA20IM  &  d.HA15.x(2) &  d.J25  & d.J20.x(2) & d.J15.x(3) & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12I_XE35'          ).setLogic( d.EM15VHI  & d.HA12I.x(2) & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12IM_XE35'         ).setLogic( d.EM15VHI  & d.HA12IM.x(2) & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12IL_XE35'         ).setLogic( d.EM15VHI  & d.HA12IL.x(2) & d.XE35 & physcond).setTriggerType( TT.calo )
        MenuItem('L1_EM15VHI_2TAU12_XE35'           ).setLogic( d.EM15VHI  & d.HA12.x(2) & d.XE35 & physcond).setTriggerType( TT.calo )

        # JET ctpid=[0x60:0x7f]
        MenuItem('L1_J5'   ).setLogic( d.J5   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J10'  ).setLogic( d.J10  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J12'  ).setLogic( d.J12  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J15'  ).setLogic( d.J15  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J20'  ).setLogic( d.J20  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J25'  ).setLogic( d.J25  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J30'  ).setLogic( d.J30  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J35'  ).setLogic( d.J35  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J40'  ).setLogic( d.J40  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J50'  ).setLogic( d.J50  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J60'  ).setLogic( d.J60  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J75'  ).setLogic( d.J75  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J75A' ).setLogic( d.J75A & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J75C' ).setLogic( d.J75C & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J85'  ).setLogic( d.J85  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J100' ).setLogic( d.J100 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J120' ).setLogic( d.J120 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J150' ).setLogic( d.J150 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J175' ).setLogic( d.J175 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J250' ).setLogic( d.J250 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J400' ).setLogic( d.J400 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J400_LAR' ).setLogic( d.J400 & physcond).setTriggerType(TT.lardigital) # ATR-22344

        MenuItem('L1_jJ5'           ).setLogic( d.jJ5         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ5p30ETA49'   ).setLogic( d.jJ530ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ10'          ).setLogic( d.jJ10         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ10p30ETA49'  ).setLogic( d.jJ1030ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ20'          ).setLogic( d.jJ20         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30'                    ).setLogic( d.jJ30  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30_EMPTY'              ).setLogic( d.jJ30  & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30_FIRSTEMPTY'         ).setLogic( d.jJ30  & firstempty).setTriggerType(TT.calo)
        MenuItem('L1_jJ30_UNPAIRED_ISO'       ).setLogic( d.jJ30  & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30_UNPAIRED_NONISO'    ).setLogic( d.jJ30  & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30_BGRP12'             ).setLogic( d.jJ30  & bgrp12cond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30_UNPAIREDB1'         ).setLogic( d.jJ30  & bgrp13cond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30_UNPAIREDB2'         ).setLogic( d.jJ30  & bgrp14cond).setTriggerType(TT.calo)
        MenuItem('L1_jJ30p0ETA25'   ).setLogic( d.jJ300ETA25   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ40'          ).setLogic( d.jJ40         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ40p0ETA25'   ).setLogic( d.jJ400ETA25   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ40p30ETA49'  ).setLogic( d.jJ4030ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ40p30ETA49_UNPAIRED_ISO').setLogic( d.jJ4030ETA49  & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_jJ50'          ).setLogic( d.jJ50         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ50p0ETA25'   ).setLogic( d.jJ500ETA25   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ50p30ETA49'  ).setLogic( d.jJ5030ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ55'          ).setLogic( d.jJ55         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ55p0ETA23'   ).setLogic( d.jJ550ETA23   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ60'          ).setLogic( d.jJ60         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ60p30ETA49'  ).setLogic( d.jJ6030ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ60p30ETA49_EMPTY'            ).setLogic( d.jJ6030ETA49  & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_jJ60p30ETA49_UNPAIRED_ISO'     ).setLogic( d.jJ6030ETA49  & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_jJ60p30ETA49_UNPAIRED_NONISO'  ).setLogic( d.jJ6030ETA49  & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_jJ70p0ETA23'   ).setLogic( d.jJ700ETA23   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ80'          ).setLogic( d.jJ80         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ80p0ETA25'   ).setLogic( d.jJ800ETA25   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ85p0ETA21'   ).setLogic( d.jJ850ETA21   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ90'          ).setLogic( d.jJ90         & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ90_UNPAIRED_ISO'       ).setLogic( d.jJ90 & unpaired_isocond).setTriggerType(TT.calo)
        MenuItem('L1_jJ90_UNPAIRED_NONISO'    ).setLogic( d.jJ90 & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_jJ90_2jJ80p0ETA25_3jJ40p0ETA25').setLogic( d.jJ90 & d.jJ800ETA25.x(2) & d.jJ400ETA25.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ90p30ETA49'  ).setLogic( d.jJ9030ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ125'         ).setLogic( d.jJ125        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ125p30ETA49' ).setLogic( d.jJ12530ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ140'         ).setLogic( d.jJ140        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ160'         ).setLogic( d.jJ160        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ160_FIRSTEMPTY').setLogic( d.jJ160      & firstempty).setTriggerType(TT.calo)
        MenuItem('L1_jJ180'         ).setLogic( d.jJ180        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ500'         ).setLogic( d.jJ500        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ500_LAR'     ).setLogic( d.jJ500        & physcond).setTriggerType(TT.lardigital) # ATR-22344

        MenuItem('L1_4jJ40'         ).setLogic( d.jJ40.x(4)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ55p0ETA23'  ).setLogic( d.jJ550ETA23.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4jJ40p0ETA25'  ).setLogic( d.jJ400ETA25.x(4) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_6jJ40'         ).setLogic( d.jJ40.x(6)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_5jJ40p0ETA25'  ).setLogic( d.jJ400ETA25.x(5) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4jJ50'         ).setLogic( d.jJ50.x(4)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ90'         ).setLogic( d.jJ90.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ70p0ETA23'  ).setLogic( d.jJ700ETA23.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ140_3jJ60'   ).setLogic( d.jJ140 & d.jJ60.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ80p0ETA25_2jJ55_jJ50p30ETA49' ).setLogic( d.jJ800ETA25 & d.jJ55.x(2) & d.jJ5030ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ55p0ETA23_2jJ40p30ETA49'      ).setLogic( d.jJ550ETA23 & d.jJ4030ETA49.x(2)            & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ80p0ETA25_2jJ40p30ETA49'      ).setLogic( d.jJ800ETA25 & d.jJ4030ETA49.x(2)            & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ85p0ETA21_3jJ40p0ETA25'       ).setLogic( d.jJ850ETA21 & d.jJ400ETA25.x(3)             & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3jJ40p0ETA25'  ).setLogic( d.jJ400ETA25.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2jJ56p0ETA49'  ).setLogic( d.jJ560ETA49.x(2) & physcond).setTriggerType(TT.calo) # prospective Run 4 L1 item, ATR-30180

        MenuItem('L1_jJ60_EMPTY'     ).setLogic( d.jJ60 & cosmiccond ).setTriggerType(TT.calo)
        MenuItem('L1_jJ60_FIRSTEMPTY').setLogic( d.jJ60 & firstempty ).setTriggerType(TT.calo)

        MenuItem('L1_MU3V_jJ20'      ).setLogic( d.MU3V & d.jJ20    & physcond).setTriggerType(TT.calo) # added temporarily 
        MenuItem('L1_MU3V_jJ30'      ).setLogic( d.MU3V & d.jJ30    & physcond).setTriggerType(TT.calo) # added temporarily 
        MenuItem('L1_MU3V_jJ40'      ).setLogic( d.MU3V & d.jJ40    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_jJ50'      ).setLogic( d.MU3V & d.jJ50    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_jJ60'      ).setLogic( d.MU3V & d.jJ60    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU5VF_jJ80'      ).setLogic( d.MU5VF & d.jJ80  & physcond).setTriggerType(TT.calo)


        MenuItem('L1_gJ20p0ETA25'         ).setLogic( d.gJ200ETA25        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gJ20p25ETA49'         ).setLogic( d.gJ2025ETA49        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gJ20p0ETA25_EMPTY'         ).setLogic( d.gJ200ETA25        & cosmiccond).setTriggerType(TT.calo)
        MenuItem('L1_gJ50p0ETA25'         ).setLogic( d.gJ500ETA25        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gJ100p0ETA25'        ).setLogic( d.gJ1000ETA25       & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gJ400p0ETA25'        ).setLogic( d.gJ4000ETA25       & physcond).setTriggerType(TT.calo)

        MenuItem('L1_gLJ80p0ETA25'         ).setLogic( d.gLJ800ETA25        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gLJ90p0ETA25'         ).setLogic( d.gLJ900ETA25        & physcond).setTriggerType(TT.calo) # prospective Run 4 L1 item, ATR-30180
        MenuItem('L1_gLJ100p0ETA25'        ).setLogic( d.gLJ1000ETA25       & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gLJ140p0ETA25'        ).setLogic( d.gLJ1400ETA25       & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gLJ160p0ETA25'        ).setLogic( d.gLJ1600ETA25       & physcond).setTriggerType(TT.calo)

        MenuItem('L1_jEM25'         ).setLogic( d.jEM25        & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jEM20M'        ).setLogic( d.jEM20M       & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jEM35'         ).setLogic( d.jEM35        & physcond).setTriggerType(TT.calo) # prospective Run 4 L1 item, ATR-30180

        MenuItem('L1_J10p31ETA49').setLogic( d.J1031ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J75p31ETA49').setLogic( d.J7531ETA49 & physcond).setTriggerType(TT.calo)


        MenuItem('L1_J10_UNPAIRED_ISO'   ).setLogic( d.J10 & unpaired_isocond   ).setTriggerType(TT.calo)
        MenuItem('L1_J10_UNPAIRED_NONISO').setLogic( d.J10 & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_J10_EMPTY'          ).setLogic( d.J10 & cosmiccond ).setTriggerType(TT.calo)
        MenuItem('L1_J10_FIRSTEMPTY'     ).setLogic( d.J10 & firstempty ).setTriggerType(TT.calo)

        MenuItem('L1_J12_UNPAIRED_ISO'    ).setLogic( d.J12 & unpaired_isocond   ).setTriggerType(TT.calo)
        MenuItem('L1_J12_UNPAIRED_NONISO' ).setLogic( d.J12 & unpaired_nonisocond).setTriggerType(TT.calo)
        MenuItem('L1_J12_EMPTY'           ).setLogic( d.J12 & cosmiccond ).setTriggerType(TT.calo)
        MenuItem('L1_J12_FIRSTEMPTY'      ).setLogic( d.J12 & firstempty ).setTriggerType(TT.calo)
        MenuItem('L1_J12_UNPAIREDB1'      ).setLogic( d.J12 & bgrp13cond  ).setTriggerType(TT.calo)
        MenuItem('L1_J12_UNPAIREDB2'      ).setLogic( d.J12 & bgrp14cond).setTriggerType(TT.calo)

        MenuItem('L1_J50_UNPAIRED_ISO'    ).setLogic( d.J50 & unpaired_isocond   ).setTriggerType(TT.calo)
        MenuItem('L1_J50_UNPAIRED_NONISO' ).setLogic( d.J50 & unpaired_nonisocond).setTriggerType(TT.calo)

        MenuItem('L1_J12_BGRP12'        ).setLogic( d.J12 & bgrp12cond ).setTriggerType(TT.calo)
        MenuItem('L1_J30p31ETA49_BGRP12').setLogic( d.J3031ETA49 & bgrp12cond ).setTriggerType(TT.calo)

        MenuItem('L1_J30_EMPTY'     ).setLogic( d.J30 & cosmiccond ).setTriggerType(TT.calo)
        MenuItem('L1_J30_FIRSTEMPTY').setLogic( d.J30 & firstempty ).setTriggerType(TT.calo)

        MenuItem('L1_J10p31ETA49_EMPTY').setLogic( d.J1031ETA49 & cosmiccond ).setTriggerType(TT.calo)
        MenuItem('L1_J15p31ETA49_UNPAIRED_ISO').setLogic( d.J1531ETA49 & unpaired_isocond).setTriggerType(TT.calo)

        MenuItem('L1_J30p31ETA49_EMPTY'          ).setLogic( d.J3031ETA49 & cosmiccond ).setTriggerType(TT.calo)
        MenuItem('L1_J30p31ETA49_FIRSTEMPTY'     ).setLogic( d.J3031ETA49 & firstempty ).setTriggerType(TT.calo)
        MenuItem('L1_J30p31ETA49_UNPAIRED_ISO'   ).setLogic( d.J3031ETA49 & unpaired_isocond   ).setTriggerType(TT.calo)
        MenuItem('L1_J30p31ETA49_UNPAIRED_NONISO').setLogic( d.J3031ETA49 & unpaired_nonisocond   ).setTriggerType(TT.calo)


        MenuItem('L1_J100_FIRSTEMPTY').setLogic( d.J100 & firstempty ).setTriggerType(TT.calo)

        # multi jet
        MenuItem('L1_2J25p31ETA49'  ).setLogic( d.J2531ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2J15'   ).setLogic( d.J15.x(2)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J15_J15p31ETA49' ).setLogic( d.J15 & d.J1531ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J20_J20p31ETA49' ).setLogic( d.J20 & d.J2031ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J10'   ).setLogic( d.J10.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J15'   ).setLogic( d.J15.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J20'   ).setLogic( d.J20.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J40'   ).setLogic( d.J40.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J50'   ).setLogic( d.J50.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J75'   ).setLogic( d.J75.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4J10'   ).setLogic( d.J10.x(4)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4J15'   ).setLogic( d.J15.x(4)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4J20'   ).setLogic( d.J20.x(4)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4J20p0ETA49').setLogic( d.J200ETA49.x(4) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4J30'   ).setLogic( d.J30.x(4)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_6J15'   ).setLogic( d.J15.x(6)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J75_3J20' ).setLogic( d.J75 & d.J20.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J85_3J30' ).setLogic( d.J85 & d.J30.x(3)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J30p0ETA49_2J20p0ETA49'      ).setLogic( d.J300ETA49 & d.J200ETA49.x(2)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J15p0ETA25_2J15p31ETA49'     ).setLogic( d.J150ETA25 & d.J1531ETA49.x(2)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J40p0ETA25_2J15p31ETA49'     ).setLogic( d.J400ETA25 & d.J1531ETA49.x(2)    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J40p0ETA25_2J25_J20p31ETA49' ).setLogic( d.J400ETA25 & d.J25.x(2) & d.J2031ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J40p0ETA25_2J30_J20p31ETA49' ).setLogic( d.J400ETA25 & d.J30.x(2) & d.J2031ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J45p0ETA21_3J15p0ETA25'      ).setLogic( d.J450ETA21 & d.J150ETA25.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J50_2J40p0ETA25_3J15p0ETA25' ).setLogic( d.J50 & d.J400ETA25.x(2) & d.J150ETA25.x(3) & physcond).setTriggerType(TT.calo)

        MenuItem('L1_4J17p0ETA22' ).setLogic( d.J170ETA22.x(4) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J25p0ETA23' ).setLogic( d.J250ETA23.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J35p0ETA23' ).setLogic( d.J350ETA23.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J15p0ETA25' ).setLogic( d.J150ETA25.x(3) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_4J15p0ETA25' ).setLogic( d.J150ETA25.x(4) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_5J15p0ETA25' ).setLogic( d.J150ETA25.x(5) & physcond).setTriggerType(TT.calo)

        # Legacy ZeroBias
        if ('Physics_HI_run3_v' in menuName or 'MC_HI_run3_v' in menuName):
            MenuItem('L1_ZB', ctpid=240).setLogic(d.ZB_J75  & physcond).setTriggerType(TT.zerobs)
        else:
            MenuItem('L1_ZB', ctpid=240).setLogic(d.ZB_EM15 & physcond).setTriggerType(TT.zerobs)

        # Phase-I ZeroBias trigger for commissioning
        # TODO: When established, replace legacy logic above with corresponding
        # Phase-I seeds
        # Unlike legacy ZeroBias, the delay logic is in the CTP firmware, so
        # we provide the seed rather than a dedicated threshold
        

        # LAr Saturation
        MenuItem('L1_LArSaturation').setLogic( d.LArSaturation & physcond ).setTriggerType(TT.calo)

        if ('HI_run3_v' in menuName):
            MenuItem('L1_ZeroBias').setLogic( d.ZeroBiasB & physcond ).setTriggerType(TT.zerobs)
        else:
            MenuItem('L1_ZeroBias').setLogic( d.ZeroBiasA & physcond ).setTriggerType(TT.zerobs)

        # combined jet - xe
        MenuItem('L1_J40_XE50').setLogic( d.J40 & d.XE50 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jJ80_jXE100').setLogic( d.jJ80 & d.jXE100 & physcond).setTriggerType(TT.calo)        
        MenuItem('L1_J40p0ETA25_XE50').setLogic( d.J400ETA25 & d.XE50 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J75_XE40' ).setLogic( d.J75 & d.XE40 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J75_XE50' ).setLogic( d.J75 & d.XE50 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2J15_XE55').setLogic( d.J15.x(2) & d.XE55 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2J40_XE45').setLogic( d.J40.x(2) & d.XE45 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2J50_XE40').setLogic( d.J50.x(2) & d.XE40 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J40_XE60' ).setLogic( d.J40 & d.XE60 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_J30p0ETA49_XE50').setLogic( d.J300ETA49 & d.XE50 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_3J15p0ETA25_XE40').setLogic( d.J150ETA25.x(3) & d.XE40 & physcond).setTriggerType(TT.calo)
        # ATR-27250 Duplicate multijet-seeded triggers to jFEX
        MenuItem('L1_3jJ40p0ETA25_jXE80').setLogic( d.jJ400ETA25.x(3) & d.jXE80 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2jJ90_jXE80').setLogic( d.jJ90.x(2) & d.jXE80 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2jJ40_jXE110').setLogic( d.jJ40.x(2) & d.jXE110 & physcond).setTriggerType(TT.calo)

        # combined em - jet
        MenuItem('L1_EM18VHI_3J20' ).setLogic( d.EM18VHI  & d.J20.x(3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_EM20VH_3J20' ).setLogic( d.EM20VH  & d.J20.x(3)  & physcond).setTriggerType(TT.calo)
        # combined em - jet Phase-1 ATR-28761
        MenuItem('L1_eEM22M_3jJ50' ).setLogic( d.eEM22M  & d.jJ50.x(3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM24L_3jJ50' ).setLogic( d.eEM24L  & d.jJ50.x(3)  & physcond).setTriggerType(TT.calo)

        ### ATR-28443 test eEMX{} + {{3,4jJY{}}} L1 seeds
        MenuItem('L1_eEM22M_3jJ40p0ETA25' ).setLogic( d.eEM22M & d.jJ400ETA25.x(3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_eEM22M_4jJ30p0ETA25' ).setLogic( d.eEM22M & d.jJ300ETA25.x(4)  & physcond).setTriggerType(TT.calo)

        # combined mu - jet
        MenuItem('L1_MU3V_J12'   ).setLogic( d.MU3V & d.J12    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_J15'   ).setLogic( d.MU3V & d.J15    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU5VF_J20'  ).setLogic( d.MU5VF & d.J20    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU5VF_J30p0ETA49_2J20p0ETA49').setLogic( d.MU5VF & d.J300ETA49 & d.J200ETA49.x(2) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU5VF_J40'  ).setLogic( d.MU5VF & d.J40    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU5VF_J75'  ).setLogic( d.MU5VF & d.J75    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_3J15'  ).setLogic( d.MU3V & d.J15.x(3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_3J20'  ).setLogic( d.MU3V & d.J20.x(3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_J20'   ).setLogic( d.MU3V & d.J20    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_J30'   ).setLogic( d.MU3V & d.J30    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_J50'   ).setLogic( d.MU3V & d.J50    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU8F_3J20'    ).setLogic( d.MU8F & d.J20.x(3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU8F_2J20'    ).setLogic( d.MU8F & d.J20.x(2)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU8F_2J15_J20').setLogic( d.MU8F & d.J15.x(2) & d.J20  & physcond).setTriggerType(TT.calo)

        MenuItem('L1_MU8F_3jJ50'    ).setLogic( d.MU8F & d.jJ50.x(3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU8F_2jJ50'    ).setLogic( d.MU8F & d.jJ50.x(2)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU8F_2jJ40_jJ50').setLogic( d.MU8F & d.jJ40.x(2) & d.jJ50  & physcond).setTriggerType(TT.calo)
        
        MenuItem('L1_MU14FCH_J50'  ).setLogic( d.MU14FCH & d.J50  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_XE60'      ).setLogic( d.MU3V & d.XE60  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2MU3V_XE60'     ).setLogic( d.MU3V.x(2) & d.XE60  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2MU3V_J40_XE50' ).setLogic( d.MU3V.x(2) & d.J40 & d.XE50  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU3V_J50_XE40'  ).setLogic( d.MU3V & d.J50 & d.XE40  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2MU3V_J40_XE20' ).setLogic( d.MU3V.x(2) & d.J40 & d.XE20  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU14FCH_J40' ).setLogic( d.MU14FCH & d.J40 & physcond).setTriggerType(TT.calo) ## ATR-14377
        MenuItem('L1_MU14FCH_XE30').setLogic( d.MU14FCH & d.XE30 & physcond).setTriggerType(TT.calo) ## ATR-14377
        MenuItem('L1_MU14FCH_XE40').setLogic( d.MU14FCH & d.XE40 & physcond).setTriggerType(TT.calo) ## ATR-19376
        MenuItem('L1_MU14FCH_jJ80' ).setLogic( d.MU14FCH & d.jJ80 & physcond).setTriggerType(TT.calo) 
        MenuItem('L1_MU14FCH_jXE70').setLogic( d.MU14FCH & d.jXE70 & physcond).setTriggerType(TT.calo) 
        MenuItem('L1_MU14FCH_jJ90' ).setLogic( d.MU14FCH & d.jJ90 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_MU14FCH_jXE80').setLogic( d.MU14FCH & d.jXE80 & physcond).setTriggerType(TT.calo)        
	# ATR-31037 

       #ATR-30618
        MenuItem('L1_ADVAET').setLogic( d.TOPO_ADVAE2A_6jJ0s_4eTAU0s_4MU0s_jXE0s_Tight & physcond )
        MenuItem('L1_ADVAEL').setLogic( d.TOPO_ADVAE2A_6jJ0s_4eTAU0s_4MU0s_jXE0s_Loose & physcond )
        # HI
        MenuItem('L1_J15_NZ' ).setLogic( d.J15      & Not(ZDC_AND) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2J15_NZ').setLogic( d.J15.x(2) & Not(ZDC_AND) & physcond).setTriggerType(TT.calo)

        MenuItem('L1_J15_NL' ).setLogic( d.J15      & Not(d.LUCID_A) & Not(d.LUCID_C) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_2J15_NL').setLogic( d.J15.x(2) & Not(d.LUCID_A) & Not(d.LUCID_C) & physcond).setTriggerType(TT.calo)

        # XE
        MenuItem('L1_XE35').setLogic( d.XE35 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE40').setLogic( d.XE40 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE45').setLogic( d.XE45 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE50').setLogic( d.XE50 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE55').setLogic( d.XE55 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE60').setLogic( d.XE60 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE70').setLogic( d.XE70 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE80').setLogic( d.XE80 & physcond).setTriggerType(TT.calo)
        # phase1
        MenuItem('L1_gXENC70'   ).setLogic( d.gXENC70    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXENC100'  ).setLogic( d.gXENC100   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXEJWOJ60' ).setLogic( d.gXEJWOJ60  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXEJWOJ70' ).setLogic( d.gXEJWOJ70  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXEJWOJ80' ).setLogic( d.gXEJWOJ80  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXEJWOJ100').setLogic( d.gXEJWOJ100 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXEJWOJ110').setLogic( d.gXEJWOJ110 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXEJWOJ120').setLogic( d.gXEJWOJ120 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gXEJWOJ500').setLogic( d.gXEJWOJ500 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gMHT500').setLogic( d.gMHT500 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE60' ).setLogic( d.jXE60  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE70' ).setLogic( d.jXE70  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE80' ).setLogic( d.jXE80  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE90' ).setLogic( d.jXE90  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE100').setLogic( d.jXE100 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE110').setLogic( d.jXE110 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE120').setLogic( d.jXE120 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXE500').setLogic( d.jXE500 & physcond).setTriggerType(TT.calo)

        MenuItem('L1_jXEC100'    ).setLogic( d.jXEC100 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jXEPerf100' ).setLogic( d.jXEPerf100 & physcond).setTriggerType(TT.calo)

        #ATR-28670- Combined jet - xe
        MenuItem('L1_jJ80_jXE120' ).setLogic( d.jJ80 & d.jXE120 & physcond).setTriggerType(TT.calo)

        # phase1 TE
        MenuItem('L1_gTE3'     ).setLogic( d.gTE3 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gTE5'     ).setLogic( d.gTE5 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gTE10'     ).setLogic( d.gTE10 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_gTE200'     ).setLogic( d.gTE200 & physcond).setTriggerType(TT.calo)

        MenuItem('L1_jTE200'     ).setLogic( d.jTE200 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTEC200'    ).setLogic( d.jTEC200 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTEFWD100'  ).setLogic( d.jTEFWD100 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTEFWDA100' ).setLogic( d.jTEFWDA100 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTEFWDC100' ).setLogic( d.jTEFWDC100 & physcond).setTriggerType(TT.calo)
        #ATR-31097
        MenuItem('L1_TeAsymmetry-jTENoSort' ).setLogic( d.TOPO_TeAsymmetry_jTENoSort_ParamSet0 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TeATIME-jTENoSort' ).setLogic( d.TOPO_TeATIME_jTENoSort_ParamSet0 & physcond).setTriggerType(TT.calo)

        # additional jTE items for 2023 heavy ion runs
        MenuItem('L1_jTE3'     ).setLogic( d.jTE3  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE4'     ).setLogic( d.jTE4  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE5'     ).setLogic( d.jTE5  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE10'     ).setLogic( d.jTE10  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE20'     ).setLogic( d.jTE20  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE50'     ).setLogic( d.jTE50  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE100'     ).setLogic( d.jTE100  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE600'     ).setLogic( d.jTE600  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE1500'    ).setLogic( d.jTE1500 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE6500'    ).setLogic( d.jTE6500 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE8300'    ).setLogic( d.jTE8300 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE9000'    ).setLogic( d.jTE9000 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE10000'   ).setLogic( d.jTE10000 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE12000'   ).setLogic( d.jTE12000 & physcond).setTriggerType(TT.calo)
        # additional VjTE items for 2023 heavy ion runs
        MenuItem('L1_VjTE10'    ).setLogic( Not(d.jTE10)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VjTE50'    ).setLogic( Not(d.jTE50)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VjTE200'    ).setLogic( Not(d.jTE200)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VjTE600'    ).setLogic( Not(d.jTE600)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE50_VjTE600'    ).setLogic(  d.jTE50  & Not(d.jTE600)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_jTE50_VjTE200').setLogic(  d.jTE50  & Not(d.jTE200)  & physcond).setTriggerType(TT.calo)

        MenuItem('L1_XE10').setLogic( d.XE10 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE20').setLogic( d.XE20 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE25').setLogic( d.XE25 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE30').setLogic( d.XE30 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE65').setLogic( d.XE65 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE75').setLogic( d.XE75 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE150').setLogic( d.XE150 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE300').setLogic( d.XE300 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XE35_BGRP7').setLogic( d.XE35 & bgrp7cond).setTriggerType(TT.calo)
        MenuItem('L1_XE40_BGRP7').setLogic( d.XE40 & bgrp7cond).setTriggerType(TT.calo)
        MenuItem('L1_XE45_BGRP7').setLogic( d.XE45 & bgrp7cond).setTriggerType(TT.calo)
        MenuItem('L1_XE50_BGRP7').setLogic( d.XE50 & bgrp7cond).setTriggerType(TT.calo)
        MenuItem('L1_XE55_BGRP7').setLogic( d.XE55 & bgrp7cond).setTriggerType(TT.calo)
        MenuItem('L1_XE60_BGRP7').setLogic( d.XE60 & bgrp7cond).setTriggerType(TT.calo)
        MenuItem('L1_XE70_BGRP7').setLogic( d.XE70 & bgrp7cond).setTriggerType(TT.calo)
        MenuItem('L1_XE80_BGRP7').setLogic( d.XE80 & bgrp7cond).setTriggerType(TT.calo)
        # XS
        MenuItem('L1_XS20').setLogic( d.XS20.x(1) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XS30').setLogic( d.XS30.x(1) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XS40').setLogic( d.XS40.x(1) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XS45').setLogic( d.XS45.x(1) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XS50').setLogic( d.XS50.x(1) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XS55').setLogic( d.XS55.x(1) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XS60').setLogic( d.XS60.x(1) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_XS65').setLogic( d.XS65.x(1) & physcond).setTriggerType(TT.calo)

        # TE
        MenuItem('L1_TE0' ).setLogic( d.TE0  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE2' ).setLogic( d.TE2  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE3' ).setLogic( d.TE3  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE4' ).setLogic( d.TE4  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE5' ).setLogic( d.TE5  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE10').setLogic( d.TE10 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE15').setLogic( d.TE15 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE20').setLogic( d.TE20 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE25').setLogic( d.TE25 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE30').setLogic( d.TE30 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE40').setLogic( d.TE40 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE45').setLogic( d.TE45 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE50').setLogic( d.TE50 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE55').setLogic( d.TE55 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE60').setLogic( d.TE60 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE65').setLogic( d.TE65 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE70').setLogic( d.TE70 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE90').setLogic( d.TE90 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE100').setLogic( d.TE100 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE120').setLogic( d.TE120 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE140').setLogic( d.TE140 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE160').setLogic( d.TE160 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE200').setLogic( d.TE200 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE280').setLogic( d.TE280 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE300').setLogic( d.TE300 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE360').setLogic( d.TE360 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE2000').setLogic( d.TE2000 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE4000').setLogic( d.TE4000 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE10000').setLogic( d.TE10000 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE12000').setLogic( d.TE12000 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE14000').setLogic( d.TE14000 & physcond).setTriggerType(TT.calo)

        MenuItem('L1_TE0p24ETA49'   ).setLogic( d.TE024ETA49    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE3p24ETA49'   ).setLogic( d.TE324ETA49    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE5p24ETA49'   ).setLogic( d.TE524ETA49    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE10p24ETA49'  ).setLogic( d.TE1024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE15p24ETA49'  ).setLogic( d.TE1524ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE20p24ETA49'  ).setLogic( d.TE2024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE25p24ETA49'  ).setLogic( d.TE2524ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE30p24ETA49'  ).setLogic( d.TE3024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE35p24ETA49'  ).setLogic( d.TE3524ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE40p24ETA49'  ).setLogic( d.TE4024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE45p24ETA49'  ).setLogic( d.TE4524ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE50p24ETA49'  ).setLogic( d.TE5024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE55p24ETA49'  ).setLogic( d.TE5524ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE60p24ETA49'  ).setLogic( d.TE6024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE65p24ETA49'  ).setLogic( d.TE6524ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE70p24ETA49'  ).setLogic( d.TE7024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE80p24ETA49'  ).setLogic( d.TE8024ETA49   & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE110p24ETA49' ).setLogic( d.TE11024ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE150p24ETA49' ).setLogic( d.TE15024ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE180p24ETA49' ).setLogic( d.TE18024ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE2000p24ETA49').setLogic( d.TE200024ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE5000p24ETA49').setLogic( d.TE200024ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE6500p24ETA49').setLogic( d.TE200024ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE8000p24ETA49').setLogic( d.TE200024ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE9000p24ETA49').setLogic( d.TE200024ETA49 & physcond).setTriggerType(TT.calo)

        # HI items
        MenuItem('L1_TE3p0ETA49'   ).setLogic( d.TE30ETA49    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE7p0ETA49'   ).setLogic( d.TE70ETA49    & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE500p0ETA49' ).setLogic( d.TE5000ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE600p0ETA49' ).setLogic( d.TE6000ETA49  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE1500p0ETA49').setLogic( d.TE15000ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE3000p0ETA49').setLogic( d.TE30000ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE3500p0ETA49').setLogic( d.TE35000ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE5000p0ETA49').setLogic( d.TE50000ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE6500p0ETA49').setLogic( d.TE65000ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE8000p0ETA49').setLogic( d.TE80000ETA49 & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE9000p0ETA49').setLogic( d.TE90000ETA49 & physcond).setTriggerType(TT.calo)

        MenuItem('L1_TE500p0ETA49_OVERLAY' ).setLogic( d.TE5000ETA49  & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE600p0ETA49_OVERLAY' ).setLogic( d.TE6000ETA49  & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE1500p0ETA49_OVERLAY').setLogic( d.TE15000ETA49 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE3000p0ETA49_OVERLAY').setLogic( d.TE30000ETA49 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE3500p0ETA49_OVERLAY').setLogic( d.TE35000ETA49 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE5000p0ETA49_OVERLAY').setLogic( d.TE50000ETA49 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE6500p0ETA49_OVERLAY').setLogic( d.TE65000ETA49 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE8000p0ETA49_OVERLAY').setLogic( d.TE80000ETA49 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE9000p0ETA49_OVERLAY').setLogic( d.TE90000ETA49 & physcond).setTriggerType(TT.zerobs)

        MenuItem('L1_TE50_VTE600p0ETA49'    ).setLogic( d.TE50 & Not(d.TE6000ETA49) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE50_VTE600p0ETA49_PEB').setLogic( d.TE50 & Not(d.TE6000ETA49) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_TE600p0ETA49_PEB'      ).setLogic( d.TE6000ETA49 & physcond).setTriggerType(TT.calo)

        MenuItem('L1_jTE1500_OVERLAY').setLogic( d.jTE1500 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_jTE4000_OVERLAY').setLogic( d.jTE4000 & physcond).setTriggerType(TT.zerobs)

        MenuItem('L1_VTE2' ).setLogic( Not(d.TE2)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VTE3' ).setLogic( Not(d.TE3)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VTE4' ).setLogic( Not(d.TE4)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VTE5' ).setLogic( Not(d.TE5)  & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VTE10').setLogic( Not(d.TE10) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VTE20').setLogic( Not(d.TE20) & physcond).setTriggerType(TT.calo)
        MenuItem('L1_VTE50').setLogic( Not(d.TE50) & physcond).setTriggerType(TT.calo)

        MenuItem('L1_TE5_NZ').setLogic( d.TE5 & Not(ZDC_AND) & physcond).setTriggerType(TT.calo)

        # MBTS
        MBTS_1   = d.MBTS_A | d.MBTS_C
        MBTS_2   = (d.MBTS_C.x(2) | d.MBTS_A) & (d.MBTS_A.x(2) | d.MBTS_C)
        MBTS_1_1 = d.MBTS_A & d.MBTS_C
        MBTS_2_2 = d.MBTS_A.x(2) & d.MBTS_C.x(2)
        MBTS_1_A = d.MBTS_A.x(1)
        MBTS_1_C = d.MBTS_C.x(1)
        MBTS_2_A = d.MBTS_A.x(2)
        MBTS_2_C = d.MBTS_C.x(2)
        MBTS_3_3 = d.MBTS_A.x(3) & d.MBTS_C.x(3)
        MBTS_4_4 = d.MBTS_A.x(4) & d.MBTS_C.x(4)
        MBTS_4_A = d.MBTS_A.x(4)
        MBTS_4_C = d.MBTS_C.x(4)


        MenuItem('L1_MBTS_A'             ).setLogic( d.MBTS_A   & physcond )
        MenuItem('L1_MBTS_C'             ).setLogic( d.MBTS_C   & physcond )
        
        MenuItem('L1_MBTS_1'             ).setLogic( MBTS_1   & physcond )
        MenuItem('L1_MBTS_1_OVERLAY'     ).setLogic( MBTS_1   & physcond ).setTriggerType(TT.zerobs)
        MenuItem('L1_MBTS_2'             ).setLogic( MBTS_2   & physcond )
        MenuItem('L1_MBTS_1_1'           ).setLogic( MBTS_1_1 & physcond )
        MenuItem('L1_MBTS_1_1_OVERLAY'   ).setLogic( MBTS_1_1 & physcond ).setTriggerType(TT.zerobs)
        MenuItem('L1_MBTS_2_2'           ).setLogic( MBTS_2_2 & physcond )
        MenuItem('L1_MBTS_2_A'           ).setLogic( MBTS_2_A & physcond )
        MenuItem('L1_MBTS_2_C'           ).setLogic( MBTS_2_C & physcond )
        MenuItem('L1_MBTS_3_3'           ).setLogic( MBTS_3_3 & physcond )
        MenuItem('L1_MBTS_4_4'           ).setLogic( MBTS_4_4 & physcond )
        MenuItem('L1_MBTS_4_A'           ).setLogic( MBTS_4_A & physcond )
        MenuItem('L1_MBTS_4_C'           ).setLogic( MBTS_4_C & physcond )
        MenuItem('L1_MBTS_1_A'           ).setLogic( MBTS_1_A & physcond )
        MenuItem('L1_MBTS_1_C'           ).setLogic( MBTS_1_C & physcond )


        MenuItem('L1_MBTS_1_UNPAIRED_ISO'   ).setLogic( MBTS_1   & unpaired_isocond)
        MenuItem('L1_MBTS_2_UNPAIRED_ISO'   ).setLogic( MBTS_2   & unpaired_isocond)
        MenuItem('L1_MBTS_1_1_UNPAIRED_ISO' ).setLogic( MBTS_1_1 & unpaired_isocond)
        MenuItem('L1_MBTS_2_2_UNPAIRED_ISO' ).setLogic( MBTS_2_2 & unpaired_isocond)
        MenuItem('L1_MBTS_3_3_UNPAIRED_ISO' ).setLogic( MBTS_3_3 & unpaired_isocond)
        MenuItem('L1_MBTS_4_4_UNPAIRED_ISO' ).setLogic( MBTS_3_3 & unpaired_isocond)
        MenuItem('L1_MBTS_1_UNPAIRED_NONISO').setLogic( MBTS_1   & unpaired_nonisocond)
        MenuItem('L1_MBTS_2_UNPAIRED_NONISO').setLogic( MBTS_2   & unpaired_nonisocond)
        MenuItem('L1_MBTS_4_A_UNPAIRED_ISO' ).setLogic( MBTS_4_A & unpaired_isocond )
        MenuItem('L1_MBTS_4_C_UNPAIRED_ISO' ).setLogic( MBTS_4_C & unpaired_isocond )

        MenuItem('L1_MBTS_1_A_EMPTY'    ).setLogic( MBTS_1_A   & cosmiccond )
        MenuItem('L1_MBTS_1_C_EMPTY'    ).setLogic( MBTS_1_C   & cosmiccond )
        MenuItem('L1_MBTS_1_EMPTY'    ).setLogic( MBTS_1   & cosmiccond )
        MenuItem('L1_MBTS_2_EMPTY'    ).setLogic( MBTS_2   & cosmiccond )
        MenuItem('L1_MBTS_1_1_EMPTY'  ).setLogic( MBTS_1_1 & cosmiccond )
        MenuItem('L1_MBTS_2_2_EMPTY'  ).setLogic( MBTS_2_2 & cosmiccond )
        MenuItem('L1_MBTS_3_3_EMPTY'  ).setLogic( MBTS_3_3 & cosmiccond )

        MenuItem('L1_MBTS_1_1_VTE50'  ).setLogic( MBTS_1_1  & Not(d.TE50) & physcond)
        MenuItem('L1_MBTS_2_2_VTE50'  ).setLogic( MBTS_2_2  & Not(d.TE50) & physcond)

        MenuItem('L1_MBTS_1_VTE2'     ).setLogic( MBTS_1  & Not(d.TE2) & physcond)
        MenuItem('L1_MBTS_1_VTE3'     ).setLogic( MBTS_1  & Not(d.TE3) & physcond)
        MenuItem('L1_MBTS_1_VTE4'     ).setLogic( MBTS_1  & Not(d.TE4) & physcond)
        MenuItem('L1_MBTS_1_VTE5'     ).setLogic( MBTS_1  & Not(d.TE5) & physcond)
        MenuItem('L1_MBTS_1_VTE10'    ).setLogic( MBTS_1  & Not(d.TE10) & physcond)
        MenuItem('L1_MBTS_1_VTE70'    ).setLogic( MBTS_1  & Not(d.TE70) & physcond)
        MenuItem('L1_MBTS_1_VTE50'    ).setLogic( MBTS_1  & Not(d.TE50) & physcond)
        MenuItem('L1_MBTS_1_VTE200'   ).setLogic( MBTS_1  & Not(d.TE200) & physcond)
        # phase-1
        MenuItem('L1_MBTS_1_VjTE200'   ).setLogic( MBTS_1  & Not(d.jTE200) & physcond)
        MenuItem('L1_MBTS_1_VjTE200_GAP_A'   ).setLogic( MBTS_1  & Not(d.jTE200) & GAPA & physcond)
        MenuItem('L1_MBTS_1_VjTE200_GAP_C'   ).setLogic( MBTS_1  & Not(d.jTE200) & GAPC & physcond)
        MenuItem('L1_MBTS_1_1_VjTE50'  ).setLogic( MBTS_1_1  & Not(d.jTE50) & physcond)

        MenuItem('L1_MBTS_2_VTE2'     ).setLogic( MBTS_2  & Not(d.TE2) & physcond)
        MenuItem('L1_MBTS_2_VTE3'     ).setLogic( MBTS_2  & Not(d.TE3) & physcond)
        MenuItem('L1_MBTS_2_VTE4'     ).setLogic( MBTS_2  & Not(d.TE4) & physcond)
        MenuItem('L1_MBTS_2_VTE5'     ).setLogic( MBTS_2  & Not(d.TE5) & physcond)
        MenuItem('L1_MBTS_2_VTE10'    ).setLogic( MBTS_2  & Not(d.TE10) & physcond)

        MenuItem('L1_MBTSA0' ).setLogic( d.MBTS_A0 & physcond)
        MenuItem('L1_MBTSA1' ).setLogic( d.MBTS_A1 & physcond)
        MenuItem('L1_MBTSA2' ).setLogic( d.MBTS_A2 & physcond)
        MenuItem('L1_MBTSA3' ).setLogic( d.MBTS_A3 & physcond)
        MenuItem('L1_MBTSA4' ).setLogic( d.MBTS_A4 & physcond)
        MenuItem('L1_MBTSA5' ).setLogic( d.MBTS_A5 & physcond)
        MenuItem('L1_MBTSA6' ).setLogic( d.MBTS_A6 & physcond)
        MenuItem('L1_MBTSA7' ).setLogic( d.MBTS_A7 & physcond)
        MenuItem('L1_MBTSA8' ).setLogic( d.MBTS_A8 & physcond)
        MenuItem('L1_MBTSA10').setLogic( d.MBTS_A10 & physcond)
        MenuItem('L1_MBTSA12').setLogic( d.MBTS_A12 & physcond)
        MenuItem('L1_MBTSA14').setLogic( d.MBTS_A14 & physcond)

        MenuItem('L1_MBTSA9' ).setLogic( d.MBTS_A9 & physcond)
        MenuItem('L1_MBTSA11').setLogic( d.MBTS_A11 & physcond)
        MenuItem('L1_MBTSA13').setLogic( d.MBTS_A13 & physcond)
        MenuItem('L1_MBTSA15').setLogic( d.MBTS_A15 & physcond)

        MenuItem('L1_MBTSC0' ).setLogic( d.MBTS_C0 & physcond)
        MenuItem('L1_MBTSC1' ).setLogic( d.MBTS_C1 & physcond)
        MenuItem('L1_MBTSC2' ).setLogic( d.MBTS_C2 & physcond)
        MenuItem('L1_MBTSC3' ).setLogic( d.MBTS_C3 & physcond)
        MenuItem('L1_MBTSC4' ).setLogic( d.MBTS_C4 & physcond)
        MenuItem('L1_MBTSC5' ).setLogic( d.MBTS_C5 & physcond)
        MenuItem('L1_MBTSC6' ).setLogic( d.MBTS_C6 & physcond)
        MenuItem('L1_MBTSC7' ).setLogic( d.MBTS_C7 & physcond)
        MenuItem('L1_MBTSC8' ).setLogic( d.MBTS_C8 & physcond)
        MenuItem('L1_MBTSC10').setLogic( d.MBTS_C10 & physcond)
        MenuItem('L1_MBTSC12').setLogic( d.MBTS_C12 & physcond)
        MenuItem('L1_MBTSC14').setLogic( d.MBTS_C14 & physcond)

        MenuItem('L1_MBTSC9' ).setLogic( d.MBTS_C9 & physcond)
        MenuItem('L1_MBTSC11').setLogic( d.MBTS_C11 & physcond)
        MenuItem('L1_MBTSC13').setLogic( d.MBTS_C13 & physcond)
        MenuItem('L1_MBTSC15').setLogic( d.MBTS_C15 & physcond)

        MenuItem('L1_MBTS_1_BGRP9'   ).setLogic( MBTS_1 & bgrp9cond )
        MenuItem('L1_MBTS_1_1_BGRP9' ).setLogic( MBTS_1_1 & bgrp9cond )
        MenuItem('L1_MBTS_2_BGRP9'   ).setLogic( MBTS_2 & bgrp9cond )

        MenuItem('L1_MBTS_1_BGRP11'  ).setLogic( MBTS_1 & bgrp11cond )
        MenuItem('L1_MBTS_1_1_BGRP11').setLogic( MBTS_1_1 & bgrp11cond)
        MenuItem('L1_MBTS_2_BGRP11'  ).setLogic( MBTS_2 & bgrp11cond )


        # ZDC

        MenuItem('L1_ZDC'             ).setLogic( (ZDC_A | ZDC_C) & physcond)
        MenuItem('L1_ZDC_A'           ).setLogic( ZDC_A & physcond)
        MenuItem('L1_ZDC_C'           ).setLogic( ZDC_C & physcond)
        MenuItem('L1_ZDC_AND'         ).setLogic( ZDC_AND & physcond)
        MenuItem('L1_ZDC_A_C'         ).setLogic( ZDC_A_C & physcond)
        
        # ZDC O+O (ATR-30690)
        MenuItem('L1_ZDC_XNXN'        ).setLogic( ZDC_XnXn & physcond)
        MenuItem('L1_ZDC_XNYN'        ).setLogic( ZDC_XnYn & physcond)
        MenuItem('L1_ZDC_XNZN'        ).setLogic( ZDC_XnZn & physcond)
        MenuItem('L1_ZDC_XN_XOR'        ).setLogic( ZDC_Xn_XOR & physcond)
        MenuItem('L1_ZDC_YN_XOR'        ).setLogic( ZDC_Yn_XOR & physcond)
        MenuItem('L1_ZDC_ZN_XOR'        ).setLogic( ZDC_Zn_XOR & physcond)
        MenuItem('L1_ZDC_YN'        ).setLogic( ZDC_Yn & physcond)
        MenuItem('L1_ZDC_ZN'        ).setLogic( ZDC_Zn & physcond)
        MenuItem('L1_ZDC_LOR'        ).setLogic( ZDC_LOR & physcond)
        MenuItem('L1_ZDC_YNYN'        ).setLogic( ZDC_YnYn & physcond)
        MenuItem('L1_ZDC_LOR_EMPTY'          ).setLogic( ZDC_LOR & cosmiccond)
        MenuItem('L1_ZDC_LOR_UNPAIRED_ISO'   ).setLogic( ZDC_LOR & unpaired_isocond)
        MenuItem('L1_ZDC_LOR_UNPAIRED_NONISO').setLogic( ZDC_LOR & unpaired_nonisocond)
        # ZDC+TRT O+O (ATR-30690)
        MenuItem('L1_TRT_ZDC_OR'      ).setLogic( d.NIMTRT & ZDC_OR & physcond)
        MenuItem('L1_TRT_ZDC_XNXN'        ).setLogic( d.NIMTRT & ZDC_XnXn & physcond)
        MenuItem('L1_TRT_ZDC_XNYN'        ).setLogic( d.NIMTRT & ZDC_XnYn & physcond)
        MenuItem('L1_TRT_ZDC_XNZN'        ).setLogic( d.NIMTRT & ZDC_XnZn & physcond)
        MenuItem('L1_TRT_ZDC_XN_XOR'        ).setLogic( d.NIMTRT & ZDC_Xn_XOR & physcond)
        MenuItem('L1_TRT_ZDC_YN_XOR'        ).setLogic( d.NIMTRT & ZDC_Yn_XOR & physcond)
        MenuItem('L1_TRT_ZDC_ZN_XOR'        ).setLogic( d.NIMTRT & ZDC_Zn_XOR & physcond)
        MenuItem('L1_TRT_ZDC_YN'        ).setLogic( d.NIMTRT & ZDC_Yn & physcond)
        MenuItem('L1_TRT_ZDC_ZN'        ).setLogic( d.NIMTRT & ZDC_Zn & physcond)
        MenuItem('L1_TRT_ZDC_LOR'        ).setLogic( d.NIMTRT & ZDC_LOR & physcond)
        MenuItem('L1_TRT_ZDC_YNYN'        ).setLogic( d.NIMTRT & ZDC_YnYn & physcond)
        MenuItem('L1_TRT_ZDC_A'           ).setLogic( d.NIMTRT & ZDC_A & physcond)
        MenuItem('L1_TRT_ZDC_C'           ).setLogic( d.NIMTRT & ZDC_C & physcond)
        MenuItem('L1_TRT_ZDC_A_C'         ).setLogic( d.NIMTRT & ZDC_A_C & physcond)

        # ZDC pp (ATR-29027)
        MenuItem('L1_ZDC_PP_A'        ).setLogic( ZDC_PP_A & physcond)
        MenuItem('L1_ZDC_PP_C'        ).setLogic( ZDC_PP_C & physcond)
        MenuItem('L1_ZDC_PP_OR'       ).setLogic( ZDC_PP_OR & physcond)
        MenuItem('L1_ZDC_PP_A_C'      ).setLogic( ZDC_PP_A_C & physcond)
        MenuItem('L1_ZDC_PP_A2'        ).setLogic( ZDC_PP_A2 & physcond)
        MenuItem('L1_ZDC_PP_C2'        ).setLogic( ZDC_PP_C2 & physcond)
        MenuItem('L1_ZDC_PP_OR2'       ).setLogic( ZDC_PP_OR2 & physcond)
        MenuItem('L1_ZDC_PP_A_EMPTY'        ).setLogic( ZDC_PP_A & cosmiccond)
        MenuItem('L1_ZDC_PP_C_EMPTY'        ).setLogic( ZDC_PP_C & cosmiccond)
        MenuItem('L1_ZDC_PP_A2_EMPTY'        ).setLogic( ZDC_PP_A2 & cosmiccond)
        MenuItem('L1_ZDC_PP_C2_EMPTY'        ).setLogic( ZDC_PP_C2 & cosmiccond)
        MenuItem('L1_ZDC_PP_A_UNPAIRED_NONISO'        ).setLogic( ZDC_PP_A & unpaired_nonisocond)
        MenuItem('L1_ZDC_PP_C_UNPAIRED_NONISO'        ).setLogic( ZDC_PP_C & unpaired_nonisocond)
        MenuItem('L1_ZDC_PP_A2_UNPAIRED_NONISO'        ).setLogic( ZDC_PP_A2 & unpaired_nonisocond)
        MenuItem('L1_ZDC_PP_C2_UNPAIRED_NONISO'        ).setLogic( ZDC_PP_C2 & unpaired_nonisocond)

        MenuItem('L1_ZDC_A_C_OVERLAY' ).setLogic( ZDC_A_C & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_ZDC_A_OVERLAY'   ).setLogic( ZDC_A & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_ZDC_C_OVERLAY'   ).setLogic( ZDC_C & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_ZDC_A_C_VTE50_OVERLAY').setLogic( ZDC_A_C & Not(d.TE50) & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE5_OVERLAY'     ).setLogic( d.TE5 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE10_OVERLAY'    ).setLogic( d.TE10 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE20_OVERLAY'    ).setLogic( d.TE20 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_TE50_OVERLAY'    ).setLogic( d.TE50 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_MBTS_1_VTE50_OVERLAY' ).setLogic( MBTS_1 & Not(d.TE50) & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_jTE50_OVERLAY'    ).setLogic( d.jTE50 & physcond).setTriggerType(TT.zerobs)
        MenuItem('L1_ZDC_A_C_VjTE50_OVERLAY').setLogic( ZDC_A_C & Not(d.jTE50) & physcond).setTriggerType(TT.zerobs)

        MenuItem('L1_ZDC_VTE200'      ).setLogic((ZDC_A | ZDC_C) & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_AND_VTE50'   ).setLogic( ZDC_AND & Not(d.TE50) & physcond)
        MenuItem('L1_ZDC_A_C_VTE50'   ).setLogic( ZDC_A_C & Not(d.TE50) & physcond)
        MenuItem('L1_ZDC_A_C_TE50'    ).setLogic( ZDC_A_C & d.TE50 & physcond)
        MenuItem('L1_ZDC_A_C_VjTE50'   ).setLogic( ZDC_A_C & Not(d.jTE50) & physcond)
        MenuItem('L1_ZDC_OR_VjTE50' ).setLogic(ZDC_OR  & Not(d.jTE50) & physcond)
        MenuItem('L1_ZDC_XOR_VjTE50').setLogic(ZDC_XOR & Not(d.jTE50) & physcond)
        MenuItem('L1_TRT_ZDC_OR_VjTE50' ).setLogic(d.NIMTRT & ZDC_XOR & Not(d.jTE50) & physcond)
        MenuItem('L1_TRT_ZDC_A_C_VjTE50').setLogic(d.NIMTRT & ZDC_A_C & Not(d.jTE50) & physcond)
        MenuItem('L1_TRT_ZDC_XOR_VjTE50').setLogic(d.NIMTRT & ZDC_XOR & Not(d.jTE50) & physcond)

        MenuItem('L1_ZDC_A_C_VTE50_PEB').setLogic( ZDC_A_C & Not(d.TE50) & physcond)

        MenuItem('L1_ZDC_A_VTE20'      ).setLogic( ZDC_A & Not(d.TE20) & physcond)
        MenuItem('L1_ZDC_A_VTE200'     ).setLogic( ZDC_A & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_C_VTE20'      ).setLogic( ZDC_C & Not(d.TE20) & physcond)
        MenuItem('L1_ZDC_C_VTE200'     ).setLogic( ZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TRT_ZDC_A_VTE50'      ).setLogic( d.NIMTRT & ZDC_A & Not(d.TE50) & physcond)
        MenuItem('L1_TRT_ZDC_C_VTE50'      ).setLogic( d.NIMTRT & ZDC_C & Not(d.TE50) & physcond)

        MenuItem('L1_ZDC_A_C_VTE200'    ).setLogic( ZDC_A_C & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_A_C_TE5_VTE200').setLogic( ZDC_A_C & d.TE5 & Not(d.TE200) & physcond)

        MenuItem('L1_VZDC_A_C_VTE50'      ).setLogic( VZDC_A_C & Not(d.TE50) & physcond)
        MenuItem('L1_VZDC_A_C_TE5_VTE200' ).setLogic( VZDC_A_C & d.TE5 & Not(d.TE200) & physcond)
        MenuItem('L1_VZDC_A_C_TE20_VTE200').setLogic( VZDC_A_C & d.TE20 & Not(d.TE200) & physcond)
        MenuItem('L1_VZDC_AORC_TE5_VTE200').setLogic( VZDC_AORC & d.TE5 & Not(d.TE200) & physcond)
        MenuItem('L1_TE20_ZDC_A_VZDC_C_VTE200').setLogic( ZDC_A & Not(ZDC_C) & d.TE20 & Not(d.TE200) & physcond)
        MenuItem('L1_TE20_ZDC_C_VZDC_A_VTE200').setLogic( ZDC_C & Not(ZDC_A) & d.TE20 & Not(d.TE200) & physcond)

        MenuItem('L1_ZDC_MBTS_1'        ).setLogic((ZDC_A | ZDC_C) & MBTS_1 & physcond)
        MenuItem('L1_ZDC_MBTS_2'        ).setLogic((ZDC_A | ZDC_C) & MBTS_2 & physcond)

        MenuItem('L1_ZDC_MBTS_1_1'      ).setLogic((ZDC_A | ZDC_C) & MBTS_1_1 & physcond)
        MenuItem('L1_ZDC_MBTS_2_2'      ).setLogic((ZDC_A | ZDC_C) & MBTS_2_2 & physcond)

        MenuItem('L1_ZDC_EMPTY'           ).setLogic( (ZDC_A | ZDC_C) & cosmiccond )
        MenuItem('L1_ZDC_A_EMPTY'           ).setLogic( ZDC_A & cosmiccond )
        MenuItem('L1_ZDC_C_EMPTY'           ).setLogic( ZDC_C & cosmiccond )
        MenuItem('L1_ZDC_UNPAIRED_ISO'    ).setLogic( (ZDC_A | ZDC_C) & unpaired_isocond )
        MenuItem('L1_ZDC_UNPAIRED_NONISO' ).setLogic( (ZDC_A | ZDC_C) & unpaired_nonisocond )

        MenuItem('L1_ZDC_AND_EMPTY'           ).setLogic( ZDC_AND & cosmiccond )
        MenuItem('L1_ZDC_AND_UNPAIRED_ISO'    ).setLogic( ZDC_AND & unpaired_isocond )
        MenuItem('L1_ZDC_AND_UNPAIRED_NONISO' ).setLogic( ZDC_AND & unpaired_nonisocond )
        MenuItem('L1_ZDC_A_UNPAIRED_NONISO' ).setLogic( ZDC_A & unpaired_nonisocond )
        MenuItem('L1_ZDC_C_UNPAIRED_NONISO' ).setLogic( ZDC_C & unpaired_nonisocond )

        MenuItem('L1_ZDC_A_C_EMPTY'           ).setLogic( ZDC_A_C & cosmiccond )
        MenuItem('L1_ZDC_A_C_UNPAIRED_ISO'    ).setLogic( ZDC_A_C & unpaired_isocond )
        MenuItem('L1_ZDC_A_C_UNPAIRED_NONISO' ).setLogic( ZDC_A_C & unpaired_nonisocond )
        MenuItem('L1_ZDC_A_UNPAIRED_ISO'    ).setLogic( ZDC_A & unpaired_isocond )
        MenuItem('L1_ZDC_C_UNPAIRED_ISO'    ).setLogic( ZDC_C & unpaired_isocond )

        MenuItem('L1_ZDC_A_C_BGRP9'      ).setLogic( ZDC_A_C & bgrp9cond & physcond)
        MenuItem('L1_ZDC_A_BGRP9'      ).setLogic( ZDC_A & bgrp9cond & physcond)
        MenuItem('L1_ZDC_C_BGRP9'      ).setLogic( ZDC_C & bgrp9cond & physcond)
        MenuItem('L1_ZDC_A_C_BGRP11'     ).setLogic( ZDC_A_C & bgrp11cond & physcond)

# ATR-12470
        MenuItem('L1_ZDC_A_VZDC_C'                  ).setLogic(PHYS_ZDC_A_VZDC_C & physcond)
        MenuItem('L1_ZDC_C_VZDC_A'                  ).setLogic(PHYS_VZDC_A_ZDC_C & physcond)
        MenuItem('L1_ZDC_C_VZDC_A_VTE200'           ).setLogic(PHYS_VZDC_A_ZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_MBTS_1_ZDC_A_VZDC_C_VTE200'    ).setLogic(MBTS_1      & PHYS_ZDC_A_VZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_MBTS_1_ZDC_C_VZDC_A_VTE200'    ).setLogic(MBTS_1      & PHYS_VZDC_A_ZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE3p0ETA49_ZDC_A_VZDC_C_VTE200').setLogic(d.TE30ETA49 & PHYS_ZDC_A_VZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE3p0ETA49_ZDC_C_VZDC_A_VTE200').setLogic(d.TE30ETA49 & PHYS_VZDC_A_ZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE4_ZDC_A_VZDC_C_VTE200'       ).setLogic(d.TE4       & PHYS_ZDC_A_VZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE4_ZDC_C_VZDC_A_VTE200'       ).setLogic(d.TE4       & PHYS_VZDC_A_ZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE5_ZDC_A_VZDC_C_VTE200'       ).setLogic(d.TE5       & PHYS_ZDC_A_VZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE5_ZDC_C_VZDC_A_VTE200'       ).setLogic(d.TE5       & PHYS_VZDC_A_ZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE7p0ETA49_ZDC_A_VZDC_C_VTE200').setLogic(d.TE70ETA49 & PHYS_ZDC_A_VZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_TE7p0ETA49_ZDC_C_VZDC_A_VTE200').setLogic(d.TE70ETA49 & PHYS_VZDC_A_ZDC_C & Not(d.TE200) & physcond)


        MenuItem('L1_ZDC_XOR'                  ).setLogic(ZDC_XOR & physcond)
        MenuItem('L1_ZDC_XOR_TE3p0ETA49_VTE200').setLogic(ZDC_XOR & d.TE30ETA49 & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_XOR_TE4_VTE200'       ).setLogic(ZDC_XOR & d.TE4 & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_XOR_TE5_VTE200'       ).setLogic(ZDC_XOR & d.TE5 & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_XOR_TE20_VTE200'      ).setLogic(ZDC_XOR & d.TE20 & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_XOR_TRT_VTE200'       ).setLogic(ZDC_XOR & d.NIMTRT & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_XOR_VTE50'            ).setLogic(ZDC_XOR & Not(d.TE50) & physcond)
        MenuItem('L1_ZDC_XOR_VTE200'           ).setLogic(ZDC_XOR & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_XOR_VTE200_MBTS_1'    ).setLogic(ZDC_XOR & Not(d.TE200) & MBTS_1 & physcond)


        # ATR-26051
        # ZDC for 2022 LHCf+ZDC special run, item names are set to be different from ZDC items for heavy ion runs
        MenuItem('L1_ZDC_OR'           ).setLogic( ZDC_OR            & physcond)
        MenuItem('L1_ZDC_XOR_E2'       ).setLogic( ZDC_XOR_E2        & physcond)
        MenuItem('L1_ZDC_XOR_E1_E3'    ).setLogic( ZDC_XOR_E1_E3     & physcond)
        MenuItem('L1_ZDC_E1_AND_E1'    ).setLogic( ZDC_E1_AND_E1     & physcond)
        MenuItem('L1_ZDC_E1_AND_E2ORE3').setLogic( ZDC_E1_AND_E2ORE3 & physcond)
        MenuItem('L1_ZDC_E2_AND_E2'    ).setLogic( ZDC_E2_AND_E2     & physcond)
        MenuItem('L1_ZDC_E2_AND_E3'    ).setLogic( ZDC_E2_AND_E3     & physcond)
        MenuItem('L1_ZDC_E3_AND_E3'    ).setLogic( ZDC_E3_AND_E3     & physcond)
        MenuItem('L1_ZDC_A_AND_C'      ).setLogic( ZDC_A_AND_C       & physcond)
        MenuItem('L1_ZDC_A_AND_C_EMPTY'     ).setLogic( ZDC_A_AND_C  & cosmiccond)
        MenuItem('L1_ZDC_A_AND_C_UNPAIRED_NONISO' ).setLogic( ZDC_A_AND_C   & unpaired_nonisocond)
        MenuItem('L1_ZDC_OR_EMPTY'          ).setLogic( ZDC_OR & cosmiccond)
        MenuItem('L1_ZDC_OR_UNPAIRED_ISO'   ).setLogic( ZDC_OR & unpaired_isocond)
        MenuItem('L1_ZDC_OR_UNPAIRED_NONISO').setLogic( ZDC_OR & unpaired_nonisocond)
        # individual ZDC bits
        MenuItem('L1_ZDC_BIT2').setLogic( d.ZDC_2 & physcond)
        MenuItem('L1_ZDC_BIT1').setLogic( d.ZDC_1 & physcond)
        MenuItem('L1_ZDC_BIT0').setLogic( d.ZDC_0 & physcond)
        # individual ZDC comb
        MenuItem('L1_ZDC_COMB0').setLogic( ZDC_comb0 & physcond)
        MenuItem('L1_ZDC_COMB1').setLogic( ZDC_comb1 & physcond)
        MenuItem('L1_ZDC_COMB2').setLogic( ZDC_comb2 & physcond)
        MenuItem('L1_ZDC_COMB3').setLogic( ZDC_comb3 & physcond)
        MenuItem('L1_ZDC_COMB4').setLogic( ZDC_comb4 & physcond)
        MenuItem('L1_ZDC_COMB5').setLogic( ZDC_comb5 & physcond)
        MenuItem('L1_ZDC_COMB6').setLogic( ZDC_comb6 & physcond)
        MenuItem('L1_ZDC_COMB7').setLogic( ZDC_comb7 & physcond)
        # ZDC calibration for LHCf+ZDC runs
        MenuItem('L1_ZDC_OR_LHCF').setLogic( (Not(ZDC_comb0) | d.NIMLHCF) & physcond)

        # ZDC for 2023 heavy ion runs
        MenuItem('L1_VZDC_A_VZDC_C'      ).setLogic( PHYS_VZDC_A_VZDC_C       & physcond)
        MenuItem('L1_1ZDC_A_VZDC_C'   ).setLogic( PHYS_1TO4ZDC_A_VZDC_C    & physcond)
        MenuItem('L1_VZDC_A_1ZDC_C'   ).setLogic( PHYS_VZDC_A_1TO4ZDC_C    & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C').setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & physcond)
        MenuItem('L1_5ZDC_A_VZDC_C'      ).setLogic( PHYS_5ZDC_A_VZDC_C       & physcond)
        MenuItem('L1_VZDC_A_5ZDC_C'      ).setLogic( PHYS_VZDC_A_5ZDC_C       & physcond)
        MenuItem('L1_ZDC_1XOR5'       ).setLogic( PHYS_ZDC_1TO4XOR5        & physcond)
        MenuItem('L1_5ZDC_A_5ZDC_C'      ).setLogic( PHYS_5ZDC_A_5ZDC_C       & physcond)

        MenuItem('L1_1ZDC_A_1ZDC_C_VTE200').setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.TE200) & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_VjTE200').setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200) & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_VjTE200_GAP_AANDC').setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200) & GAPAC & physcond)
        MenuItem('L1_ZDC_1XOR5_VTE200').setLogic( PHYS_ZDC_1TO4XOR5 & Not(d.TE200) & physcond)
        MenuItem('L1_ZDC_1XOR5_VjTE200').setLogic( PHYS_ZDC_1TO4XOR5 & Not(d.jTE200) & physcond)
        MenuItem('L1_ZDC_XOR_VjTE200' ).setLogic(ZDC_XOR & Not(d.jTE200) & physcond)
        MenuItem('L1_MBTS_1_VZDC_A_ZDC_C_VTE200' ).setLogic( MBTS_1 & PHYS_VZDC_A_ZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_1_VZDC_A_ZDC_C_VjTE200' ).setLogic( MBTS_1 & PHYS_VZDC_A_ZDC_C & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_VZDC_A_ZDC_C_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_VZDC_A_ZDC_C & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_VZDC_A_ZDC_C_jTE3_VjTE200' ).setLogic( MBTS_1 & PHYS_VZDC_A_ZDC_C & d.jTE3 &Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_VZDC_A_ZDC_C_jTE3_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_VZDC_A_ZDC_C & d.jTE3 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_VZDC_A_ZDC_C_jTE5_VjTE200' ).setLogic( MBTS_1 & PHYS_VZDC_A_ZDC_C & d.jTE5 &Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_VZDC_A_ZDC_C_jTE5_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_VZDC_A_ZDC_C & d.jTE5 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_VTE200' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_VjTE200' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_jTE3_VjTE200' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE3 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_jTE3_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE3 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_jTE5_VjTE200' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE5 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_NZDC_VjTE200' ).setLogic( MBTS_1 & PHYS_1ZDC_NZDC & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_1ZDC_NZDC_jTE3_VjTE200' ).setLogic( MBTS_1 & PHYS_1ZDC_NZDC & d.jTE3 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_1ZDC_NZDC_jTE3_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_1ZDC_NZDC & d.jTE3 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_NZDC_jTE3_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_1ZDC_NZDC & d.jTE3 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_NZDC_jTE5_VjTE200' ).setLogic( MBTS_1 & PHYS_1ZDC_NZDC & d.jTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_1ZDC_NZDC_jTE5_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_1ZDC_NZDC & d.jTE5 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_NZDC_jTE5_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_1ZDC_NZDC & d.jTE5 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_VTE200' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_VjTE200' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_jTE3_VjTE200' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & d.jTE3 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_jTE5_VjTE200' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & d.jTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_jTE5_VjTE200_GAP_A' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & d.jTE5 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_1_ZDC_A_VZDC_C_VjTE200' ).setLogic( MBTS_1 & PHYS_ZDC_A_VZDC_C & Not(d.jTE200)  & physcond)
        MenuItem('L1_MBTS_1_ZDC_A_VZDC_C_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_ZDC_A_VZDC_C & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_ZDC_A_VZDC_C_jTE3_VjTE200' ).setLogic( MBTS_1 & PHYS_ZDC_A_VZDC_C & d.jTE3 & Not(d.jTE200)  & physcond)
        MenuItem('L1_MBTS_1_ZDC_A_VZDC_C_jTE3_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_ZDC_A_VZDC_C & d.jTE3 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_ZDC_A_VZDC_C_jTE5_VjTE200' ).setLogic( MBTS_1 & PHYS_ZDC_A_VZDC_C & d.jTE5 & Not(d.jTE200)  & physcond)
        MenuItem('L1_MBTS_1_ZDC_A_VZDC_C_jTE5_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_ZDC_A_VZDC_C & d.jTE5 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_1ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE5 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_1_ZDC_1XOR5_jTE5_VjTE200_GAP_C' ).setLogic( MBTS_1 & PHYS_ZDC_1TO4XOR5 & d.jTE5 & Not(d.jTE200) & GAPC  & physcond)

        MenuItem('L1_VZDC_A_ZDC_C_TE3_VTE200' ).setLogic( PHYS_VZDC_A_ZDC_C & d.TE3 & Not(d.TE200)   & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_jTE3_VjTE200' ).setLogic(  PHYS_VZDC_A_ZDC_C &d.jTE3 &  Not(d.jTE200) & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_jTE3_VjTE200_GAP_A' ).setLogic(  PHYS_VZDC_A_ZDC_C &d.jTE3 &  Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_gTE3_VjTE200' ).setLogic(  PHYS_VZDC_A_ZDC_C &d.gTE3 &  Not(d.jTE200) & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_TE3_VTE200' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.TE3 &  Not(d.TE200)   & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_jTE3_VjTE200' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE3 &  Not(d.jTE200)   & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_jTE3_VjTE200_GAP_A' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE3 &  Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_1ZDC_NZDC_jTE3_VjTE200' ).setLogic( PHYS_1ZDC_NZDC & d.jTE3 &  Not(d.jTE200)   & physcond)
        MenuItem('L1_1ZDC_NZDC_jTE3_VjTE200_GAP_A' ).setLogic( PHYS_1ZDC_NZDC & d.jTE3 &  Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_1ZDC_NZDC_jTE3_VjTE200_GAP_C' ).setLogic( PHYS_1ZDC_NZDC & d.jTE3 &  Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_1XOR5_TE3_VTE200' ).setLogic( PHYS_ZDC_1TO4XOR5 & d.TE3 & Not(d.TE200)   & physcond)
        MenuItem('L1_ZDC_1XOR5_jTE3_VjTE200' ).setLogic( PHYS_ZDC_1TO4XOR5 & d.jTE3 & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_1XOR5_jTE3_VjTE200_GAP_A' ).setLogic( PHYS_ZDC_1TO4XOR5 & d.jTE3 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_TE3_VTE200' ).setLogic( PHYS_ZDC_A_VZDC_C & d.TE3 & Not(d.TE200)   & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_jTE3_VjTE200' ).setLogic( PHYS_ZDC_A_VZDC_C & d.jTE3 & Not(d.jTE200)  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_jTE3_VjTE200_GAP_C' ).setLogic( PHYS_ZDC_A_VZDC_C & d.jTE3 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_gTE3_VjTE200' ).setLogic( PHYS_ZDC_A_VZDC_C & d.gTE3 & Not(d.jTE200)  & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_jTE3_VjTE200_GAP_C' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE3 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_1XOR5_jTE3_VjTE200_GAP_C' ).setLogic(  PHYS_ZDC_1TO4XOR5 & d.jTE3 & Not(d.jTE200) & GAPC  & physcond)

        MenuItem('L1_VZDC_A_ZDC_C_TE5_VTE200' ).setLogic( PHYS_VZDC_A_ZDC_C & d.TE5 & Not(d.TE200)   & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_jTE5_VjTE200' ).setLogic(  PHYS_VZDC_A_ZDC_C &d.jTE5 &  Not(d.jTE200)  & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_jTE5_VjTE200_GAP_A' ).setLogic(  PHYS_VZDC_A_ZDC_C &d.jTE5 &  Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_gTE5_VjTE200' ).setLogic(  PHYS_VZDC_A_ZDC_C &d.gTE5 &  Not(d.jTE200)  & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_TE5_VTE200' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.TE5 &  Not(d.TE200)   & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_jTE5_VjTE200' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE5 &  Not(d.jTE200)   & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_A' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE5 &  Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_1ZDC_NZDC_jTE5_VjTE200_GAP_A' ).setLogic( PHYS_1ZDC_NZDC & d.jTE5 &  Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_1ZDC_NZDC_jTE5_VjTE200_GAP_C' ).setLogic( PHYS_1ZDC_NZDC & d.jTE5 &  Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_1XOR5_TE5_VTE200' ).setLogic( PHYS_ZDC_1TO4XOR5 & d.TE5 & Not(d.TE200)   & physcond)
        MenuItem('L1_ZDC_1XOR5_jTE5_VjTE200' ).setLogic( PHYS_ZDC_1TO4XOR5 & d.jTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_1XOR5_jTE5_VjTE200_GAP_A' ).setLogic( PHYS_ZDC_1TO4XOR5 & d.jTE5 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_TE5_VTE200' ).setLogic( PHYS_ZDC_A_VZDC_C & d.TE5 & Not(d.TE200)   & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_jTE5_VjTE200' ).setLogic( PHYS_ZDC_A_VZDC_C & d.jTE5 & Not(d.jTE200)  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_jTE5_VjTE200_GAP_C' ).setLogic( PHYS_ZDC_A_VZDC_C & d.jTE5 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_gTE5_VjTE200' ).setLogic( PHYS_ZDC_A_VZDC_C & d.gTE5 & Not(d.jTE200)  & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_jTE5_VjTE200_GAP_C' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & d.jTE5 & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_1XOR5_jTE5_VjTE200_GAP_C' ).setLogic(  PHYS_ZDC_1TO4XOR5 & d.jTE5 & Not(d.jTE200) & GAPC  & physcond)

        MenuItem('L1_MBTS_1_ZDC_XOR_VTE200' ).setLogic( MBTS_1 & ZDC_XOR & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_1_ZDC_XOR_VjTE200' ).setLogic( MBTS_1 & ZDC_XOR & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_XOR_TE3_VTE200' ).setLogic( ZDC_XOR & d.TE3 & Not(d.TE200)   & physcond)
        MenuItem('L1_ZDC_XOR_jTE3_VjTE200' ).setLogic( ZDC_XOR & d.jTE3 & Not(d.jTE200)   & physcond)
        
        MenuItem('L1_VZDC_A_ZDC_C_VTE200' ).setLogic( PHYS_VZDC_A_ZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_VjTE200' ).setLogic( PHYS_VZDC_A_ZDC_C & Not(d.jTE200)   & physcond)
        MenuItem('L1_VZDC_A_ZDC_C_VjTE200_GAP_A' ).setLogic( PHYS_VZDC_A_ZDC_C & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_VjTE200_GAP_A' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C  &  Not(d.jTE200) & GAPA   & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_VjTE200_GAP_C' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C  &  Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_1XOR5_VjTE200_GAP_A' ).setLogic( PHYS_ZDC_1TO4XOR5  & Not(d.jTE200) & GAPA   & physcond)
        MenuItem('L1_ZDC_1XOR5_VjTE200_GAP_C' ).setLogic( PHYS_ZDC_1TO4XOR5  & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_VTE200' ).setLogic( PHYS_ZDC_A_VZDC_C  & Not(d.TE200)   & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_VjTE200' ).setLogic( PHYS_ZDC_A_VZDC_C  & Not(d.jTE200)  & physcond)
        MenuItem('L1_ZDC_A_VZDC_C_VjTE200_GAP_C' ).setLogic( PHYS_ZDC_A_VZDC_C  & Not(d.jTE200) & GAPC  & physcond)

        MenuItem('L1_MBTS_2_VZDC_A_ZDC_C_VTE200' ).setLogic( MBTS_2 & PHYS_VZDC_A_ZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_2_VZDC_A_ZDC_C_VjTE200' ).setLogic( MBTS_2 & PHYS_VZDC_A_ZDC_C & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_2_VZDC_A_ZDC_C_VjTE200_GAP_A' ).setLogic( MBTS_2 & PHYS_VZDC_A_ZDC_C & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_2_1ZDC_A_1ZDC_C_VTE200' ).setLogic( MBTS_2 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_2_1ZDC_A_1ZDC_C_VjTE200' ).setLogic( MBTS_2 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_2_1ZDC_A_1ZDC_C_VjTE200_GAP_A' ).setLogic( MBTS_2 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_2_1ZDC_NZDC_VjTE200' ).setLogic( MBTS_2 & PHYS_1ZDC_NZDC & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_2_1ZDC_NZDC_VjTE200_GAP_A' ).setLogic( MBTS_2 & PHYS_1ZDC_NZDC & Not(d.jTE200) & GAPA   & physcond)
        MenuItem('L1_MBTS_2_1ZDC_NZDC_VjTE200_GAP_C' ).setLogic( MBTS_2 & PHYS_1ZDC_NZDC & Not(d.jTE200) & GAPC   & physcond)
        MenuItem('L1_MBTS_2_ZDC_1XOR5_VTE200' ).setLogic( MBTS_2 & PHYS_ZDC_1TO4XOR5 & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_2_ZDC_1XOR5_VjTE200' ).setLogic( MBTS_2 & PHYS_ZDC_1TO4XOR5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_2_ZDC_1XOR5_VjTE200_GAP_A' ).setLogic( MBTS_2 & PHYS_ZDC_1TO4XOR5 & Not(d.jTE200) & GAPA  & physcond)
        MenuItem('L1_MBTS_2_ZDC_A_VZDC_C_VTE200' ).setLogic( MBTS_2 & PHYS_ZDC_A_VZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_MBTS_2_ZDC_A_VZDC_C_VjTE200' ).setLogic( MBTS_2 & PHYS_ZDC_A_VZDC_C & Not(d.jTE200)   & physcond)
        MenuItem('L1_MBTS_2_ZDC_A_VZDC_C_VjTE200_GAP_C' ).setLogic( MBTS_2 & PHYS_ZDC_A_VZDC_C & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_2_1ZDC_A_1ZDC_C_VjTE200_GAP_C' ).setLogic( MBTS_2 & PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE200) & GAPC  & physcond)
        MenuItem('L1_MBTS_2_ZDC_1XOR5_VjTE200_GAP_C' ).setLogic( MBTS_2 & PHYS_ZDC_1TO4XOR5 & Not(d.jTE200) & GAPC  & physcond)

        MenuItem('L1_VZDC_A_VZDC_C_TE5_VTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & d.TE5 & Not(d.TE200)   & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_jTE5_VjTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & d.jTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_gTE5_VjTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & d.gTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_TE10_VTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & d.TE10 & Not(d.TE200)   & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_jTE10_VjTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & d.jTE10 & Not(d.jTE200)   & physcond)
        MenuItem('L1_1ZDC_NZDC_jTE10_VjTE200' ).setLogic( PHYS_1ZDC_NZDC & d.jTE10 & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_XOR_jTE10_VjTE200' ).setLogic( ZDC_XOR & d.jTE10 & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_5XOR_TE5_VTE200'      ).setLogic( PHYS_ZDC_5XOR & d.TE5 & Not(d.TE200)    & physcond)
        MenuItem('L1_ZDC_5XOR_jTE5_VjTE200'      ).setLogic( PHYS_ZDC_5XOR & d.jTE5 & Not(d.jTE200)    & physcond)
        MenuItem('L1_ZDC_XOR4_TE5_VTE200'   ).setLogic( PHYS_ZDC_XOR4 & d.TE5 & Not(d.TE200)    & physcond)
        MenuItem('L1_ZDC_XOR4_jTE5_VjTE200'   ).setLogic( PHYS_ZDC_XOR4  & d.jTE5 & Not(d.jTE200)    & physcond)
        MenuItem('L1_ZDC_XOR_jTE5_VjTE200' ).setLogic(ZDC_XOR & d.jTE5 & Not(d.jTE200) & physcond)
        MenuItem('L1_ZDC_XOR_gTE5_VjTE200' ).setLogic(ZDC_XOR & d.gTE5 & Not(d.jTE200) & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_VTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_VjTE200' ).setLogic(  PHYS_VZDC_A_VZDC_C &  Not(d.jTE200)  & physcond)
        # ATR-30533
        MenuItem('L1_VZDC_A_VZDC_C_jJ5_VjTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & d.jJ5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_1ZDC_NZDC_jJ5_VjTE200' ).setLogic( PHYS_1ZDC_NZDC & d.jJ5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_XOR_jJ5_VjTE200' ).setLogic( ZDC_XOR & d.jJ5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_jJ10_VjTE200' ).setLogic( PHYS_VZDC_A_VZDC_C & d.jJ10 & Not(d.jTE200)   & physcond)
        MenuItem('L1_1ZDC_NZDC_jJ10_VjTE200' ).setLogic( PHYS_1ZDC_NZDC & d.jJ10 & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_XOR_jJ10_VjTE200' ).setLogic( ZDC_XOR & d.jJ10 & Not(d.jTE200)   & physcond)

        # ATR-30727
        MenuItem('L1_eTAU1_jJ5_VjTE200' ).setLogic( d.eTAU1 & d.jJ5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_jJ5_TRT_VjTE200' ).setLogic( d.jJ5 & d.NIMTRT & Not(d.jTE200)   & physcond)
        MenuItem('L1_eTAU1_jJ5_TRT_VjTE200' ).setLogic( d.eTAU1 & d.jJ5 & d.NIMTRT & Not(d.jTE200)   & physcond)
        MenuItem('L1_2jJ5_VjTE200' ).setLogic( d.jJ5.x(2) & Not(d.jTE200)   & physcond)
        MenuItem('L1_2jJ5_TRT_VjTE200' ).setLogic( d.jJ5.x(2) & d.NIMTRT & Not(d.jTE200)   & physcond)
        MenuItem('L1_jJ5p30ETA49_VjTE200' ).setLogic( d.jJ530ETA49 &  Not(d.jTE200)   & physcond)
        MenuItem('L1_jJ10p30ETA49_VjTE200' ).setLogic( d.jJ1030ETA49 &  Not(d.jTE200)   & physcond)
        MenuItem('L1_2jJ5p30ETA49_VjTE200' ).setLogic( d.jJ530ETA49.x(2) & Not(d.jTE200)   & physcond)
        MenuItem('L1_2jJ10p30ETA49_VjTE200' ).setLogic( d.jJ1030ETA49.x(2) & Not(d.jTE200)   & physcond)

        MenuItem('L1_VZDC_A_VZDC_C_TE5_VTE200_UNPAIRED_ISO' ).setLogic( PHYS_VZDC_A_VZDC_C & d.TE5 & Not(d.TE200)   & unpaired_isocond)
        MenuItem('L1_VZDC_A_VZDC_C_jTE5_VjTE200_UNPAIRED_ISO' ).setLogic( PHYS_VZDC_A_VZDC_C & d.jTE5 & Not(d.jTE200)   & unpaired_isocond)
        MenuItem('L1_VZDC_A_VZDC_C_gTE5_VjTE200_UNPAIRED_ISO' ).setLogic( PHYS_VZDC_A_VZDC_C & d.gTE5 & Not(d.jTE200)   & unpaired_isocond)
        MenuItem('L1_ZDC_XOR_TE5_VTE200_UNPAIRED_ISO' ).setLogic(ZDC_XOR & d.TE5 & Not(d.TE200) & unpaired_isocond)
        MenuItem('L1_ZDC_XOR_jTE5_VjTE200_UNPAIRED_ISO' ).setLogic(ZDC_XOR & d.jTE5 & Not(d.jTE200) & unpaired_isocond)
        MenuItem('L1_ZDC_XOR_gTE5_VjTE200_UNPAIRED_ISO' ).setLogic(ZDC_XOR & d.gTE5 & Not(d.jTE200) & unpaired_isocond)
        MenuItem('L1_5ZDC_A_5ZDC_C_TE5_VTE200'  ).setLogic( PHYS_5ZDC_A_5ZDC_C  & d.TE5 & Not(d.TE200)   & physcond)
        MenuItem('L1_5ZDC_A_5ZDC_C_jTE5_VjTE200'  ).setLogic( PHYS_5ZDC_A_5ZDC_C  & d.jTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_5ZDC_A_5ZDC_C_gTE5_VjTE200'  ).setLogic( PHYS_5ZDC_A_5ZDC_C  & d.gTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_ZDC_XOR_TE5' ).setLogic(ZDC_XOR & d.TE5  & physcond)
        MenuItem('L1_ZDC_XOR_jTE5' ).setLogic(ZDC_XOR & d.jTE5  & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_TE5' ).setLogic( PHYS_VZDC_A_VZDC_C & d.TE5   & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_jTE5' ).setLogic(  PHYS_VZDC_A_VZDC_C &  d.jTE5  & physcond)
        MenuItem('L1_1ZDC_NZDC_TE5_VTE200'      ).setLogic( PHYS_1ZDC_NZDC & d.TE5 & Not(d.TE200)    & physcond)
        MenuItem('L1_1ZDC_NZDC_jTE5_VjTE200'      ).setLogic( PHYS_1ZDC_NZDC & d.jTE5 & Not(d.jTE200)    & physcond)
        MenuItem('L1_1ZDC_NZDC_gTE5_VjTE200'      ).setLogic( PHYS_1ZDC_NZDC & d.gTE5 & Not(d.jTE200)    & physcond)

        MenuItem('L1_ZDC_A_C_VTE10'         ).setLogic( ZDC_A_C & Not(d.TE10) & physcond)
        MenuItem('L1_ZDC_A_C_VjTE10'         ).setLogic( ZDC_A_C & Not(d.jTE10) & physcond)
        MenuItem('L1_ZDC_A_C_VTE10_UNPAIRED_ISO'         ).setLogic( ZDC_A_C & Not(d.TE10) & unpaired_isocond)
        MenuItem('L1_ZDC_A_C_VjTE10_UNPAIRED_ISO'         ).setLogic( ZDC_A_C & Not(d.jTE10) & unpaired_isocond)
        MenuItem('L1_ZDC_A_C_VTE10_UNPAIRED_NONISO'         ).setLogic( ZDC_A_C & Not(d.TE10) & unpaired_nonisocond)
        MenuItem('L1_ZDC_A_C_VjTE10_UNPAIRED_NONISO'         ).setLogic( ZDC_A_C & Not(d.jTE10) & unpaired_nonisocond)
        MenuItem('L1_ZDC_A_C_VTE10_EMPTY'         ).setLogic( ZDC_A_C & Not(d.TE10) & cosmiccond)
        MenuItem('L1_ZDC_A_C_VjTE10_EMPTY'         ).setLogic( ZDC_A_C & Not(d.jTE10) & cosmiccond)
        MenuItem('L1_ZDC_XOR_VTE10' ).setLogic(ZDC_XOR  & Not(d.TE10) & physcond)
        MenuItem('L1_ZDC_XOR_VjTE10' ).setLogic(ZDC_XOR  & Not(d.jTE10) & physcond)
        MenuItem('L1_ZDC_XOR_VjTE10_UNPAIRED_NONISO' ).setLogic(ZDC_XOR  & Not(d.jTE10) & unpaired_nonisocond)
        MenuItem('L1_ZDC_OR_VTE200_UNPAIRED_ISO'         ).setLogic( ZDCOR & Not(d.TE200) & unpaired_isocond)
        MenuItem('L1_MBTS_1_ZDC_OR_VTE200_UNPAIRED_ISO'         ).setLogic( MBTS_1 & ZDCOR & Not(d.TE200) & unpaired_isocond)
        MenuItem('L1_ZDC_OR_VjTE200_UNPAIRED_ISO'         ).setLogic( ZDCOR & Not(d.jTE200) & unpaired_isocond)
        MenuItem('L1_MBTS_1_ZDC_OR_VjTE200_UNPAIRED_ISO'         ).setLogic( MBTS_1 & ZDCOR & Not(d.jTE200) & unpaired_isocond)
        MenuItem('L1_TRT_ZDC_A_C_VjTE10'         ).setLogic(d.NIMTRT & ZDC_A_C & Not(d.jTE10) & physcond)
        MenuItem('L1_TRT_ZDC_XOR_VjTE10' ).setLogic(d.NIMTRT & ZDC_XOR  & Not(d.jTE10) & physcond)
        MenuItem('L1_TRT_ZDC_XOR_VjTE10_UNPAIRED_NONISO'  ).setLogic(d.NIMTRT & ZDC_XOR & Not(d.jTE10) & unpaired_nonisocond)
        MenuItem('L1_VZDC_A_VZDC_C_VjTE50').setLogic(PHYS_VZDC_A_VZDC_C & Not(d.jTE50) & physcond)
        MenuItem('L1_ZDC_A_VjTE200').setLogic(ZDC_A & Not(d.jTE200) & physcond)
        MenuItem('L1_ZDC_C_VjTE200').setLogic(ZDC_C & Not(d.jTE200) & physcond)
        MenuItem('L1_TRT_ZDC_A_VjTE50').setLogic(d.NIMTRT & ZDC_A & Not(d.jTE50) & physcond)
        MenuItem('L1_TRT_ZDC_C_VjTE50').setLogic(d.NIMTRT & ZDC_C & Not(d.jTE50) & physcond)

        MenuItem('L1_TAU1_VZDC_A_VZDC_C_VTE100' ).setLogic( d.HA1 & PHYS_VZDC_A_VZDC_C & Not(d.TE100)   & physcond)
        MenuItem('L1_TAU1_ZDC_XOR4_VTE100' ).setLogic( d.HA1 & PHYS_ZDC_XOR4 & Not(d.TE100)   & physcond)
        MenuItem('L1_eEM1_VZDC_A_VZDC_C_VjTE100' ).setLogic( d.eEM1 & PHYS_VZDC_A_VZDC_C & Not(d.jTE100)  & physcond)
        MenuItem('L1_eEM1_ZDC_XOR4_VjTE100' ).setLogic( d.eEM1 & PHYS_ZDC_XOR4 & Not(d.jTE100)   & physcond)
        MenuItem('L1_eEM1_VZDC_A_VZDC_C_VjTE100_GAP_AANDC' ).setLogic( d.eEM1 & PHYS_VZDC_A_VZDC_C & Not(d.jTE100) & GAPAC   & physcond)
        MenuItem('L1_eEM1_ZDC_XOR4_VjTE100_GAP_AANDC' ).setLogic( d.eEM1 & PHYS_ZDC_XOR4 & Not(d.jTE100) & GAPAC   & physcond)
        MenuItem('L1_TAU2_VZDC_A_VZDC_C_VTE100' ).setLogic( d.HA2 & PHYS_VZDC_A_VZDC_C & Not(d.TE100)   & physcond)
        MenuItem('L1_TAU2_ZDC_XOR4_VTE100' ).setLogic( d.HA2 & PHYS_ZDC_XOR4 & Not(d.TE100)   & physcond)
        MenuItem('L1_eEM2_VZDC_A_VZDC_C_VjTE100' ).setLogic( d.eEM2 & PHYS_VZDC_A_VZDC_C & Not(d.jTE100)  & physcond)
        MenuItem('L1_eEM2_ZDC_XOR4_VjTE100' ).setLogic( d.eEM2 & PHYS_ZDC_XOR4 & Not(d.jTE100) &  physcond)
        MenuItem('L1_eEM2_VZDC_A_VZDC_C_VjTE100_GAP_AANDC' ).setLogic( d.eEM2 & PHYS_VZDC_A_VZDC_C & Not(d.jTE100) & GAPAC  & physcond)
        MenuItem('L1_eEM2_ZDC_XOR4_VjTE100_GAP_AANDC' ).setLogic( d.eEM2 & PHYS_ZDC_XOR4 & Not(d.jTE100) & GAPAC  & physcond)
        MenuItem('L1_TAU1_TRT_VZDC_A_VZDC_C_VTE100' ).setLogic( d.HA1 & d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.TE100)   & physcond)
        MenuItem('L1_TAU1_TRT_ZDC_XOR4_VTE100' ).setLogic( d.HA1 & d.NIMTRT & PHYS_ZDC_XOR4 & Not(d.TE100)   & physcond)
        MenuItem('L1_eTAU1_TRT_VZDC_A_VZDC_C_VjTE100' ).setLogic( d.eTAU1 & d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.jTE100)   & physcond)
        MenuItem('L1_eTAU1_TRT_ZDC_XOR4_VjTE100' ).setLogic( d.eTAU1 & d.NIMTRT & PHYS_ZDC_XOR4 & Not(d.jTE100)   & physcond)
        MenuItem('L1_eEM1_TRT_VZDC_A_VZDC_C_VjTE100' ).setLogic( d.eEM1 & d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.jTE100)   & physcond)
        MenuItem('L1_eEM1_TRT_ZDC_XOR4_VjTE100' ).setLogic( d.eEM1 & d.NIMTRT & PHYS_ZDC_XOR4 & Not(d.jTE100)   & physcond)
        MenuItem('L1_TAU1_TRT_VTE50' ).setLogic( d.HA1 & d.NIMTRT & Not(d.TE50)   & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_VTE50' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.TE50)   & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_VTE20' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.TE20)   & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_VTE200' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.TE200)   & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_VjTE50' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.jTE50) & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_VjTE20' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.jTE20)  & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_VjTE50_GAP_AANDC' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.jTE50) & GAPAC  & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_VjTE20_GAP_AANDC' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & Not(d.jTE20) & GAPAC  & physcond)
        MenuItem('L1_TRT_VZDC_A_VZDC_C_jTE5_VjTE200' ).setLogic( d.NIMTRT & PHYS_VZDC_A_VZDC_C & d.jTE5 & Not(d.jTE200)   & physcond)
        MenuItem('L1_eEM1_TRT_VjTE100' ).setLogic( d.eEM1 & d.NIMTRT & Not(d.jTE100)   & physcond)
        MenuItem('L1_eEM1_TRT_VjTE200' ).setLogic( d.eEM1 & d.NIMTRT & Not(d.jTE200)   & physcond)
        MenuItem('L1_eTAU1_TRT_VjTE100' ).setLogic( d.eTAU1 & d.NIMTRT & Not(d.jTE100)   & physcond)
        MenuItem('L1_eTAU1_TRT_VjTE200' ).setLogic( d.eTAU1 & d.NIMTRT & Not(d.jTE200)   & physcond)
        MenuItem('L1_jTAU1_TRT_VjTE100' ).setLogic( d.jTAU1 & d.NIMTRT & Not(d.jTE100)   & physcond)
        MenuItem('L1_eEM1_TRT_VjTE100_GAP_AANDC' ).setLogic( d.eEM1 & d.NIMTRT & Not(d.jTE100) & GAPAC  & physcond)
        MenuItem('L1_eTAU1_TRT_VjTE100_GAP_AANDC' ).setLogic( d.eTAU1 & d.NIMTRT & Not(d.jTE100) & GAPAC  & physcond)
        MenuItem('L1_jTAU1_TRT_VjTE100_GAP_AANDC' ).setLogic( d.jTAU1 & d.NIMTRT & Not(d.jTE100) & GAPAC  & physcond)
        MenuItem('L1_eEM1_TRT_VjTE200_GAP_AANDC' ).setLogic( d.eEM1 & d.NIMTRT & Not(d.jTE200) & GAPAC  & physcond)
        MenuItem('L1_eTAU1_TRT_VjTE200_GAP_AANDC' ).setLogic( d.eTAU1 & d.NIMTRT & Not(d.jTE200) & GAPAC  & physcond)
                
        MenuItem('L1_1ZDC_A_1ZDC_C_VTE50' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.TE50)  & physcond)
        MenuItem('L1_1ZDC_A_1ZDC_C_VjTE50_GAP_AANDC' ).setLogic( PHYS_1TO4ZDC_A_1TO4ZDC_C & Not(d.jTE50) & GAPAC  & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_VTE50' ).setLogic( PHYS_VZDC_A_VZDC_C & Not(d.TE50)  & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_VjTE50_GAP_AANDC' ).setLogic( PHYS_VZDC_A_VZDC_C & Not(d.jTE50) & GAPAC  & physcond)
        MenuItem('L1_ZDC_XOR4_VTE50' ).setLogic( PHYS_ZDC_XOR4 & Not(d.TE50)  & physcond)
        MenuItem('L1_ZDC_XOR4_VjTE50_GAP_AANDC' ).setLogic( PHYS_ZDC_XOR4 & Not(d.jTE50) & GAPAC  & physcond)
        MenuItem('L1_ZDC_XOR4_VTE200' ).setLogic( PHYS_ZDC_XOR4 & Not(d.TE200)  & physcond)
        MenuItem('L1_ZDC_XOR4_VjTE200_GAP_AANDC' ).setLogic( PHYS_ZDC_XOR4 & Not(d.jTE200) & GAPAC  & physcond)
        MenuItem('L1_VZDC_A_VZDC_C_VjTE200_GAP_AANDC' ).setLogic(  PHYS_VZDC_A_VZDC_C  &  Not(d.jTE200) & GAPAC  & physcond)

        #ATR-30208
        MenuItem('L1_TRT_ZDC_XOR_VjTE200' ).setLogic( d.NIMTRT & ZDC_XOR & Not(d.jTE200)   & physcond)
        MenuItem('L1_eEM1_TRT_ZDC_XOR_VjTE200' ).setLogic( d.eEM1 & d.NIMTRT & ZDC_XOR & Not(d.jTE200)   & physcond)
        MenuItem('L1_eTAU1_TRT_ZDC_XOR_VjTE200' ).setLogic( d.eTAU1 & d.NIMTRT & ZDC_XOR & Not(d.jTE200)   & physcond)
        MenuItem('L1_jTAU1_TRT_ZDC_XOR_VjTE200' ).setLogic( d.jTAU1 & d.NIMTRT & ZDC_XOR & Not(d.jTE200)   & physcond)
        MenuItem('L1_TRT_1ZDC_NZDC_VjTE200' ).setLogic( d.NIMTRT & PHYS_1ZDC_NZDC & Not(d.jTE200)   & physcond)
        # ATR-30471
        MenuItem('L1_TRT_ZDC_XOR_jTE5_VjTE200' ).setLogic( d.NIMTRT & ZDC_XOR & d.jTE5 & Not(d.jTE200)   & physcond)

        #ATR-30020
        MenuItem('L1_ZDC_HELT15_jTE4000').setLogic( ZDC_HELT15 & d.jTE4000 & physcond )
        MenuItem('L1_ZDC_HELT20_jTE4000').setLogic( ZDC_HELT20 & d.jTE4000 & physcond )
        MenuItem('L1_ZDC_HELT25_jTE4000').setLogic( ZDC_HELT25 & d.jTE4000 & physcond )
        MenuItem('L1_ZDC_HELT35_jTE4000').setLogic( ZDC_HELT35 & d.jTE4000 & physcond )
        MenuItem('L1_ZDC_HELT50_jTE4000').setLogic( ZDC_HELT50 & d.jTE4000 & physcond )

        # ATR-14967
        MenuItem('L1_TE5_VZDC_A'           ).setLogic( d.TE5 & Not(ZDC_A) & physcond)
        MenuItem('L1_TE5_VZDC_C'           ).setLogic( d.TE5 & Not(ZDC_C) & physcond)
        MenuItem('L1_TE10_VZDC_A'          ).setLogic( d.TE10 & Not(ZDC_A) & physcond)
        MenuItem('L1_TE10_VZDC_C'          ).setLogic( d.TE10 & Not(ZDC_C) & physcond)
        MenuItem('L1_TE20_VZDC_A'          ).setLogic( d.TE20 & Not(ZDC_A) & physcond)
        MenuItem('L1_TE20_VZDC_C'          ).setLogic( d.TE20 & Not(ZDC_C) & physcond)
        MenuItem('L1_VTE10_VZDC_A'         ).setLogic( Not(d.TE10) & Not(ZDC_A) & physcond)
        MenuItem('L1_VTE10_VZDC_C'         ).setLogic( Not(d.TE10) & Not(ZDC_C) & physcond)
        MenuItem('L1_J5_VZDC_A'            ).setLogic( d.J5  & Not(ZDC_A) & physcond)
        MenuItem('L1_J5_VZDC_C'            ).setLogic( d.J5  & Not(ZDC_C) & physcond)
        MenuItem('L1_J10_VZDC_A'           ).setLogic( d.J10 & Not(ZDC_A) & physcond)
        MenuItem('L1_J10_VZDC_C'           ).setLogic( d.J10 & Not(ZDC_C) & physcond)
        MenuItem('L1_J15_VZDC_A'           ).setLogic( d.J15 & Not(ZDC_A) & physcond)
        MenuItem('L1_J15_VZDC_C'           ).setLogic( d.J15 & Not(ZDC_C) & physcond)
        MenuItem('L1_J20_VZDC_A'           ).setLogic( d.J20 & Not(ZDC_A) & physcond)
        MenuItem('L1_J20_VZDC_C'           ).setLogic( d.J20 & Not(ZDC_C) & physcond)
        MenuItem('L1_MU3V_VZDC_A'          ).setLogic( d.MU3V & Not(ZDC_A) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_VZDC_C'          ).setLogic( d.MU3V & Not(ZDC_C) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_VZDC_A'         ).setLogic( d.MU3V.x(2) & Not(ZDC_A) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_VZDC_C'         ).setLogic( d.MU3V.x(2) & Not(ZDC_C) & physcond).setTriggerType(TT.muon)

        MenuItem('L1_MU3V_VTE10_VZDC_A' ).setLogic( d.MU3V      & Not(d.TE10) & Not(ZDC_A) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_VTE10_VZDC_C' ).setLogic( d.MU3V      & Not(d.TE10) & Not(ZDC_C) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_VTE10_VZDC_A').setLogic( d.MU3V.x(2) & Not(d.TE10) & Not(ZDC_A) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_2MU3V_VTE10_VZDC_C').setLogic( d.MU3V.x(2) & Not(d.TE10) & Not(ZDC_C) & physcond).setTriggerType(TT.muon)

        MenuItem('L1_MU3V_VZDC_A_C'         ).setLogic( d.MU3V  & VZDC_A_C & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU3V_VZDC_AORC_VTE200' ).setLogic( d.MU3V  & VZDC_AORC & Not(d.TE200) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_VZDC_A'          ).setLogic( d.MU5VF & Not(ZDC_A) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_VZDC_C'          ).setLogic( d.MU5VF & Not(ZDC_C) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_VTE10_VZDC_A'    ).setLogic( d.MU5VF & Not(d.TE10) & Not(ZDC_A) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_VTE10_VZDC_C'    ).setLogic( d.MU5VF & Not(d.TE10) & Not(ZDC_C) & physcond).setTriggerType(TT.muon)
        MenuItem('L1_MU5VF_VZDC_A_C'        ).setLogic( d.MU5VF & VZDC_A_C & physcond).setTriggerType(TT.muon)
        MenuItem('L1_RD0_FILLED_VZDC_A'    ).setLogic( d.RNDM0 & Not(ZDC_A) & physcond)
        MenuItem('L1_RD0_FILLED_VZDC_C'    ).setLogic( d.RNDM0 & Not(ZDC_C) & physcond)
        MenuItem('L1_MBTS_4_C_VZDC_A'      ).setLogic( MBTS_4_C & Not(ZDC_A) & physcond)
        MenuItem('L1_MBTS_4_A_VZDC_C'      ).setLogic( MBTS_4_A & Not(ZDC_C) & physcond)
        MenuItem('L1_MBTS_2_C_VZDC_A'      ).setLogic( MBTS_2_C & Not(ZDC_A) & physcond)
        MenuItem('L1_MBTS_2_A_VZDC_C'      ).setLogic( MBTS_2_A & Not(ZDC_C) & physcond)




        # VDM
        MenuItem('L1_ZDC_A_C_BGRP7'     ).setLogic( ZDC_A_C & bgrp7cond)
        MenuItem('L1_LUCID_BGRP7'       ).setLogic( (d.LUCID_A | d.LUCID_C) & bgrp7cond)
        MenuItem('L1_TRT_BGRP11'        ).setLogic( d.NIMTRT & bgrp11cond )

        # LUCID
        MenuItem('L1_LUCID_A'           ).setLogic( d.LUCID_A             & physcond)
        MenuItem('L1_LUCID_C'           ).setLogic( d.LUCID_C             & physcond)
        MenuItem('L1_LUCID_A_C',        ).setLogic( d.LUCID_A & d.LUCID_C   & physcond)
        MenuItem('L1_LUCID'             ).setLogic((d.LUCID_A | d.LUCID_C)  & physcond)
        MenuItem('L1_LUCID_COMM'        ).setLogic( d.LUCID_COMM          & physcond)
        MenuItem('L1_LUCID_COMM_EMPTY'  ).setLogic( d.LUCID_COMM          & cosmiccond)
        MenuItem('L1_LUCID_EMPTY'       ).setLogic((d.LUCID_A | d.LUCID_C)  & cosmiccond)
        MenuItem('L1_LUCID_A_C_EMPTY'   ).setLogic( d.LUCID_A & d.LUCID_C   & cosmiccond)
        MenuItem('L1_LUCID_UNPAIRED_ISO').setLogic((d.LUCID_A | d.LUCID_C)  & unpaired_isocond)
        MenuItem('L1_LUCID_COMM_UNPAIRED_ISO' ).setLogic( d.LUCID_COMM        & unpaired_isocond )
        MenuItem('L1_LUCID_A_C_UNPAIRED_ISO'  ).setLogic( d.LUCID_A & d.LUCID_C & unpaired_isocond)
        MenuItem('L1_LUCID_A_C_UNPAIRED_NONISO').setLogic(d.LUCID_A & d.LUCID_C & unpaired_nonisocond)

        MenuItem('L1_LUCID_BGRP9').setLogic( (d.LUCID_A | d.LUCID_C) & bgrp9cond)
        MenuItem('L1_LUCID_BGRP11').setLogic( (d.LUCID_A | d.LUCID_C) & bgrp11cond)

        MenuItem('L1_LUCID_A_BGRP11').setLogic( d.LUCID_A & bgrp11cond)
        MenuItem('L1_LUCID_C_BGRP11').setLogic( d.LUCID_C & bgrp11cond)

        # BCM
        MenuItem('L1_BCM_Wide'                   ).setLogic( d.BCM_Wide & physcond )
        MenuItem('L1_BCM_Wide_BGRP12'            ).setLogic( d.BCM_Wide & bgrp12cond )
        MenuItem('L1_BCM_Wide_EMPTY'             ).setLogic( d.BCM_Wide & cosmiccond )
        MenuItem('L1_BCM_Wide_UNPAIRED_ISO'      ).setLogic( d.BCM_Wide & unpaired_isocond ) # no needed anymore
        MenuItem('L1_BCM_Wide_UNPAIRED_NONISO'   ).setLogic( d.BCM_Wide & unpaired_nonisocond ) # no needed anymore
        MenuItem('L1_BCM_Wide_UNPAIREDB1'        ).setLogic( d.BCM_Wide & bgrp13cond )
        MenuItem('L1_BCM_Wide_UNPAIREDB2'        ).setLogic( d.BCM_Wide & bgrp14cond )

        MenuItem('L1_BCM_HT_BGRP12'               ).setLogic( d.BCM_Comb.x(7) & bgrp12cond)
        MenuItem('L1_BCM_AC_CA_BGRP12'            ).setLogic((d.BCM_AtoC | d.BCM_CtoA) & bgrp12cond )
        MenuItem('L1_BCM_AC_CA_UNPAIRED_ISO'     ).setLogic((d.BCM_AtoC | d.BCM_CtoA) & unpaired_isocond)

        MenuItem('L1_BCM_AC_UNPAIRED_ISO'        ).setLogic( d.BCM_AtoC & unpaired_isocond) # no needed anymore
        MenuItem('L1_BCM_CA_UNPAIRED_ISO'        ).setLogic( d.BCM_CtoA & unpaired_isocond) # no needed anymore

        MenuItem('L1_BCM_AC_UNPAIRED_NONISO'     ).setLogic( d.BCM_AtoC & unpaired_nonisocond) # no needed anymore
        MenuItem('L1_BCM_CA_UNPAIRED_NONISO'     ).setLogic( d.BCM_CtoA & unpaired_nonisocond) # no needed anymore

        MenuItem('L1_BCM_AC_CALIB'     ).setLogic( d.BCM_AtoC & calibcond)
        MenuItem('L1_BCM_CA_CALIB'     ).setLogic( d.BCM_CtoA & calibcond)
        MenuItem('L1_BCM_Wide_CALIB'   ).setLogic( d.BCM_Wide & calibcond) # no needed anymore

        MenuItem('L1_BCM_AC_UNPAIREDB1'  ).setLogic( d.BCM_AtoC & bgrp13cond) # no needed anymore
        MenuItem('L1_BCM_CA_UNPAIREDB2'  ).setLogic( d.BCM_CtoA & bgrp14cond) # no needed anymore

        MenuItem('L1_BCM_2A_EMPTY' ).setLogic( d.BCM6 & cosmiccond)
        MenuItem('L1_BCM_2C_EMPTY' ).setLogic( d.BCM7 & cosmiccond)

        MenuItem('L1_BCM_2A_UNPAIREDB1' ).setLogic( d.BCM6 & bgrp13cond)
        MenuItem('L1_BCM_2C_UNPAIREDB1' ).setLogic( d.BCM7 & bgrp13cond)
        MenuItem('L1_BCM_2A_UNPAIREDB2' ).setLogic( d.BCM6 & bgrp14cond)
        MenuItem('L1_BCM_2C_UNPAIREDB2' ).setLogic( d.BCM7 & bgrp14cond)

        MenuItem('L1_BCM_2A_UNPAIRED_ISO' ).setLogic( d.BCM6 & unpaired_isocond)
        MenuItem('L1_BCM_2C_UNPAIRED_ISO' ).setLogic( d.BCM7 & unpaired_isocond)
        MenuItem('L1_BCM_2A_UNPAIRED_NONISO' ).setLogic( d.BCM6 & unpaired_nonisocond)
        MenuItem('L1_BCM_2C_UNPAIRED_NONISO' ).setLogic( d.BCM7 & unpaired_nonisocond)

        MenuItem('L1_BCM_2A_CALIB' ).setLogic( d.BCM6 & calibcond)
        MenuItem('L1_BCM_2C_CALIB' ).setLogic( d.BCM7 & calibcond)

        MenuItem('L1_BCM_2A_FIRSTINTRAIN' ).setLogic( d.BCM6 & firstintrain)
        MenuItem('L1_BCM_2C_FIRSTINTRAIN' ).setLogic( d.BCM7 & firstintrain)

        MenuItem('L1_BCM_2A_2C_BGRP12'       ).setLogic((d.BCM6 | d.BCM7) & bgrp12cond )
        MenuItem('L1_BCM_2A_2C_UNPAIRED_ISO' ).setLogic((d.BCM6 | d.BCM7) & unpaired_isocond)

        # RANDOM
        MenuItem('L1_RD0_FILLED'         ).setLogic( d.RNDM0 & physcond           ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_EMPTY'          ).setLogic( d.RNDM0 & cosmiccond         ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_UNPAIRED_ISO'   ).setLogic( d.RNDM0 & unpaired_isocond   ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_UNPAIRED_NONISO').setLogic( d.RNDM0 & unpaired_nonisocond).setTriggerType(TT.rand)
        MenuItem('L1_RD0_FIRSTEMPTY'     ).setLogic( d.RNDM0 & firstempty         ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_BGRP0'          ).setLogic( d.RNDM0 & d.BGRP0            ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_BGRP7'          ).setLogic( d.RNDM0 & bgrp7cond          ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_BGRP9'          ).setLogic( d.RNDM0 & bgrp9cond          ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_BGRP10'         ).setLogic( d.RNDM0 & bgrp10cond         ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_BGRP11'         ).setLogic( d.RNDM0 & bgrp11cond         ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_BGRP15'         ).setLogic( d.RNDM0 & d.BGRP0 & d.BGRP15 ).setTriggerType(TT.rand)
        MenuItem('L1_RD0_FIRSTINTRAIN'   ).setLogic( d.RNDM0 & firstintrain       ).setTriggerType(TT.rand)

        MenuItem('L1_RD1_FILLED'         ).setLogic( d.RNDM1 & physcond           ).setTriggerType(TT.zerobs) # used to be TT.rand
        MenuItem('L1_RD1_EMPTY'          ).setLogic( d.RNDM1 & cosmiccond         ).setTriggerType(TT.zerobs)
        MenuItem('L1_RD1_BGRP10'         ).setLogic( d.RNDM1 & bgrp10cond         ).setTriggerType(TT.zerobs)

        MenuItem('L1_RD2_FILLED'         ).setLogic( d.RNDM2 & physcond           ).setTriggerType(TT.rand)
        MenuItem('L1_RD2_EMPTY'          ).setLogic( d.RNDM2 & cosmiccond         ).setTriggerType(TT.rand)
        MenuItem('L1_RD2_BGRP11'         ).setLogic( d.RNDM2 & bgrp11cond         ).setTriggerType(TT.rand)
        MenuItem('L1_RD2_BGRP12'         ).setLogic( d.RNDM2 & bgrp12cond         ).setTriggerType(TT.rand)

        MenuItem('L1_RD3_FILLED'         ).setLogic( d.RNDM3 & physcond           ).setTriggerType(TT.rand)
        MenuItem('L1_RD3_EMPTY'          ).setLogic( d.RNDM3 & cosmiccond         ).setTriggerType(TT.rand)

        MenuItem('L1_BPTX0_BGRP12', ctpid=0xf1).setLogic(d.BPTX0 & bgrp12cond ).setTriggerType(TT.rand)
        MenuItem('L1_BPTX1_BGRP12', ctpid=0xf2).setLogic(d.BPTX1 & bgrp12cond ).setTriggerType(TT.rand)

        MenuItem('L1_RD0_LAR_FILLED'         ).setLogic( d.RNDM0 & physcond ).setTriggerType(TT.lardigital)
        MenuItem('L1_RD0_LAR_EMPTY'          ).setLogic( d.RNDM0 & cosmiccond ).setTriggerType(TT.lardigital)
        MenuItem('L1_RD0_LAR_FIRSTEMPTY'     ).setLogic( d.RNDM0 & firstempty ).setTriggerType(TT.lardigital)
        MenuItem('L1_RD0_LAR_BGRP7'          ).setLogic( d.RNDM0 & bgrp7cond ).setTriggerType(TT.lardigital)

        # lumi measurements
        MenuItem('L1_MLZ_A').setLogic( (d.MBTS_A|ZDC_A|d.LUCID_A) & physcond)
        MenuItem('L1_MLZ_C').setLogic( (d.MBTS_C|ZDC_C|d.LUCID_C) & physcond)
        MenuItem('L1_MBLZ' ).setLogic( ( (d.MBTS_A|ZDC_A|d.LUCID_A) & (d.MBTS_C|ZDC_C|d.LUCID_C) | d.BCM_Wide) & physcond )

        MenuItem('L1_CALREQ0', ctpid=0x1fd).setLogic( d.CAL0 & calibcond).setTriggerType(TT.calreq0)
        MenuItem('L1_CALREQ1', ctpid=0x1fe).setLogic( d.CAL1 & calibcond).setTriggerType(TT.calreq1)
        MenuItem('L1_CALREQ2', ctpid=0x1ff).setLogic( d.CAL2 & calibcond).setTriggerType(TT.calreq2)

        # TRT
        MenuItem('L1_TRT'       , ctpid=0x4e).setLogic(d.NIMTRT & d.BGRP0).setTriggerType(TT.nim)
        MenuItem('L1_TRT_FILLED').setLogic(d.NIMTRT & physcond).setTriggerType(TT.nim)
        MenuItem('L1_TRT_EMPTY' ).setLogic(d.NIMTRT & cosmiccond).setTriggerType(TT.nim)
        MenuItem('L1_TRT_VTE200').setLogic(d.NIMTRT & Not(d.TE200) & physcond)
        MenuItem('L1_TRT_VTE50' ).setLogic(d.NIMTRT & Not(d.TE50)  & physcond)
        MenuItem('L1_TRT_VTE20' ).setLogic(d.NIMTRT & Not(d.TE20)  & physcond)

        #TRT + Phase-1 Calo
        MenuItem('L1_TRT_VjTE20' ).setLogic(d.NIMTRT & Not(d.jTE20)  & physcond)
        MenuItem('L1_TRT_VjTE50' ).setLogic(d.NIMTRT & Not(d.jTE50)  & physcond)
        MenuItem('L1_TRT_VjTE200' ).setLogic(d.NIMTRT & Not(d.jTE200)  & physcond)
        MenuItem('L1_TRT_VjTE50_GAP_AANDC' ).setLogic(d.NIMTRT & Not(d.jTE50) & GAPAC  & physcond)
        MenuItem('L1_TRT_VjTE200_GAP_AANDC' ).setLogic(d.NIMTRT & Not(d.jTE200) & GAPAC  & physcond)

        # TGC
        MenuItem('L1_TGC_BURST').setLogic(d.NIMTGC & bgrp12cond ).setTriggerType(TT.nim)

        # LHCF
        MenuItem('L1_LHCF').setLogic( d.NIMLHCF & physcond).setTriggerType(TT.nim)
        MenuItem('L1_LHCF_UNPAIRED_ISO').setLogic( d.NIMLHCF & unpaired_isocond).setTriggerType(TT.nim)
        MenuItem('L1_LHCF_EMPTY').setLogic( d.NIMLHCF & cosmiccond).setTriggerType(TT.nim)

        MBTS_INNER = (d.MBTS_A0 | d.MBTS_A1 |  d.MBTS_A2 | d.MBTS_A3 | d.MBTS_A4 | d.MBTS_A5 | d.MBTS_A6 | d.MBTS_A7 | d.MBTS_C0 | d.MBTS_C1 | d.MBTS_C2 | d.MBTS_C3 | d.MBTS_C4 | d.MBTS_C5 | d.MBTS_C6 | d.MBTS_C7)

        #AFP (ATR-23476)
        AFP_A = (d.AFP_NSA & d.AFP_FSA)
        AFP_C = (d.AFP_NSC & d.AFP_FSC)
        AFP_TOF_A = (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1 | d.AFP_FSA_TOF_T2 | d.AFP_FSA_TOF_T3)
        AFP_TOF_C = (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1 | d.AFP_FSC_TOF_T2 | d.AFP_FSC_TOF_T3)
        MenuItem('L1_AFP_NSA_BGRP12').setLogic( d.AFP_NSA & bgrp12cond)
        MenuItem('L1_AFP_NSC_BGRP12').setLogic( d.AFP_NSC & bgrp12cond)
        MenuItem('L1_AFP_FSA_BGRP12').setLogic( d.AFP_FSA & bgrp12cond)
        MenuItem('L1_AFP_FSC_BGRP12').setLogic( d.AFP_FSC & bgrp12cond)
        MenuItem('L1_AFP_FSA_TOF_T0_BGRP12').setLogic( d.AFP_FSA_TOF_T0 & bgrp12cond)
        MenuItem('L1_AFP_FSC_TOF_T0_BGRP12').setLogic( d.AFP_FSC_TOF_T0 & bgrp12cond)
        MenuItem('L1_AFP_FSA_TOF_T1_BGRP12').setLogic( d.AFP_FSA_TOF_T1 & bgrp12cond)
        MenuItem('L1_AFP_FSC_TOF_T1_BGRP12').setLogic( d.AFP_FSC_TOF_T1 & bgrp12cond)
        MenuItem('L1_AFP_FSA_TOF_T2_BGRP12').setLogic( d.AFP_FSA_TOF_T2 & bgrp12cond)
        MenuItem('L1_AFP_FSC_TOF_T2_BGRP12').setLogic( d.AFP_FSC_TOF_T2 & bgrp12cond)
        MenuItem('L1_AFP_FSA_TOF_T3_BGRP12').setLogic( d.AFP_FSA_TOF_T3 & bgrp12cond)
        MenuItem('L1_AFP_FSC_TOF_T3_BGRP12').setLogic( d.AFP_FSC_TOF_T3 & bgrp12cond)

        MenuItem('L1_AFP_A').setLogic( AFP_A & physcond)
        MenuItem('L1_AFP_C').setLogic( AFP_C & physcond)
        MenuItem('L1_AFP_A_OR_C').setLogic( (AFP_A | AFP_C) & physcond)
        MenuItem('L1_AFP_A_AND_C').setLogic( AFP_A & AFP_C & physcond)
        MenuItem('L1_AFP_A_AND_C_TOF').setLogic( AFP_TOF_A & AFP_TOF_C & physcond)
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & physcond)

        MenuItem('L1_AFP_A_OR_C_UNPAIRED_ISO').setLogic( (AFP_A | AFP_C) & unpaired_isocond )
        MenuItem('L1_AFP_A_OR_C_UNPAIRED_NONISO').setLogic( (AFP_A | AFP_C) & unpaired_nonisocond )
        MenuItem('L1_AFP_A_OR_C_EMPTY').setLogic( (AFP_A | AFP_C) & cosmiccond)
        MenuItem('L1_AFP_A_OR_C_FIRSTEMPTY').setLogic( (AFP_A | AFP_C) & firstempty)
        MenuItem('L1_AFP_A_OR_C_TOF_UNPAIRED_ISO').setLogic( (AFP_TOF_A | AFP_TOF_C) & unpaired_isocond )
        MenuItem('L1_AFP_A_OR_C_TOF_UNPAIRED_NONISO').setLogic( (AFP_TOF_A | AFP_TOF_C) & unpaired_nonisocond )
        MenuItem('L1_AFP_A_OR_C_TOF_EMPTY').setLogic( (AFP_TOF_A | AFP_TOF_C) & cosmiccond)
        MenuItem('L1_AFP_A_OR_C_TOF_FIRSTEMPTY').setLogic( (AFP_TOF_A | AFP_TOF_C) & firstempty)

        MenuItem('L1_AFP_A_AND_C_J5').setLogic( AFP_A & AFP_C & d.J5 & physcond )
        MenuItem('L1_AFP_A_OR_C_J5').setLogic( (AFP_A | AFP_C) & d.J5 & physcond )
        MenuItem('L1_AFP_A_AND_C_J12').setLogic( AFP_A & AFP_C & d.J12 & physcond )
        MenuItem('L1_AFP_A_OR_C_J12').setLogic( (AFP_A | AFP_C) & d.J12 & physcond )
        MenuItem('L1_AFP_A_AND_C_jJ20').setLogic( AFP_A & AFP_C & d.jJ20 & physcond )
        MenuItem('L1_AFP_A_OR_C_jJ20').setLogic( (AFP_A | AFP_C) & d.jJ20 & physcond )
        MenuItem('L1_AFP_A_AND_C_jJ30').setLogic( AFP_A & AFP_C & d.jJ30 & physcond )
        MenuItem('L1_AFP_A_OR_C_jJ30').setLogic( (AFP_A | AFP_C) & d.jJ30 & physcond )
        MenuItem('L1_MU5VF_AFP_A_OR_C').setLogic( (AFP_A | AFP_C) & d.MU5VF & physcond )
        MenuItem('L1_MU5VF_AFP_A_AND_C').setLogic( AFP_A & AFP_C & d.MU5VF & physcond )
        # MenuItem('L1_EM7_AFP_A_OR_C').setLogic( (AFP_A | AFP_C) & d.EM7 & physcond )
        # MenuItem('L1_EM7_AFP_A_AND_C').setLogic( AFP_A & AFP_C & d.EM7 & physcond )
        MenuItem('L1_eEM9_AFP_A_OR_C').setLogic( (AFP_A | AFP_C) & d.eEM9 & physcond )
        MenuItem('L1_eEM9_AFP_A_AND_C').setLogic( AFP_A & AFP_C & d.eEM9 & physcond )

        MenuItem('L1_AFP_A_AND_C_MBTS_2').setLogic( AFP_A & AFP_C & MBTS_2 & physcond )
        MenuItem('L1_AFP_A_OR_C_MBTS_2').setLogic( (AFP_A | AFP_C) & MBTS_2 & physcond )

        MenuItem('L1_AFP_A_AND_C_TOF_J20').setLogic( AFP_TOF_A & AFP_TOF_C & d.J20 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_J20').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.J20 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_J30').setLogic( AFP_TOF_A & AFP_TOF_C & d.J30 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_J30').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.J30 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_J50').setLogic( AFP_TOF_A & AFP_TOF_C & d.J50 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_J50').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.J50 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_J75').setLogic( AFP_TOF_A & AFP_TOF_C & d.J75 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_J75').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.J75 & physcond )

        MenuItem('L1_AFP_A_AND_C_TOF_jJ50').setLogic( AFP_TOF_A & AFP_TOF_C & d.jJ50 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_jJ50').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.jJ50 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_jJ60').setLogic( AFP_TOF_A & AFP_TOF_C & d.jJ60 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_jJ60').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.jJ60 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_jJ90').setLogic( AFP_TOF_A & AFP_TOF_C & d.jJ90 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_jJ90').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.jJ90 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_jJ125').setLogic( AFP_TOF_A & AFP_TOF_C & d.jJ125 & physcond )
        MenuItem('L1_AFP_A_AND_C_TOF_T0T1_jJ125').setLogic( (d.AFP_FSA_TOF_T0 | d.AFP_FSA_TOF_T1) & (d.AFP_FSC_TOF_T0 | d.AFP_FSC_TOF_T1) & d.jJ125 & physcond )

        try:

            # Legacy primary (items passed via the merger board):
            MenuItem('L1_HT190-J15s5pETA21').setLogic( d.R2TOPO_HT190_J15s5pETA21   & physcond)
            MenuItem('L1_BPH-0M9-EM7-EM5').setLogic( d.R2TOPO_0INVM9_EM7ab_EMab & physcond)
            MenuItem('L1_BPH-0DR3-EM7J15').setLogic( d.R2TOPO_0DR03_EM7ab_CJ15ab & physcond)
            MenuItem('L1_BPH-0M9-EM7-EM5_MU5VF').setLogic( d.MU5VF & d.R2TOPO_0INVM9_EM7ab_EMab & physcond)
            MenuItem('L1_BPH-0DR3-EM7J15_MU5VF').setLogic( d.MU5VF & d.R2TOPO_0DR03_EM7ab_CJ15ab & physcond)
            MenuItem('L1_BPH-0M9-EM7-EM5_2MU3V').setLogic( d.MU3V.x(2) & d.R2TOPO_0INVM9_EM7ab_EMab & physcond)
            MenuItem('L1_BPH-0DR3-EM7J15_2MU3V').setLogic( d.MU3V.x(2) & d.R2TOPO_0DR03_EM7ab_CJ15ab & physcond)
            MenuItem("L1_JPSI-1M5-EM7" ).setLogic( d.R2TOPO_1INVM5_EM7s1_EMs6  & physcond)
            MenuItem("L1_JPSI-1M5-EM12").setLogic( d.R2TOPO_1INVM5_EM12s1_EMs6 & physcond)
            MenuItem('L1_MJJ-500-NFF').setLogic( d.R2TOPO_500INVM9999_J30s6_AJ20s6 & physcond)
            MenuItem('L1_MJJ-700').setLogic( d.R2TOPO_700INVM9999_AJ30s6_AJ20s6 & physcond)
            MenuItem('L1_EM18VHI_MJJ-300').setLogic( d.EM18VHI & d.R2TOPO_300INVM9999_AJ30s6_AJ20s6 & physcond)
            MenuItem('L1_HT150-J20s5pETA31_MJJ-400-CF').setLogic( d.R2TOPO_HT150_J20s5pETA31 & d.R2TOPO_400INVM9999_AJ30s6pETA31_AJ20s6p31ETA49 & physcond)
            MenuItem('L1_30M-EM20ITAU12').setLogic( d.R2TOPO_DISAMB_30INVM_EM20his2_TAU12ab & physcond)
            MenuItem('L1_LLP-RO').setLogic( d.R2TOPO_100RATIO_0MATCH_TAU30si2_EMall & physcond)
            MenuItem('L1_LLP-NOMATCH').setLogic( d.R2TOPO_NOT_0MATCH_TAU30si1_EMall & physcond)
            MenuItem('L1_DPHI-2EM3').setLogic( d.R2TOPO_27DPHI32_EMs1_EMs6 & physcond)
            MenuItem('L1_SC111-CJ15').setLogic(  d.R2TOPO_SC111_CJ15abpETA26 & physcond)
            MenuItem('L1_J50_DETA20-J50J').setLogic( d.J50 & d.R2TOPO_0DETA20_J50s1_Js2 & physcond)
            MenuItem('L1_DR-TAU20ITAU12I-J25').setLogic( d.R2TOPO_1DISAMB_J25ab_0DR28_TAU20abi_TAU12abi & physcond)
            MenuItem('L1_TAU60_DR-TAU20ITAU12I' ).setLogic( d.HA60 & d.R2TOPO_0DR28_TAU20abi_TAU12abi & physcond)
            MenuItem('L1_DR-TAU20ITAU12I' ).setLogic( d.R2TOPO_0DR28_TAU20abi_TAU12abi & physcond)
            MenuItem('L1_LAR-ZEE').setLogic( d.R2TOPO_ZEE_EM20shi2 & physcond).setTriggerType( TT.lardigital ) # LAr demo (ATR-11897, ATR-23403)

            # Phase-1:
            MenuItem('L1_LAR-ZEE-eEM').setLogic( d.TOPO_ZEE_eEM24sm2 & physcond).setTriggerType( TT.lardigital ) # LAr demo (ATR-23403),
            MenuItem('L1_LATE-MU8F_jXE70').setLogic( d.TOPO_LATE_MU10s1 & d.jXE70 & physcond)
            MenuItem('L1_LATE-MU8F_jJ90' ).setLogic( d.TOPO_LATE_MU10s1 & d.jJ90 & physcond)
            MenuItem('L1_LFV-MU8VF').setLogic( d.TOPO_0DR15_2MU5VFab & d.MU8VF & d.MU5VF.x(2) & physcond)
            MenuItem('L1_LFV-MU5VF' ).setLogic( d.TOPO_0DR15_2MU5VFab & d.MU5VF.x(2) & physcond)
            MenuItem('L1_LFV-eEM10L-MU8VF' ).setLogic( d.TOPO_0INVM10_0DR15_eEM10abl_MU8Fab & d.MU8VF & physcond)
            MenuItem('L1_LFV-eEM15L-MU5VF' ).setLogic( d.TOPO_0INVM10_0DR15_eEM15abl_MU5VFab & physcond)

            #BLS
            #ATR-19720
            MenuItem('L1_BPH-2M9-0DR15-2MU3V'         ).setLogic( d.TOPO_2INVM9_0DR15_2MU3Vab & physcond) 
            MenuItem('L1_BPH-2M9-0DR15-2MU3VF'        ).setLogic( d.TOPO_2INVM9_0DR15_2MU3VFab & physcond) 
            MenuItem('L1_BPH-2M9-0DR15-MU5VFMU3V'     ).setLogic( d.TOPO_2INVM9_0DR15_MU5VFab_MU3Vab & physcond)
            MenuItem('L1_BPH-2M9-2DR15-2MU5VF'        ).setLogic( d.TOPO_2INVM9_2DR15_2MU5VFab & physcond) 
            MenuItem('L1_BPH-8M15-0DR22-MU5VFMU3V-BO' ).setLogic( d.TOPO_8INVM15_0DR22_CMU5VFab_CMU3Vab & physcond) 
            MenuItem('L1_BPH-8M15-0DR22-2MU5VF'       ).setLogic( d.TOPO_8INVM15_0DR22_2MU5VFab & physcond) 
            #ATR-19355
            MenuItem('L1_BPH-0M10-3MU3V'              ).setLogic( d.TOPO_0INVM10_3MU3Vab & physcond) 
            MenuItem('L1_BPH-0M10-3MU3VF'             ).setLogic( d.TOPO_0INVM10_3MU3VFab & physcond) 
            #ATR-19638
            MenuItem('L1_BPH-0M10C-3MU3V'             ).setLogic( d.TOPO_0INVM10C_3MU3Vab & physcond) 
            #ATR-19639 
            MenuItem('L1_BPH-2M9-0DR15-C-MU5VFMU3V'   ).setLogic( d.TOPO_2INVM9_0DR15_C_MU5VFab_MU3Vab & physcond) 

            # ATR-21566
            MenuItem('L1_BPH-7M22-2MU3VF'       ).setLogic( d.TOPO_7INVM22_2MU3VFab & physcond) 
            MenuItem('L1_BPH-7M22-MU5VFMU3VF'   ).setLogic( d.TOPO_7INVM22_MU5VFab_MU3VFab & physcond) 
            MenuItem('L1_BPH-7M22-0DR20-2MU3V'  ).setLogic( d.TOPO_7INVM22_0DR20_2MU3Vab & physcond) 
            MenuItem('L1_BPH-7M22-0DR20-2MU3VF' ).setLogic( d.TOPO_7INVM22_0DR20_2MU3VFab & physcond) 
            MenuItem('L1_BPH-7M22-0DR12-2MU3V'  ).setLogic( d.TOPO_7INVM22_0DR12_2MU3Vab & physcond) 

            #ATR-22782
            MenuItem('L1_BPH-7M11-25DR99-2MU3VF'   ).setLogic( d.TOPO_7INVM11_25DR99_2MU3VFab & physcond) 
            MenuItem('L1_BPH-7M14-MU5VFMU3VF'      ).setLogic( d.TOPO_7INVM14_MU5VFab_MU3VFab & physcond)
            MenuItem('L1_BPH-7M14-2MU3VF'          ).setLogic( d.TOPO_7INVM14_2MU3VFab & physcond) 
            MenuItem('L1_BPH-7M14-2MU3V'           ).setLogic( d.TOPO_7INVM14_2MU3Vab & physcond) 

            #ATR-24932
            MenuItem('L1_BPH-7M14-0DR25-MU5VFMU3VF'   ).setLogic( d.TOPO_7INVM14_0DR25_MU5VFab_MU3VFab & physcond)

            # ATR-19510, SM Low-mass DY
            MenuItem('L1_DY-BOX-2MU3VF').setLogic( d.TOPO_5DETA99_5DPHI99_2MU3VFab & physcond)
            MenuItem('L1_DY-BOX-MU5VFMU3V').setLogic( d.TOPO_5DETA99_5DPHI99_MU5VFab_MU3Vab & physcond) 
            MenuItem('L1_DY-BOX-2MU5VF').setLogic( d.TOPO_5DETA99_5DPHI99_2MU5VFab & physcond) 

            #ATR-18824
            MenuItem('L1_ZAFB-04DPHI-eEM18M' ).setLogic( d.TOPO_60INVM_04DPHI32_eEM18abm_jEM25s625ETA49 & physcond)
            MenuItem('L1_ZAFB-25DPHI-eEM18M' ).setLogic( d.TOPO_60INVM_25DPHI32_eEM18abm_jEM25s625ETA49 & physcond)
            MenuItem('L1_ZAFB-04DPHIM-eEM18M' ).setLogic( d.TOPO_60INVM_04DPHI32_eEM18abm_jEM20sm625ETA49 & physcond)
            MenuItem('L1_ZAFB-25DPHIM-eEM18M' ).setLogic( d.TOPO_60INVM_25DPHI32_eEM18abm_jEM20sm625ETA49 & physcond)
            #ATR-30822
            MenuItem('L1_eEM18M_jEM25' ).setLogic( d.eEM18M & d.jEM25 & physcond).setTriggerType( TT.calo )
            #ATR-19302:
            MenuItem('L1_DPHI-M70-2eEM12M' ).setLogic( d.TOPO_0INVM70_27DPHI32_eEM12sm1_eEM12sm6 & physcond)
            MenuItem('L1_DPHI-M70-2eEM15M' ).setLogic( d.TOPO_0INVM70_27DPHI32_eEM15sm1_eEM15sm6 & physcond)
            #ATR-21637
            MenuItem('L1_DPHI-M70-2eEM9' ).setLogic( d.TOPO_0INVM70_27DPHI32_eEM9s1_eEM9s6 & physcond)
            MenuItem('L1_DPHI-M70-2eEM9L' ).setLogic( d.TOPO_0INVM70_27DPHI32_eEM9sl1_eEM9sl6 & physcond)
            #ATR-27782 Boosted diPhoton
            MenuItem('L1_DPHI12-M70-2eEM9L').setLogic(d.TOPO_0INVM70_0DPHI12_eEM9sl1_eEM9sl6 & physcond)
            MenuItem('L1_DPHI12-M70-2eEM12L').setLogic(d.TOPO_0INVM70_0DPHI12_eEM12sl1_eEM12sl6 & physcond)
            MenuItem('L1_2DR15-M70-2eEM9L').setLogic(d.TOPO_0INVM70_2DR15_eEM9sl1_eEM9sl6 & physcond)
            MenuItem('L1_2DR15-M70-2eEM12L').setLogic(d.TOPO_0INVM70_2DR15_eEM12sl1_eEM12sl6 & physcond)
            # dr-m ranges
            MenuItem('L1_13DR25-25M70-2eEM12L').setLogic(d.TOPO_25INVM70_13DR25_eEM12sl1_eEM12sl6 & physcond)
            MenuItem('L1_2DR15-0M30-2eEM12L').setLogic(d.TOPO_0INVM30_2DR15_eEM12sl1_eEM12sl6 & physcond)
            # asymmetric et + ranges
            MenuItem('L1_13DR25-25M70-eEM12LeEM9L').setLogic(d.TOPO_25INVM70_13DR25_eEM12sl1_eEM9sl6 & physcond)
            MenuItem('L1_2DR15-0M30-eEM12LeEM9L').setLogic(d.TOPO_0INVM30_2DR15_eEM12sl1_eEM9sl6 & physcond)

            #ATR-19376
            MenuItem('L1_10DR-MU14FCH-MU5VF'             ).setLogic( d.TOPO_10DR99_2MU5VFab & d.MU14FCH & physcond) 
            MenuItem('L1_10DR-MU14FCH-MU5VF_EMPTY'       ).setLogic( d.TOPO_10DR99_2MU5VFab & d.MU14FCH & cosmiccond) 
            MenuItem('L1_10DR-MU14FCH-MU5VF_UNPAIRED_ISO').setLogic( d.TOPO_10DR99_2MU5VFab & d.MU14FCH & unpaired_isocond) 

            #Missing: KF
            # subset of legacy chains migrated phase1 boards
            MenuItem('L1_BPH-0M9-eEM9-eEM7').setLogic( d.TOPO_0INVM9_eEM9ab_eEMab & physcond)
            MenuItem('L1_BPH-0M9-eEM9-eEM7_MU5VF').setLogic( d.MU5VF & d.TOPO_0INVM9_eEM9ab_eEMab & physcond)
            MenuItem('L1_BPH-0M9-eEM9-eEM7_2MU3V').setLogic( d.MU3V.x(2) & d.TOPO_0INVM9_eEM9ab_eEMab & physcond) 
            MenuItem('L1_BPH-0DR3-eEM9jJ40').setLogic( d.TOPO_0DR03_eEM9ab_CjJ40ab & physcond)
            MenuItem('L1_BPH-0DR3-eEM9jJ40_MU5VF').setLogic(  d.MU5VF & d.TOPO_0DR03_eEM9ab_CjJ40ab & physcond)
            MenuItem('L1_BPH-0DR3-eEM9jJ40_2MU3V').setLogic( d.MU3V.x(2) & d.TOPO_0DR03_eEM9ab_CjJ40ab & physcond)

            MenuItem("L1_JPSI-1M5-eEM9" ).setLogic( d.TOPO_1INVM5_eEM9s1_eEMs6  & physcond)
            MenuItem("L1_JPSI-1M5-eEM15").setLogic( d.TOPO_1INVM5_eEM15s1_eEMs6 & physcond)

            MenuItem('L1_LLP-RO-eEM').setLogic( d.TOPO_100RATIO_0MATCH_eTAU40si2_eEMall & physcond)
            MenuItem('L1_LLP-NOMATCH-eEM').setLogic( d.TOPO_NOT_0MATCH_eTAU40si1_eEMall & physcond)

            MenuItem('L1_DR-eTAU30LeTAU20L-jJ55').setLogic( d.TOPO_2DISAMB_jJ55ab_0DR28_eTAU30abl_eTAU20abl & physcond)
            MenuItem('L1_cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L').setLogic( d.cTAU30M & d.cTAU20M.x(2) & d.TOPO_0DR28_eTAU30abl_eTAU20abl & physcond)
            MenuItem('L1_cTAU30M_2cTAU20M_DR-eTAU30MeTAU20M').setLogic( d.cTAU30M & d.cTAU20M.x(2) & d.TOPO_0DR28_eTAU30abm_eTAU20abm & physcond)
            MenuItem('L1_cTAU30M_2cTAU20M_DR-eTAU30LeTAU20L-jJ55').setLogic( d.cTAU30M & d.cTAU20M.x(2) & d.TOPO_2DISAMB_jJ55ab_0DR28_eTAU30abl_eTAU20abl & physcond)
            MenuItem('L1_cTAU30M_2cTAU20M_DR-eTAU30eTAU20').setLogic( d.cTAU30M & d.cTAU20M.x(2) & d.TOPO_0DR28_eTAU30ab_eTAU20ab & physcond)
            MenuItem('L1_cTAU30M_2cTAU20M_DR-eTAU30eTAU20-jJ55').setLogic( d.cTAU30M & d.cTAU20M.x(2) & d.TOPO_2DISAMB_jJ55ab_0DR28_eTAU30ab_eTAU20ab & physcond)
            MenuItem('L1_cTAU20M_DR-eTAU20eTAU12-jJ40').setLogic( d.cTAU20M & d.TOPO_2DISAMB_jJ40ab_0DR10_eTAU20ab_eTAU12ab & physcond)
            MenuItem('L1_eTAU80_2cTAU30M_DR-eTAU30eTAU20').setLogic( d.eTAU80 & d.cTAU30M.x(2) & d.TOPO_0DR28_eTAU30ab_eTAU20ab & physcond)
            
            
            MenuItem('L1_4jJ30p0ETA24_0DETA24-eTAU30eTAU12').setLogic         ( d.jJ300ETA25.x(4) & d.TOPO_0DETA24_eTAU30s2_eTAU12s2      & physcond)
            MenuItem('L1_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20').setLogic ( d.jJ300ETA25.x(4) & d.TOPO_0DETA24_4DPHI99_eTAU30ab_eTAU20ab  & physcond)
            MenuItem('L1_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12').setLogic ( d.jJ300ETA25.x(4) & d.TOPO_0DETA24_4DPHI99_eTAU30ab_eTAU12ab  & physcond)
            MenuItem('L1_4jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12').setLogic( d.jJ300ETA25.x(4) & d.TOPO_0DETA24_10DPHI99_eTAU30ab_eTAU12ab & physcond)
            MenuItem('L1_jJ85p0ETA21_3jJ40p0ETA25_cTAU20M_2cTAU12M').setLogic   ( d.jJ850ETA21 & d.jJ400ETA25.x(3) & d.cTAU20M & d.cTAU12M.x(2)   & physcond)
            #ATR-27132
            MenuItem('L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24-eTAU30eTAU12').setLogic         ( d.cTAU20M & d.cTAU12M & d.jJ300ETA25.x(4) & d.TOPO_0DETA24_eTAU30s2_eTAU12s2          & physcond)
            MenuItem('L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU20').setLogic ( d.cTAU20M & d.cTAU12M & d.jJ300ETA25.x(4) & d.TOPO_0DETA24_4DPHI99_eTAU30ab_eTAU20ab  & physcond)
            MenuItem('L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_4DPHI99-eTAU30eTAU12').setLogic ( d.cTAU20M & d.cTAU12M & d.jJ300ETA25.x(4) & d.TOPO_0DETA24_4DPHI99_eTAU30ab_eTAU12ab  & physcond)
            MenuItem('L1_cTAU20M_cTAU12M_4jJ30p0ETA24_0DETA24_10DPHI99-eTAU30eTAU12').setLogic( d.cTAU20M & d.cTAU12M & d.jJ300ETA25.x(4) & d.TOPO_0DETA24_10DPHI99_eTAU30ab_eTAU12ab & physcond)

            MenuItem('L1_jMJJ-400-NFF-0DPHI20').setLogic( d.TOPO_400INVM_0DPHI20_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-400-NFF-0DPHI22').setLogic( d.TOPO_400INVM_0DPHI22_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-400-NFF-0DPHI24').setLogic( d.TOPO_400INVM_0DPHI24_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-400-NFF-0DPHI26').setLogic( d.TOPO_400INVM_0DPHI26_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-300-NFF').setLogic( d.TOPO_300INVM_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-400-NFF').setLogic( d.TOPO_400INVM_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-500-NFF').setLogic( d.TOPO_500INVM_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-700').setLogic( d.TOPO_700INVM_AjJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_jMJJ-700-NFF').setLogic( d.TOPO_700INVM_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_MU5VF_jMJJ-300-NFF').setLogic( d.MU5VF & d.TOPO_300INVM_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_eEM22M_jMJJ-300').setLogic( d.eEM22M & d.TOPO_300INVM_AjJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_eEM22M_jMJJ-300-NFF').setLogic( d.eEM22M & d.TOPO_300INVM_jJ60s6_AjJ50s6 & physcond)
            MenuItem('L1_HT150-jJ50s5pETA32_jMJJ-400-CF').setLogic( d.TOPO_HT150_jJ50s5pETA32   & d.TOPO_400INVM_AjJ60s6pETA32_AjJ50s6p30ETA49 & physcond)
            MenuItem('L1_jMJJ-400-CF').setLogic( d.TOPO_400INVM_AjJ60s6pETA32_AjJ50s6p30ETA49 & physcond)
            MenuItem('L1_jJ90_DETA20-jJ90J').setLogic( d.jJ50 & d.TOPO_0DETA20_jJ90s1_jJs2 & physcond)
            MenuItem('L1_HT190-jJ40s5pETA21').setLogic( d.TOPO_HT190_jJ40s5pETA21   & physcond)
            MenuItem('L1_SC175-SCjJ10').setLogic(  d.TOPO_SC175_SCjJ10abpETA26 & physcond)

            #ATR-30179
            MenuItem('L1_cTAU30M_3DR35-MU8F-eTAU30').setLogic( d.cTAU30M & d.TOPO_3DR35_MU8Fab_eTAU30ab & physcond)
            
            # Needed?
            MenuItem('L1_DPHI-2eEM5').setLogic( d.TOPO_27DPHI32_eEMs1_eEMs6 & physcond)
            # Need to redefine these wrt Phase-I TE
            MenuItem('L1_DPHI-2eEM5_VTE5p24ETA49').setLogic( d.TOPO_27DPHI32_eEMs1_eEMs6 & Not(d.TE524ETA49) & physcond).setTriggerType(TT.calo) 
            MenuItem('L1_DPHI-2eEM5_VTE10').setLogic( d.TOPO_27DPHI32_eEMs1_eEMs6 & Not(d.TE10) & physcond).setTriggerType(TT.calo)
            MenuItem('L1_DPHI-2eEM9_VTE50').setLogic( d.eEM9.x(2) & d.TOPO_27DPHI32_eEMs1_eEMs6 & Not(d.TE50) & physcond).setTriggerType(TT.calo)
            MenuItem('L1_BTAG-MU3VjJ40').setLogic( d.TOPO_0DR04_MU3Vab_CjJ40ab & physcond)
            MenuItem('L1_BTAG-MU5VFjJ80').setLogic( d.TOPO_0DR04_MU5VFab_CjJ80ab & physcond)
            MenuItem('L1_BTAG-MU5VFjJ40_2jJ40p0ETA25').setLogic( d.TOPO_0DR04_MU5VFab_CjJ40ab & d.jJ400ETA25.x(2) & physcond)
            MenuItem('L1_BTAG-MU5VFjJ20_2jJ40p0ETA25_jJ50p0ETA25').setLogic(  d.TOPO_0DR04_MU5VFab_CjJ20ab  & d.jJ400ETA25.x(2) & d.jJ500ETA25 & physcond)
            MenuItem('L1_BTAG-MU5VFjJ20_2jJ40p0ETA25').setLogic(  d.TOPO_0DR04_MU5VFab_CjJ20ab  & d.jJ400ETA25.x(2) & physcond)
            MenuItem('L1_BTAG-MU3VFjJ20_2jJ40p0ETA25').setLogic(  d.TOPO_0DR04_MU3VFab_CjJ20ab  & d.jJ400ETA25.x(2) & physcond)
            MenuItem('L1_BPH-8M15-2MU3V-BO'    ).setLogic( d.TOPO_8INVM15_2CMU3Vab & physcond)           # 96% for Upsi
            #ATR-29784
            MenuItem('L1_DPHI-2eEM1').setLogic( d.TOPO_23DPHI32_2eEM1s & physcond)
            MenuItem('L1_DPHI-2eTAU1').setLogic( d.TOPO_23DPHI32_2eTAU1s & physcond)
            MenuItem('L1_DPHI-2eEM1_VjTE200').setLogic( d.TOPO_23DPHI32_2eEM1s & Not(d.jTE200) & physcond)
            MenuItem('L1_DPHI-2eTAU1_VjTE200').setLogic( d.TOPO_23DPHI32_2eTAU1s & Not(d.jTE200) & physcond)
            MenuItem('L1_DPHI-2eEM1_VjTE200_GAP_AANDC').setLogic( d.TOPO_23DPHI32_2eEM1s & Not(d.jTE200) & GAPAC & physcond)
            MenuItem('L1_DPHI-2eTAU1_VjTE200_GAP_AANDC').setLogic( d.TOPO_23DPHI32_2eTAU1s & Not(d.jTE200) & GAPAC & physcond)
            MenuItem('L1_DPHI-2jTAU1').setLogic( d.TOPO_23DPHI32_2jTAU1s & physcond)

            # ATR-30727
            MenuItem('L1_10INVM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_10INVM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_15INVM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_15INVM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_20INVM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_20INVM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_25INVM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_25INVM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_10SUM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_10SUM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_15SUM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_15SUM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_20SUM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_20SUM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_25SUM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_25SUM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_15INVM-15SUM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_15INVM200_15SUM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)
            MenuItem('L1_20INVM-20SUM-DPHI-2jJ5_VjTE200').setLogic(d.TOPO_20INVM200_20SUM200_20DPHI32_2jJ5s & Not(d.jTE200) & physcond)

            # ATR-30728
            MenuItem('L1_1INVM-DPHI-2eTAU1_VjTE200').setLogic(d.TOPO_1INVM200_23DPHI32_2eTAU1s & Not(d.jTE200) & physcond)
            MenuItem('L1_2INVM-DPHI-2eTAU1_VjTE200').setLogic(d.TOPO_2INVM200_23DPHI32_2eTAU1s & Not(d.jTE200) & physcond)
            MenuItem('L1_3INVM-DPHI-2eTAU1_VjTE200').setLogic(d.TOPO_3INVM200_23DPHI32_2eTAU1s & Not(d.jTE200) & physcond)
            MenuItem('L1_4INVM-DPHI-2eTAU1_VjTE200').setLogic(d.TOPO_4INVM200_23DPHI32_2eTAU1s & Not(d.jTE200) & physcond)
            MenuItem('L1_3SUM-DPHI-2eTAU1_VjTE200').setLogic(d.TOPO_3SUM200_23DPHI32_2eTAU1s & Not(d.jTE200) & physcond)
            MenuItem('L1_4SUM-DPHI-2eTAU1_VjTE200').setLogic(d.TOPO_4SUM200_23DPHI32_2eTAU1s & Not(d.jTE200) & physcond)

            # g-2 tau (ATR-30638)
            MenuItem('L1_2cTAU50M_DPHI-2eTAU50').setLogic(d.cTAU50M.x(2) & d.TOPO_30DPHI32_2eTAU50s & physcond)
            MenuItem('L1_eTAU70_2cTAU50M_DPHI-2eTAU50').setLogic(d.eTAU70 & d.cTAU50M.x(2) & d.TOPO_30DPHI32_2eTAU50s & physcond)

            # ATR-30824
            MenuItem('L1_BTAG-MU3VFjJ40').setLogic( d.TOPO_0DR04_MU3VFab_CjJ40ab & physcond)
            MenuItem('L1_BTAG-MU3VFjJ40_2jJ30p0ETA25').setLogic( d.TOPO_0DR04_MU3VFab_CjJ40ab & d.jJ300ETA25.x(2) & physcond)
            MenuItem('L1_BTAG-MU3VFjJ40_2jJ30p0ETA25_jJ50p0ETA25').setLogic( d.TOPO_0DR04_MU3VFab_CjJ40ab & d.jJ300ETA25.x(2) & d.jJ500ETA25 & physcond)
            MenuItem('L1_BTAG-MU3VFjJ40_2jJ40p0ETA25').setLogic( d.TOPO_0DR04_MU3VFab_CjJ40ab & d.jJ400ETA25.x(2) & physcond)
            MenuItem('L1_BTAG-MU3VFjJ40_2jJ40p0ETA25_jJ50p0ETA25').setLogic( d.TOPO_0DR04_MU3VFab_CjJ40ab & d.jJ400ETA25.x(2) & d.jJ500ETA25 & physcond)

            MenuItem('L1_BTAG-MU5VFjJ40').setLogic( d.TOPO_0DR04_MU5VFab_CjJ40ab & physcond)
            MenuItem('L1_BTAG-MU5VFjJ40_2jJ30p0ETA25').setLogic( d.TOPO_0DR04_MU5VFab_CjJ40ab & d.jJ300ETA25.x(2) & physcond)
            MenuItem('L1_BTAG-MU5VFjJ40_2jJ30p0ETA25_jJ50p0ETA25').setLogic( d.TOPO_0DR04_MU5VFab_CjJ40ab & d.jJ300ETA25.x(2) & d.jJ500ETA25 & physcond)
            MenuItem('L1_BTAG-MU5VFjJ40_2jJ40p0ETA25_jJ50p0ETA25').setLogic( d.TOPO_0DR04_MU5VFab_CjJ40ab & d.jJ400ETA25.x(2) & d.jJ500ETA25 & physcond)

        except NameError as ex:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            fn,ln,_,_ = traceback.extract_tb(exc_traceback)[0]
            fn = fn.rsplit("/",1)[-1]

            msg = ex.args[0]
            log.error("Creation of L1Topo item failed, since variable %s" % msg)
            m = re.match("name '(?P<varname>.*)' is not defined", msg)
            if m:
                newmsg = "In line %i in file %s, %s" % (ln, fn, msg)
                missingVar =  m.groupdict()["varname"]
                key = missingVar.split('_',1)[-1]
                if key in ItemDef.otherTopoNames:
                    alternative = ', '.join(ItemDef.otherTopoNames[key])
                    log.info("However, there is an alternative defined: %s" % alternative)
                    newmsg += ", however there is an alternative: %s" % alternative
                ex.args=(newmsg,)
            raise


        except Exception as ex:
            log.error( "Creation of L1Topo item failed, will abort!: %s" , ex)
            raise

        # Phase-II
        ItemDef_run4.registerItems(d, menuName)

        # =======================================================
        #
        # Here we define the items for the secondary partitions
        #
        # =======================================================

        # Partition 2
        MenuItem.currentPartition = 2

        # Partition 3
        MenuItem.currentPartition = 3

        MenuItem('L1_RD3_BGRP15'         ).setLogic( d.RNDM3 & d.BGRP0 & d.BGRP15             ).setTriggerType(TT.rand)


        # reset to partition 1
        MenuItem.currentPartition = 1

        MenuItem.l1configForRegistration = None

    @staticmethod
    def registerItems_AllCTPIn(tc):
        """
        CTP maps the L1 items to its inputs via two switch matrices.
        The first one is used to identify all CTP inputs, and is constructed
        using a dummy menu which accesses every input item.
        We construct this menu dynamically by reading all of the thresholds
        on every connector.
        The real L1 menu is used to generate the second switch matrix only.
        """

        MenuItem.l1configForRegistration = tc

        d = ItemDef.threshold_conditions(tc)

        # Use this as a default
        physcond = d.BGRP0 & d.BGRP1

        for board in list(L1MenuFlags.boards().values()) + list(L1MenuFlags.legacyBoards().values()):
            log.info(f"Currently reading config for board {board['name']}")

            # Only neededed for the direct CTPIN cables
            if board['name'] not in ['Ctpin7','Ctpin8','Ctpin9']:
                continue

            # Ignore the sub-boards, just use the merger board
            if board['name'] in ['LegacyTopo0','LegacyTopo1']:
                continue

            for conn in board['connectors']:
                itemname = f"L1_{conn['name']}_Thresholds"
                
                # Handling for Phase-I topo algorithms
                if conn['name'] in ['Topo2El','Topo3El','MuCTPiEl']:
                    # Accumulate list of thresholds for the connector
                    thresholds = []

                    prefix = 'MUTOPO' if conn['name'] == 'MuCTPiEl' else 'TOPO'

                    for g in conn['algorithmGroups']:
                        for a in g['algorithms']:
                            for l in a.outputlines:
                                thresholds.append(f"{prefix}_{l}")
                    log.info(f"Combining {len(thresholds)} thresholds into item {itemname} for connector {conn['name']}")
                    
                    # Need to do AND here because the AND logic occurs in the fully connected CAM,
                    # while OR is later in the LUTs which support fewer inputs
                    item_str = '&'.join(f"d.{t.replace('-','_')}" for t in thresholds)
                    log.info(item_str)
                    MenuItem(itemname).setLogic(eval(item_str) & physcond)


                elif conn['name'] == 'LegacyTopoMerged':
                    # Accumulate list of thresholds for the connector
                    thresholds = []
                    for g in conn['signalGroups']:
                        for s in g['signals']:
                            if s.__class__==str:
                                thresholds.append(s)
                            elif s.__class__==tuple and s[0]:
                                thresholds.append(s[0])
                    log.info(f"Combining {len(thresholds)} thresholds into item {itemname} for connector {conn['name']}")

                    # Need to do AND here because the AND logic occurs in the fully connected CAM,
                    # while OR is later in the LUTs which support fewer inputs
                    item_str = '&'.join(f"d.{t.replace('-','_')}" for t in thresholds)
                    log.info(item_str)
                    MenuItem(itemname).setLogic(eval(item_str) & physcond)


                # Handling for multiplicity thresholds
                else:

                    # Accumulate list of thresholds for the connector
                    thresholds = []
                    for t in conn['thresholds']:
                        if t.__class__==str and 'SPARE' not in t:
                            thresholds.append(t)
                        elif t.__class__==tuple and t[0] and 'SPARE' not in t[0]:
                            thresholds.append(t[0])
                    log.info(f"Combining {len(thresholds)} thresholds into item {itemname} for connector {conn['name']}")
                    
                    # Need to do AND here because the AND logic occurs in the fully connected CAM,
                    # while OR is later in the LUTs which support fewer inputs
                    # For some reason we drop the 'p' (Run 2 '.') here
                    item_str = '&'.join(f"d.{t.replace('p','')}" for t in thresholds)
                    log.info(item_str)
                    MenuItem(itemname).setLogic(eval(item_str) & physcond)

        MenuItem.l1configForRegistration = None
