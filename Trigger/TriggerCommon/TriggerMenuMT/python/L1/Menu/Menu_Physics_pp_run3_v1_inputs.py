# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from collections import OrderedDict as odict

from ..Base.L1MenuFlags import L1MenuFlags
from ..Base.MenuConfObj import TopoMenuDef


def remapThresholds(L1MenuFlags):
    # remap thresholds. TODO: add checks in case the remap does not fulfill HW constraints?
    for boardName, boardDef in L1MenuFlags.boards().items():
        if "connectors" in boardDef:
            for c in boardDef["connectors"]:
                if "thresholds" in c:
                    thresholdsToRemove = []
                    for thrIndex, thrName in enumerate(c["thresholds"]):
                        nBits = 0
                        if type(thrName)==tuple:
                            (thrName,nBits) = thrName
                        if thrName in L1MenuFlags.ThresholdMap():
                            if (L1MenuFlags.ThresholdMap()[thrName] != ''):
                                if nBits > 0:
                                    c["thresholds"][thrIndex] = (L1MenuFlags.ThresholdMap()[thrName],nBits)
                                else:
                                    c["thresholds"][thrIndex] = L1MenuFlags.ThresholdMap()[thrName]
                            else:
                                thresholdsToRemove.append(thrIndex) 
                    for i in reversed(thresholdsToRemove):
                        del c["thresholds"][i]


def defineInputsMenu():

    ctpinBoards = odict() # Ctpin/Slot9 (CTPCAL, NIM1, NIM2)
    topoBoards = odict()  # Topo1, Topo2, Topo3
    muctpiBoard = odict() # MuCTPi

    #-----------------------------------
    # SLOT 9 / CON 1 (CTPCal, NIM1,NIM2)
    # https://twiki.cern.ch/twiki/bin/view/Atlas/LevelOneCentralTriggerSetup#CTPIN_Slot_9
    #-----------------------------------
    ctpinBoards["Ctpin9"] = odict()
    ctpinBoards["Ctpin9"]["connectors"] = []
    ctpinBoards["Ctpin9"]["connectors"] += [
        {
            "name" : "CTPCAL",
            "format" : "multiplicity",
            "nbitsDefault" : 1,
            "type" : "ctpin",
            "legacy" : False,
            "thresholds" : [
                'BCM_AtoC', 'BCM_CtoA', 'BCM_Wide', # 3 x 1-bit BCM
                ('BCM_Comb',3), # 1x 3-bit BCM
                'BCM6', 'BCM7', 'BCM8', # 2-hit BCM, for Run 3. 8 is not used
                (None, 6),
                'BMA0', 'BMA1',  # 2x BMA demonstrator
                'BPTX0','BPTX1', # 2x BPTX
                'LUCID_A', 'LUCID_C', # 2x LUCID 
                (None,4),
                'ZDC_0', 'ZDC_1', 'ZDC_2', # 3x ZDC
                'CAL0','CAL1','CAL2', # 3 x CALREQ
            ]
        },
        {
            "name" : "NIM1",
            "format" : "multiplicity",
            "nbitsDefault" : 1,
            "type" : "ctpin",
            "legacy" : False,
            "thresholds" : [
                'MBTS_A0', 'MBTS_A1', 'MBTS_A2', 'MBTS_A3', 'MBTS_A4'  , 'MBTS_A5', 'MBTS_A6', 'MBTS_A7', 'MBTS_A8', 'MBTS_A9', 'MBTS_A10', 'MBTS_A11',
                'MBTS_A12', 'MBTS_A13', 'MBTS_A14', 'MBTS_A15', # 16x MBTSSI 
                ('MBTS_A',3),         # 1x MBTS_A
                'NIML1A',             # L1A for CTP monitoring itself
                'NIMLHCF',            # LHCF
                'AFP_NSA', 'AFP_FSA', 'AFP_FSA_TOF_T0', 'AFP_FSA_TOF_T1', 'AFP_FSA_TOF_T2', 'AFP_FSA_TOF_T3',   # 2xAFP
                'BMA2', 'BMA3',  # 2x BMA demonstrator
            ]
        },
        {
            "name" : "NIM2",
            "format" : "multiplicity",
            "nbitsDefault" : 1,
            "type" : "ctpin",
            "legacy" : False,
            "thresholds" : [
                'MBTS_C0', 'MBTS_C1', 'MBTS_C2', 'MBTS_C3', 'MBTS_C4', 'MBTS_C5', 'MBTS_C6', 'MBTS_C7', 'MBTS_C8', 'MBTS_C9', 'MBTS_C10', 'MBTS_C11', 
                'MBTS_C12', 'MBTS_C13', 'MBTS_C14', 'MBTS_C15', # 16x MBTSSI 
                ('MBTS_C',3), # 1x MBTS_C
                'NIMTGC',     # TGC
                'NIMRPC',     # RPC
                'NIMTRT',     # TRT
                'AFP_NSC', 'AFP_FSC', 'AFP_FSC_TOF_T0', 'AFP_FSC_TOF_T1', 'AFP_FSC_TOF_T2', 'AFP_FSC_TOF_T3',   # 2xAFP
                'ZDC_ALT_0', 'ZDC_ALT_1', 'ZDC_ALT_2' # 3xZDC alternative LUCROD
            ]
        }
    ]


    #
    # new topo board for multiplicities
    #
    topoBoards["Topo1"] = odict([("connectors",[])])
    topoBoards["Topo1"]["connectors"].append({ # first optical connector
        "name" : "Topo1Opt0",
        "format" : "multiplicity",
        "nbitsDefault" : 2,
        "type" : "optical",
        "legacy" : False,
        "thresholds" : [  # Topo1A: eFex EM, eFex TAU, gJ, gLJ 
            # eEM thresholds for commissioning
            ('eEM5',3), ('eEM7',3), ('eEM9',3), ('eEM10L',3), 

            (None,3), (None,3), (None,3), (None,3), 

            'eEM12L', 'eEM15', 'eEM18', 'eEM18L', 'eEM18M',
            'eEM22M', 'eEM24L',
            #beam splashes
            'eEM22A', 'eEM22C',
            #ATR-26333, adding eEM12, potentially more efficient than eEM12L in central HI collisions
            'eEM12',
            # variable eEM  thresholds
            'eEM24VM', 'eEM26', 'eEM26L', 'eEM26M', 'eEM26T', 'eEM28M', 'eEM40L',
            #ATR-26979, eEMSPARE1 was replaced by eEM1, eEMSPARE2 was replaced by eEM2, decrement other eEMSPARE thresholds
            'eEM1', 'eEM2',

            # eEM thresholds for production
            'eEMSPARE1',

            ('ZeroBiasA', 1)
        ],

    })

    topoBoards["Topo1"]["connectors"].append({ # second optical connector
        "name" : "Topo1Opt1",
        "format" : "multiplicity",
        "nbitsDefault" : 2,
        "type" : "optical",
        "fpga" : 0,
        "legacy" : False,
        "thresholds" : [ # Topo1A: eFex EM, eFex TAU, gFEX gJ, gFEX gLJ
            # eTAU thresholds for production
            ('eTAU70',3), ('eTAU12',3), ('eTAU20',3),

            (None, 3),

            'eTAU20L', 'eTAU20M', 'eTAU30', 'eTAU30M',
            'eTAU35', 'eTAU35M', 'eTAU40HM', 'eTAU40HT',
            'eTAU60', 'eTAU60HM', 'eTAU80', 'eTAU140',

            None, None,
        
            # gLJ thresholds for production
            'gLJ80p0ETA25', 'gLJ100p0ETA25', 'gLJ140p0ETA25', 'gLJ160p0ETA25',

             # prospective Run 4 L1 item, ATR-30180
            'gLJ90p0ETA25',

            # gLJ thresholds for commissioning
            'gLJSPARE1', 'gLJSPARE2', 'gLJSPARE3',

            None,

            # gJ thresholds for production
            ('gJ20p0ETA25',3), ('gJ20p25ETA49',3), ('gJSPARE1',3),

            (None,3),

            'gJ50p0ETA25', 'gJ100p0ETA25', 'gJ400p0ETA25',

            None,
        ]
    })

    topoBoards["Topo1"]["connectors"].append({ # third optical connector
        "name" : "Topo1Opt2",
        "format" : "multiplicity",
        "nbitsDefault" : 2,
        "type" : "optical",
        "fpga" : 1,
        "legacy" : False,
        "thresholds" : [ # Topo1B: jFex small-R jet, jFex large-R jet, combined eFex/jFex TAU, gFex+jFex EX, gFex+jFex SumET, jFex TAU
            # jJ thresholds for commissioning
            ('jJ5',3), ('jJ10',3),
            ('jJ20',3), ('jJ30',3), ('jJ30p0ETA25',3), ('jJ40',3), ('jJ40p0ETA25',3),
            ('jJ50',3), ('jJ55',3), ('jJ55p0ETA23',3), ('jJ60',3),

            (None,3),

            'jJ70p0ETA23', 'jJ80', 'jJ80p0ETA25', 'jJ85p0ETA21', 'jJ90', 'jJ125',
            'jJ140', 'jJ160', 'jJ180', 'jJ500',

            #'jJ15p30ETA49','jJ20p30ETA49',
            'jJ40p30ETA49', 'jJ50p30ETA49', 'jJ60p30ETA49', 'jJ90p30ETA49', 'jJ125p30ETA49',

            'jJ5p30ETA49', 'jJ10p30ETA49',
            # jJ thresholds for production
            'jJ50p0ETA25',

            # prospective Run 4 L1 item, ATR-30180
            'jJ56p0ETA49',

            None, None,

            # jLJ thresholds for commissioning
            'jLJ80', 'jLJ120', 'jLJ140', 'jLJ180',

            # jLJ thresholds for production
            'jLJ60', 'jLJ100', 'jLJ160', 'jLJ200',

        ]
    })

    topoBoards["Topo1"]["connectors"].append({ # fourth optical connector
        "name" : "Topo1Opt3",
        "format" : "multiplicity",
        "nbitsDefault" : 2,
        "type" : "optical",
        "fpga" : 1,
        "legacy" : False,
        "thresholds" : [ # Topo1B: jFex small-R jet, jFex large-R jet, combined eFex/jFex TAU, gFex+jFex EX, gFex+jFex SumET, jFex TAU
            # jTAU thresholds
            ('jTAU20',3), 
            (None,3),

            'jTAU1',
            None, None, None,

            # cTAU thresholds
            ('cTAU12M',3), ('cTAU20M',3), ('cTAUSPARE1',3), 

            'cTAU30M', 'cTAU35M', 'cTAU50M',
            'cTAUSPARE2',

            # jEM thresholds for commissioning
            'jEM25', 'jEM20M', 
            # jEM thresholds for production

            # prospective Run 4 L1 item, ATR-30180
            'jEM35',
    
            # LAr saturation for Phase-I
            ('LArSaturation',1),
            # ZeroBias Topo Algo
            ('ZeroBiasB', 1),

            # energy thresholds
            # commissioning
            # jXE
            ('jXE60',1), ('jXE70',1), ('jXE80',1), ('jXE90',1), ('jXE100',1), ('jXE110',1), ('jXE120',1), ('jXE500',1),
            # gXE
            #('gXERHO70',1), ('gXERHO100',1),
            ('gXENC70',1), ('gXENC100',1),
            ('gXEJWOJ60',1), ('gXEJWOJ70',1), ('gXEJWOJ80',1), ('gXEJWOJ100',1), ('gXEJWOJ110',1), ('gXEJWOJ120',1), ('gXEJWOJ500',1),
            # gTE
            ('gTE3',1), ('gTE5',1), ('gTE10',1), ('gTE200',1),

            # MHT
            ('gMHT500',1),

            # test thresholds
            ('jXEC100',1),
            ('jTE200',1), ('jTEC200',1), ('jTEFWD100',1), ('jTEFWDA100',1), ('jTEFWDC100',1),
            # additional heavy ion jTE items
            ('jTE3',1), ('jTE4',1), ('jTE10',1), ('jTE5',1), ('jTE20',1), ('jTE50',1),
            ('jTE100',1) , ('jTE600',1), ('jTE1500',1), ('jTE4000',1), ('jTE6500',1), ('jTE8300',1), ('jTE9000',1), ('jTE10000',1),('jTE12000',1),
            ('jTEFWDA1',1), ('jTEFWDC1',1), ('jTEFWDA5',1), ('jTEFWDC5',1),

            # spare energy thresholds for commissioning
            ('jXESPARE1',1),

        ]
    })

    topoBoards["Topo2"] = odict()
    topoBoards["Topo2"]["connectors"] = []
    topoBoards["Topo2"]["connectors"].append({
        "name" : "Topo2El",
        "format" : "topological",
        "type" : "electrical",
        "legacy" : False,
        "algorithmGroups" : [
            {
                "fpga" : 0,
                "clock" : 0,
                "algorithms" : [
                    TopoMenuDef( '2INVM9-0DR15-MU5VFab-MU3Vab',          outputbits = 0 ), # BLS 
                    TopoMenuDef( '2INVM9-0DR15-2MU3Vab',                 outputbits = 1 ), # BLS 
                    TopoMenuDef( 'INVM_DR_2MU5VFab',                     outputbits = (2,3), outputlines = [ '2INVM9-2DR15-2MU5VFab',      # BLS
                                                                                                             '8INVM15-0DR22-2MU5VFab' ] ), # BLS
                    TopoMenuDef( '5DETA99-5DPHI99-2MU3VFab',              outputbits = 4 ), # Low-mass DY
                    TopoMenuDef( '5DETA99-5DPHI99-MU5VFab-MU3Vab',       outputbits = 5 ), # Low-mass DY
                    TopoMenuDef( '5DETA99-5DPHI99-2MU5VFab',             outputbits = 6 ), # Low-mass DY
                    TopoMenuDef( 'DR_2MU5VFab',                          outputbits = (7,8) , outputlines = [  '0DR15-2MU5VFab' ,   # LVF
                                                                                                              '10DR99-2MU5VFab'] ), # Msonly Narrow Scan
                    TopoMenuDef( '8INVM15-0DR22-CMU5VFab-CMU3Vab',       outputbits = 9 ), # BLS
                    TopoMenuDef( 'LATE-MU10s1',                          outputbits = 10 ),
                    TopoMenuDef( 'INVM_DR_2MU3VFab',                     outputbits = (11,12), outputlines = ['2INVM9-0DR15-2MU3VFab',
                                                                                                              '7INVM11-25DR99-2MU3VFab'] ), #BLS, ATR-21566
        ]
            },

            {
                "fpga" : 0,
                "clock" : 1,
                "algorithms" : [
                    TopoMenuDef( '0INVM10-3MU3Vab',                          outputbits = 0 ), # BLS
                    TopoMenuDef( '0DR04-MU3Vab-CjJ40ab',                     outputbits = 1 ), # Bjet, TODO: not a primary
                    TopoMenuDef( '0DR04-MU3VFab-CjJ40ab'                 , outputbits = 2),
                    TopoMenuDef( '0DR04-MU5VFab-CjJ40ab'                 , outputbits = 3),
                    TopoMenuDef( '0DR04-MU5VFab-CjJ80ab',                    outputbits = 4 ), # Bjet, TODO: not a primary
                    TopoMenuDef( '0INVM10-3MU3VFab',                         outputbits = 5 ), # BLS
                    TopoMenuDef( '2DISAMB-jJ55ab-0DR28-eTAU30abl-eTAU20abl', outputbits = 6 ),
                    TopoMenuDef( '2DISAMB-jJ40ab-0DR10-eTAU20ab-eTAU12ab',   outputbits = 7 ),
                    TopoMenuDef( '2DISAMB_jJ55ab_DR_eTAU_eTAU',              outputbits = (8), outputlines = [ '2DISAMB-jJ55ab-0DR28-eTAU30ab-eTAU20ab']),
                    TopoMenuDef( 'DR_eTAU30ab_eTAU20ab',                     outputbits = (9), outputlines = [ '0DR28-eTAU30ab-eTAU20ab'  ]),
                    TopoMenuDef( 'ADVAE2A-jJ0s-eTAU0s-MU0s-jXE0s',           outputbits = (10,11), outputlines = [ 'ADVAE2A-6jJ0s-4eTAU0s-4MU0s-jXE0s-Tight',
                                                                                                                   'ADVAE2A-6jJ0s-4eTAU0s-4MU0s-jXE0s-Loose' ] ), # VAE AD
                    TopoMenuDef( '0DR28-eTAU30abl-eTAU20abl',                outputbits = 12 ),
                ]
            },
            
            {
                "fpga" : 1,
                "clock" : 0,
                "algorithms" : [
                    TopoMenuDef( 'INVM_DPHI_eEMsm6',                     outputbits = (0,1), outputlines = [ '0INVM70-27DPHI32-eEM12sm1-eEM12sm6',
                                                                                                             '0INVM70-27DPHI32-eEM15sm1-eEM15sm6' ] ),
                    
                    TopoMenuDef( 'ZEE-eEM24sm2',                         outputbits = 2 ),
                    TopoMenuDef( '0DR03-eEM9ab-CjJ40ab',                 outputbits = 3 ),
                    TopoMenuDef( 'INVM_eEMs6',                           outputbits = (4,5), outputlines = [ '1INVM5-eEM9s1-eEMs6',
                                                                                                             '1INVM5-eEM15s1-eEMs6'] ),
                    TopoMenuDef( '27DPHI32-eEMs1-eEMs6',                 outputbits = 6 ),
                    TopoMenuDef( '0INVM70-27DPHI32-eEM9s1-eEM9s6',       outputbits = 7 ),
                    TopoMenuDef( '0INVM70-27DPHI32-eEM9sl1-eEM9sl6',     outputbits = 8 ),
                    TopoMenuDef( '0INVM9-eEM9ab-eEMab',                  outputbits = 9 ),
                    TopoMenuDef( 'INVM_BOOSTDR_Ranges_eEM12sl6',         outputbits = (10,11), outputlines = ['0INVM30-2DR15-eEM12sl1-eEM12sl6', 
                                                                                                              '25INVM70-13DR25-eEM12sl1-eEM12sl6']),  

                ]
            },
            
            {
                "fpga" : 1,
                "clock" : 1,
                "algorithms" : [
                    TopoMenuDef( 'jINVM_DPHI_NFF',                             outputbits = (0,3), outputlines = ['400INVM-0DPHI26-jJ60s6-AjJ50s6',
                                                                                                                  '400INVM-0DPHI24-jJ60s6-AjJ50s6',
                                                                                                                  '400INVM-0DPHI22-jJ60s6-AjJ50s6',
                                                                                                                  '400INVM-0DPHI20-jJ60s6-AjJ50s6'] ),
                    TopoMenuDef( 'jINVM',                                       outputbits = (4,7), outputlines = ['300INVM-AjJ60s6-AjJ50s6',
                                                                                                                  '400INVM-AjJ60s6-AjJ50s6',
                                                                                                                  '500INVM-AjJ60s6-AjJ50s6',
                                                                                                                  '700INVM-AjJ60s6-AjJ50s6'] ),
                    TopoMenuDef( 'jINVM_DPHI',                                  outputbits = (8,11), outputlines = ['400INVM-0DPHI26-AjJ60s6-AjJ50s6',
                                                                                                                   '400INVM-0DPHI24-AjJ60s6-AjJ50s6',
                                                                                                                   '400INVM-0DPHI22-AjJ60s6-AjJ50s6',
                                                                                                                   '400INVM-0DPHI20-AjJ60s6-AjJ50s6'] ),
                    TopoMenuDef( 'DPHI_jXE40delay_jJ40s',                    outputbits = (12, 13), outputlines = ['0DPHI10-jXE40delay-jJ40s',
                                                                                                                   '0DPHI99-jXE40delay-jJ40s'] ),
                ]
            }
        ]
    })

    topoBoards["Topo3"] = odict()
    topoBoards["Topo3"]["connectors"] = []
    topoBoards["Topo3"]["connectors"].append({
        "name" : "Topo3El",
        "format" : "topological",
        "type" : "electrical",
        "legacy" : False,
        "algorithmGroups" : [
            {
                "fpga" : 0,
                "clock" : 0,
                "algorithms" : [
                    TopoMenuDef( 'HT190-jJ40s5pETA21',                       outputbits = 0 ),
                    TopoMenuDef( 'jINVM_NFF',                                outputbits = (1,4), outputlines = ['300INVM-jJ60s6-AjJ50s6',
                                                                                                                '400INVM-jJ60s6-AjJ50s6',
                                                                                                                '500INVM-jJ60s6-AjJ50s6',
                                                                                                                '700INVM-jJ60s6-AjJ50s6',] ), # TODO: needed? 
                    TopoMenuDef( 'HT150-jJ50s5pETA32',                       outputbits = 5 ),
                    TopoMenuDef( '400INVM-AjJ60s6pETA32-AjJ50s6p30ETA49',    outputbits = 6 ),
                    TopoMenuDef( 'SC175-SCjJ10abpETA26',                     outputbits = 7 ),
                    TopoMenuDef( '0DETA20-jJ90s1-jJs2',                      outputbits = 8 ),
                    TopoMenuDef( '100RATIO-0MATCH-eTAU40si2-eEMall',         outputbits = 9 ),
                    TopoMenuDef( 'NOT-0MATCH-eTAU40si1-eEMall',              outputbits = 10),
                    TopoMenuDef( '0DETA24-eTAU30s2-eTAU12s2',                outputbits = 11),
                    TopoMenuDef( '0DETA24-4DPHI99-eTAU30ab-eTAU12ab',        outputbits = 12),
                    # Heavy ion Topo triggers
                    TopoMenuDef('23DPHI32-2eEM1s', outputbits=13), #ATR-29784
                    TopoMenuDef('23DPHI32-2eTAU1s', outputbits=14), #ATR-29784
                    TopoMenuDef('23DPHI32-2jTAU1s', outputbits=15), #ATR-29784
                ]
            },

            {
                "fpga" : 0,
                "clock" : 1,
                "algorithms" : [
                    TopoMenuDef( 'KF-jXE-AjJall',                  outputbits = (0,5), outputlines = [ 'KF-jXE40-AjJall',
                                                                                                       'KF-jXE50-AjJall',
                                                                                                       'KF-jXE55-AjJall',
                                                                                                       'KF-jXE60-AjJall',
                                                                                                       'KF-jXE65-AjJall',
                                                                                                       'KF-jXE75-AjJall'] ),
                    TopoMenuDef( 'ZAFB_DPHI',                      outputbits = (6,7), outputlines = [ '60INVM-04DPHI32-eEM18abm-jEM25s625ETA49',
                                                                                                       '60INVM-25DPHI32-eEM18abm-jEM25s625ETA49'] ),
                    TopoMenuDef( 'CEP_CjJ',                        outputbits = (8,9), outputlines = [ 'CEP-CjJ90s6',
                                                                                                       'CEP-CjJ100s6'] ),
                    TopoMenuDef( 'ZAFB_DPHIM',                    outputbits = (10,11), outputlines = [ '60INVM-04DPHI32-eEM18abm-jEM20sm625ETA49',
                                                                                                         '60INVM-25DPHI32-eEM18abm-jEM20sm625ETA49'] ),
                ]
            },

            {
                "fpga" : 1,
                "clock" : 0,
                "algorithms" : [
                    TopoMenuDef( 'INVM_DR_eEM_MU',                       outputbits = (0,1), outputlines =  ['0INVM10-0DR15-eEM10abl-MU8Fab',
                                                                                                            '0INVM10-0DR15-eEM15abl-MU5VFab' ]) #LFV
                    
                ]
            },

            {
                "fpga" : 1,
                "clock" : 1,
                "algorithms" : [
                    TopoMenuDef('7INVM14-MU5VFab-MU3VFab',              outputbits=0), #BLS, ATR-22782
                    TopoMenuDef('7INVM14-2MU3Vab',                      outputbits=1), #BLS, ATR-22782
                    TopoMenuDef('INVM_2MU3VFab',                        outputbits=(2,3), outputlines = ['7INVM14-2MU3VFab', 
                                                                                                          '7INVM22-2MU3VFab' ] ), #BLS, ATR-21566
                    TopoMenuDef('7INVM22-MU5VFab-MU3VFab',              outputbits=4), #BLS, ATR-21566
                    TopoMenuDef('7INVM22_DR_2MU3Vab',                   outputbits=(5,6), outputlines = ['7INVM22-0DR20-2MU3Vab',
                                                                                                          '7INVM22-0DR12-2MU3Vab']),
                    TopoMenuDef('2INVM9-0DR15-C-MU5VFab-MU3Vab',        outputbits=7), #BLS, test
                    TopoMenuDef('0INVM10C-3MU3Vab',                     outputbits=8), #BLS, test
                    TopoMenuDef('7INVM14-0DR25-MU5VFab-MU3VFab',        outputbits=9), #BLS 
                    TopoMenuDef('7INVM22-0DR20-2MU3VFab',               outputbits=10), #BLS, ATR-21566

                    TopoMenuDef('30DPHI32-2eTAU50s',                    outputbits=11), # g-2 trigger
                    TopoMenuDef('3DR35-MU8Fab-eTAU30ab',                outputbits = 12), #ATR-30656
                ]
            }
        ]
    })

    muctpiBoard["MuCTPi"] = odict() 
    muctpiBoard["MuCTPi"]["connectors"] = []
    muctpiBoard["MuCTPi"]["connectors"].append({
        "name" : "MuCTPiOpt0",
        "format" : "multiplicity",
        "nbitsDefault" : 2,
        "type" : "optical",
        "legacy" : False,
        "thresholds" : [
            ('MU3V',3), ('MU3VF',3), ('MU3VC',3), ('MU3EOF',3), ('MU5VF',3), 
            'MU8F', 'MU8VF', 'MU8FC', 'MU8FH', 'MU8VFC', 'MU9VF', 'MU9VFC', 'MU12FCH', 
            'MU14FCH', 'MU14FCHR', 'MU15VFCH', 'MU15VFCHR', 'MU18VFCH', 'MU20VFC',
            'MU4BO', 'MU4BOM', 'MU10BO', 'MU10BOM', 'MU12BOM',
            'MU8EOF', 'MU14EOF', 
            # 57 bits for standard muon thresholds
            (None,7),
            # 64th bit for NSW monitoring
            ('NSWMon', 1)
        ]

    })

    muctpiBoard["MuCTPi"]["connectors"].append({
        "name" : "MuCTPiEl",
        "format" : "topological",
        "type" : "electrical",
        "legacy" : False,
        "algorithmGroups" : [
            {
                "fpga" : 0,
                "clock" : 0,
                "algorithms" : []
            },
            {
                "fpga" : 0,
                "clock" : 1,
                "algorithms" : [
                    TopoMenuDef( "MUCTP-0DR15-2MU5VFab",            outputbits = 0 ),
                ]
            },
            {
                "fpga" : 1,
                "clock" : 0,
                "algorithms" : []
            },
            {
                "fpga" : 1,
                "clock" : 1,
                "algorithms" : []
            }
        ]
    })


    L1MenuFlags.boards().clear()

    L1MenuFlags.boards().update( topoBoards )   # Topo1, Topo2, Topo3

    L1MenuFlags.boards().update( muctpiBoard )  # MuCTPi

    L1MenuFlags.boards().update( ctpinBoards )  # CTPIN/Slot9 NIM1, NIM2, CALREQ

    #----------------------------------------------

    remapThresholds(L1MenuFlags)
