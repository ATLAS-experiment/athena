# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from ..Base.L1MenuFlags import L1MenuFlags
from ..Base.MenuConfObj import TopoMenuDef


def defineInputsMenu():

    ctpinBoards = dict() # Ctpin/Slot9 (CTPCAL, NIM1, NIM2)
    topoBoards = dict()  # Topo1, Topo2, Topo3
    muctpiBoard = dict() # MuCTPi


    #-----------------------------------
    # SLOT 9 / CON 1 (CTPCal, NIM1,NIM2)
    # https://twiki.cern.ch/twiki/bin/view/Atlas/LevelOneCentralTriggerSetup#CTPIN_Slot_9
    #-----------------------------------
    ctpinBoards["Ctpin9"] = dict()
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
                'AFP_NSC', 'AFP_FSC', 'AFP_FSC_TOF_T0', 'AFP_FSC_TOF_T1', 'AFP_FSC_TOF_T2', 'AFP_FSC_TOF_T3'   # 2xAFP
            ]
        }
    ]


    #
    # new topo board for multiplicities
    #
    topoBoards["Topo1"] = dict([("connectors",[])])
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
        "thresholds" : [ # Topo1A: eFex EM, eFex TAU, gJ, gLJ
            # eTAU thresholds for commissioning
            ('eTAU12',3), ('eTAU20',3), ('eTAU1',3),

            (None,3),

            'eTAU20L', 'eTAU20M', 'eTAU30', 'eTAU30M',
            'eTAU35', 'eTAU35M', 'eTAU40HM','eTAU60', 'eTAU80', 'eTAU140',
        
            # eTAU thresholds for production
            #'eTAUSPARE6', 'eTAUSPARE7', 
            
            None, None,
            
            # gLJ thresholds for commissioning
            'gLJ80p0ETA25', 'gLJ100p0ETA25', 'gLJ140p0ETA25', 'gLJ160p0ETA25',

            # gLJ thresholds for production
            'gLJSPARE1', 'gLJSPARE2', 'gLJSPARE3', 'gLJSPARE4',

            None, 

            # gJ thresholds for commissioning
            ('gJ20p0ETA25',3), ('gJ20p25ETA49',3), ('gJSPARE1',3),

            (None,3),

            'gJ50p0ETA25', 'gJ100p0ETA25', 
            'gJ400p0ETA25',
            
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
            ('jJ20',3), ('jJ30',3), ('jJ30p0ETA25',3), ('jJ40',3), ('jJ40p0ETA25',3),
            ('jJ50',3), ('jJ55',3), ('jJ55p0ETA23',3), ('jJ60',3),
            ('jJSPARE1',3), ('jJSPARE2',3),

            (None,3),

            'jJ70p0ETA23', 'jJ80', 'jJ80p0ETA25', 'jJ85p0ETA21', 'jJ90', 'jJ125',
            'jJ140', 'jJ160', 'jJ180', 'jJ500',

            'jJ15p30ETA49','jJ20p30ETA49',
            'jJ40p30ETA49', 'jJ50p30ETA49', 'jJ60p30ETA49', 'jJ90p30ETA49', 'jJ125p30ETA49',

            # jJ thresholds for production
            'jJSPARE3', 'jJSPARE4',

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
            # jTAU thresholds for commissioning
            ('jTAU20',3), 

            (None,3),

            'jTAU30', 'jTAU30M',
            # jTAU thresholds for production
            'jTAU1',

            None, 

            # cTAU thresholds for commissioning
            ('cTAU12M',3), ('cTAU20M',3),# ('cTAUSPARE1',3), 
            (None,3),

            'cTAU30M', 'cTAU35M', 
            # cTAU thresholds for production
            'cTAUSPARE2',

            None,

            # jEM thresholds for commissioning
            'jEM20', 'jEM20M', 
            # jEM thresholds for production
            'jEMSPARE1',
    
            # LAr saturation for Phase-I
            ('LArSaturation',1),
            # ZeroBias Topo Algo
            ('ZeroBiasB', 1),

         #   (None,1),

            # energy thresholds
            # commissioning
            # jXE
            ('jXE60',1), ('jXE70',1), ('jXE80',1), ('jXE90',1), ('jXE100',1), ('jXE110',1), ('jXE120',1), ('jXE500',1),
            # gXE
            #('gXERHO70',1), ('gXERHO100',1),
            ('gXENC70',1), ('gXENC100',1),
            ('gXEJWOJ60',1), ('gXEJWOJ70',1), ('gXEJWOJ80',1), ('gXEJWOJ100',1), ('gXEJWOJ110',1), ('gXEJWOJ120',1), ('gXEJWOJ500',1),
            # gTE
            ('gTE200',1),

            # MHT
            ('gMHT500',1),

            # test thresholds
            ('jXEC100',1),
            ('jTE200',1), ('jTEC200',1), ('jTEFWD100',1), ('jTEFWDA100',1), ('jTEFWDC100',1),
            # additional heavy ion jTE items
            ('jTE3',1), ('jTE4',1), ('jTE10',1), ('jTE5',1), ('jTE20',1), ('jTE50',1),
            ('jTE100',1) , ('jTE600',1), ('jTE1500',1), ('jTE3000',1), ('jTE4000',1), ('jTE6500',1), ('jTE7000',1), ('jTE7500',1),
            ('jTEFWDA1',1), ('jTEFWDC1',1), ('jTEFWDA5',1), ('jTEFWDC5',1),

            # spare energy thresholds for commissioning
            ('jXESPARE1',1), ('jXESPARE2',1), ('jXESPARE3',1), ('jXESPARE4',1), ('jXESPARE5',1),

            # production
            # decrement jXESPARE for additional heavy ion jTE thresholds
            # ('jXESPARE6',1), ('jXESPARE7',1), ('jXESPARE8',1),('jXESPARE9',1), ('jXESPARE10',1), ('jXESPARE11',1),
            # ('jXESPARE12',1), ('jXESPARE13',1), ('jXESPARE14',1),

        ]
    })

    topoBoards["Topo2"] = dict()
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
                    TopoMenuDef( '0INVM10-3MU3Vab',                          outputbits = 0 ), # added due to ATR-29924
                ]
            },            
            {
                "fpga" : 0,
                "clock" : 1,
                "algorithms" : [
                ]
            }, 
            {
                "fpga" : 1,
                "clock" : 0,
                "algorithms" : [
                    TopoMenuDef( 'ZEE-eEM24sm2',                         outputbits = 2 ),
                ]
            },
            {
                "fpga" : 1,
                "clock" : 1,
                "algorithms" : [
                ]
            }
        ]
    })

    topoBoards["Topo3"] = dict()
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
                        TopoMenuDef('23DPHI32-2eEM1s', outputbits=13), #ATR-29784
                        TopoMenuDef('23DPHI32-2eTAU1s', outputbits=14), #ATR-29784
                        TopoMenuDef('23DPHI32-2jTAU1s', outputbits=15), #ATR-29784
                    ]
            },
            {
                "fpga" : 0,
                "clock" : 1,
                "algorithms" : [
                    ]
            },
            {
                "fpga" : 1,
                "clock" : 0,
                "algorithms" : [
                        TopoMenuDef( '7INVM14-MU5VFab-MU3VFab',              outputbits = 0 ), # added due to ATR-29924
                    ]
            },
            {
                "fpga" : 1,
                "clock" : 1,
                "algorithms" : [
                    ]
            },
        ]

    })

    muctpiBoard["MuCTPi"] = dict()
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

    L1MenuFlags.boards().clear()

    L1MenuFlags.boards().update( topoBoards )   # Topo1, Topo2, Topo3

    L1MenuFlags.boards().update( muctpiBoard )  # MuCTPi

    L1MenuFlags.boards().update( ctpinBoards )  # CTPIN/Slot9 NIM1, NIM2, CALREQ
    #----------------------------------------------

    def remapThresholds():
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

    #----------------------------------------------

    remapThresholds()

