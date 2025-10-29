# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from collections import namedtuple
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from ..Base.TopoAlgos import EMMultiplicityAlgo, TauMultiplicityAlgo, JetMultiplicityAlgo, XEMultiplicityAlgo, LArSaturationAlgo, ZeroBiasAlgo
from ..Base.TopoAlgorithms import AlgType, AlgCategory

class TopoAlgoDefMultiplicity:
    """
    Defines the TopoAlgorithms that calculate multiplicities for L1Calo thresholds
    The thresholds have to be explicitly defined here.
    """
    @staticmethod
    def registerTopoAlgos(tm):

        emThresholds_3bits = [
            'eEM5', 'eEM7', 'eEM9', 'eEM10L', 
        ]
        emThresholds_2bits = [
            'eEM12L', 'eEM15', 'eEM18',  'eEM18L', 'eEM18M', 'eEM22M',
            'eEM24L', 
            'eEM22A', 'eEM22C',
            'eEM12',
        ]
        emVarThresholds_2bits = [
            'eEM24VM',  'eEM26',  'eEM26L', 'eEM26M', 'eEM26T', 'eEM28M',
            'eEM1', 'eEM2', 'eEM20M', 'eEM24M',
            'eEM40L',

            # spares
            'eEMSPARE1',
        ]

        for em in emThresholds_3bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em,
                                      nbits = 3, classtype='eEmMultiplicity')
            tm.registerTopoAlgo(alg)

        for em in emThresholds_2bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em,
                                      nbits = 2, classtype='eEmMultiplicity')
            tm.registerTopoAlgo(alg)

        for em in emVarThresholds_2bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em,
                                      nbits = 2, classtype='eEmVarMultiplicity')
            tm.registerTopoAlgo(alg)

        emThresholds_2bits = [ 
            'jEM25', 'jEM20M',

            'jEM35', # prospective Run 4 L1 item, ATR-30180
        ]
        for em in emThresholds_2bits:
            alg = EMMultiplicityAlgo( name = em,
                                      threshold = em, 
                                      nbits = 2, classtype='jEmMultiplicity')
            tm.registerTopoAlgo(alg)
                
        etauThresholds_3bits = [ 
            'eTAU1', 'eTAU2', 'eTAU12', 'eTAU20', 'eTAU70',
        ]
        jtauThresholds_3bits = [ 
            'jTAU20'
        ]        
        ctauThresholds_3bits = [ 
            'cTAU12M', 'cTAU20M', 'cTAUSPARE1',
        ]
        etauThresholds_2bits = [ 
            'eTAU20L', 'eTAU20M', 'eTAU30', 'eTAU30M', 'eTAU35', 'eTAU35M', 'eTAU40HM', 'eTAU60', 'eTAU80', 'eTAU140', 
            'eTAU40HT', 'eTAU60HM','eTAU60HL', 'eTAU80HL', 
        ]
        jtauThresholds_2bits = [ 
            'jTAU1',
        ]
        ctauThresholds_2bits = [ 
            'cTAU30M', 'cTAU35M',  'cTAU50M', 

            # spares
            'cTAUSPARE2',
        ]

        for tau in etauThresholds_3bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 3, classtype='eTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in jtauThresholds_3bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 3, classtype='jTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in ctauThresholds_3bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 3, classtype='cTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in etauThresholds_2bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 2, classtype='eTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in jtauThresholds_2bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 2, classtype='jTauMultiplicity')
            tm.registerTopoAlgo(alg)

        for tau in ctauThresholds_2bits:
            alg = TauMultiplicityAlgo( name = tau,
                                       threshold = tau, 
                                       nbits = 2, classtype='cTauMultiplicity')
            tm.registerTopoAlgo(alg)


        jJThresholds_3bits = [ 
            'jJ5', 'jJ10', 'jJ20', 'jJ30', 'jJ30p0ETA25', 'jJ40', 'jJ40p0ETA25', 'jJ50', 'jJ55', 'jJ55p0ETA23', 'jJ60',
        ]
        jJThresholds_2bits = [ 
            'jJ70p0ETA23', 'jJ80', 'jJ80p0ETA25', 'jJ85p0ETA21',
            'jJ90', 'jJ125',
            'jJ140', 'jJ160', 'jJ180', 'jJ500',

            'jJ40p30ETA49', 'jJ50p30ETA49', 'jJ60p30ETA49', 'jJ90p30ETA49', 'jJ125p30ETA49',

            'jJ50p0ETA25',

            'jJ5p30ETA49', 'jJ10p30ETA49',

            'jJ56p0ETA49', # prospective Run 4 L1 item, ATR-30180

        ]

        for jJet in jJThresholds_3bits:
            alg = JetMultiplicityAlgo( name = jJet,
                                       threshold = jJet,
                                       nbits = 3, classtype='jJetMultiplicity')
            tm.registerTopoAlgo(alg)

        for jJet in jJThresholds_2bits:
            alg = JetMultiplicityAlgo( name = jJet,
                                       threshold = jJet,
                                       nbits = 2, classtype='jJetMultiplicity')
            tm.registerTopoAlgo(alg)


        gJThresholds_3bits = [ 'gJ20p0ETA25', 'gJ20p25ETA49', 'gJSPARE1', ]
        gJThresholds_2bits = [ 'gJ50p0ETA25', 'gJ100p0ETA25', 'gJ400p0ETA25' ]

        for gJet in gJThresholds_3bits:
            alg = JetMultiplicityAlgo( name = gJet,
                                       threshold = gJet,
                                       nbits = 3, classtype='gJetMultiplicity')
            tm.registerTopoAlgo(alg)

        for gJet in gJThresholds_2bits:
            alg = JetMultiplicityAlgo( name = gJet,
                                       threshold = gJet, 
                                       nbits = 2, classtype='gJetMultiplicity')
            tm.registerTopoAlgo(alg)

        gLJThresholds_2bits = [ 
            'gLJ80p0ETA25', 'gLJ100p0ETA25', 'gLJ140p0ETA25', 'gLJ160p0ETA25', 

            'gLJ90p0ETA25', # prospective Run 4 L1 item, ATR-30180

            # spares
            'gLJSPARE1', 'gLJSPARE2', 'gLJSPARE3',
        ]

        for gLJet in gLJThresholds_2bits:
            alg = JetMultiplicityAlgo( name = gLJet,
                                       threshold = gLJet, 
                                       nbits = 2, classtype='gLJetMultiplicity')
            tm.registerTopoAlgo(alg)

        XEThresholds = [ 
            'gXEJWOJ60', 'gXEJWOJ70', 'gXEJWOJ80', 'gXEJWOJ100', 'gXEJWOJ110', 'gXEJWOJ120', 'gXEJWOJ500',
            #'gXERHO70', 'gXERHO100', 
            'gXENC70', 'gXENC100',

            'jXE60', 'jXE70', 'jXE80', 'jXE90', 'jXE100', 'jXE110', 'jXE120', 'jXE500',

            'jXEC100', 'jTE200', 'jTEC200', 'jTEFWD100', 'jTEFWDA100', 'jTEFWDC100', 
            'gTE3', 'gTE5', 'gTE10', 'gTE200', 
            'gESPRESSO200',
            # additional jTE thresholds needed for heavy ion runs
            'jTE3','jTE4','jTE5', 'jTE10', 'jTE20','jTE50',
            'jTE100', 'jTE600', 'jTE1500', 'jTE4000', 'jTE6500',
            'jTEFWDA1', 'jTEFWDC1', 'jTEFWDA5', 'jTEFWDC5', 'jTEFWD2600', 'jTEFWD5600', 'jTEFWD6300', 'jTEFWD6600',

            'gMHT500',

            'jXEPerf100',

            # spares (for any energy thresholds) 
        ]

        for XE in XEThresholds:
            alg = XEMultiplicityAlgo( name = XE, 
                                      threshold = XE,
                                      nbits = 1)
            tm.registerTopoAlgo(alg)

        tm.registerTopoAlgo(LArSaturationAlgo())

        tm.registerTopoAlgo(ZeroBiasAlgo("ZeroBiasA"))
        tm.registerTopoAlgo(ZeroBiasAlgo("ZeroBiasB"))


    @staticmethod
    def checkMultAlgoFWconstraints(l1menu):
        """
        List of the constraints in terms of multiplicity algorithms, to make sure the menu fits
        in the Topo1 FW.

        The Phase-I L1Topo boards contain 2 FPGAs, with 2 output fibers each (connected to the CTP).
        Up to 96 bits per fiber.
        """
        
        # Any changes in this mapping intended for use in P1 require changes in the L1Topo firmware!

        # If you include additional algorithms to the L1 MC Menu only for development and tests in Athena, 
        # that will still require reserving additional space space in each fiber for the new thrsholds.
        # Please add them to the end of each fiber, and label those lines.

        # Thresholds containing "Perf" in the name are explicitly for development only within Athena,
        # and don't require declaring dedicated bits (they're ignored by this check)

        multLimits = namedtuple('ML', ['thrtype', 'conn', 'nbit', 'startbit', 'endbit'])
        multiplicities = [
            # FPGA 0, Topo1 fiber 0
            multLimits(thrtype='eEM',           conn='Topo1Opt0', nbit=3, startbit=0,  endbit=11),
            multLimits(thrtype='eEM',           conn='Topo1Opt0', nbit=2, startbit=24, endbit=43),
            multLimits(thrtype='eEMV',          conn='Topo1Opt0', nbit=2, startbit=44, endbit=63),
            multLimits(thrtype='ZeroBiasA',     conn='Topo1Opt0', nbit=1, startbit=64, endbit=64),

            # FPGA 0, Topo1 fiber 1
            multLimits(thrtype='eTAU',          conn='Topo1Opt1', nbit=3, startbit=0,  endbit=8 ),
            multLimits(thrtype='eTAU',          conn='Topo1Opt1', nbit=2, startbit=12, endbit=35),
            multLimits(thrtype='gLJ',           conn='Topo1Opt1', nbit=2, startbit=40, endbit=55),
            multLimits(thrtype='gJ',            conn='Topo1Opt1', nbit=3, startbit=58, endbit=66),
            multLimits(thrtype='gJ',            conn='Topo1Opt1', nbit=2, startbit=70, endbit=75),
            multLimits(thrtype='eTAU',          conn='Topo1Opt1', nbit=2, startbit=78, endbit=81), # L1 MC pp Menu only
            
            # FPGA 1, Topo1 fiber 2
            multLimits(thrtype='jJ',            conn='Topo1Opt2', nbit=3, startbit=0,  endbit=32),
            multLimits(thrtype='jJ',            conn='Topo1Opt2', nbit=2, startbit=36, endbit=73),
            
            # FPGA 1, Topo1 fiber 3
            multLimits(thrtype='jTAU',          conn='Topo1Opt3', nbit=3, startbit=0,  endbit=2 ),
            multLimits(thrtype='jTAU',          conn='Topo1Opt3', nbit=2, startbit=6,  endbit=7 ),
            multLimits(thrtype='cTAU',          conn='Topo1Opt3', nbit=3, startbit=14, endbit=22),
            multLimits(thrtype='cTAU',          conn='Topo1Opt3', nbit=2, startbit=23, endbit=30),
            multLimits(thrtype='jEM',           conn='Topo1Opt3', nbit=2, startbit=31, endbit=36),
            multLimits(thrtype='LArSaturation', conn='Topo1Opt3', nbit=1, startbit=37, endbit=37),
            multLimits(thrtype='ZeroBiasB',     conn='Topo1Opt3', nbit=1, startbit=38, endbit=38),
            multLimits(thrtype='EN',            conn='Topo1Opt3', nbit=1, startbit=39, endbit=86),
        ]

        for conn in l1menu.connectors:
            if 'Topo1' not in conn.name or conn.legacy: continue

            for tl in conn.triggerLines:
                if 'Perf' in tl.name: continue

                algo = l1menu.topoAlgos.topoAlgos[AlgCategory.MULTI][AlgType.MULT][f'Mult_{tl.name}']

                thrtype = algo.input
                if 'LArSaturation' in algo.name:
                    thrtype = 'LArSaturation'
                elif 'XE' in algo.input or 'TE' in algo.input or 'MHT' in algo.input or 'ESPRESSO' in algo.input:
                    thrtype = 'EN'
                elif 'eEmVar' in algo.classtype:
                    thrtype = 'eEMV'

                for ml in multiplicities:
                    if conn.name == ml.conn and thrtype == ml.thrtype and algo.nbits == ml.nbit and tl.startbit >= ml.startbit and tl.endbit <= ml.endbit:
                        break
                else:
                    raise RuntimeError(f'The multiplicity algorithm {algo.name} with startbit {tl.startbit} does not fit in the current Topo1 and CTP FWs. If this is intended, please correct the multiplicity constraints and communicate the new menu to the L1Topo and CTP groups.')

            # To ensure the correct alignment, also check for the Empty and (Empty, nbits) instances in the L1 Menu input fibers
            for etl in conn.emptyTriggerLines:
                log.debug('Empty Multiplicity placeholder in bits %i-%i of %s', etl.startbit, etl.endbit, conn.name)
                for ml in multiplicities:
                    # We don't want to have any overlaps with the multiplicity declarations
                    if conn.name == ml.conn and etl.startbit >= ml.startbit and etl.endbit <= ml.endbit:
                        raise RuntimeError(f'One of the Empty or (Empty, nbits) Multiplicity algorithm placeholders in the L1 Menu is overlapping with the declared multiplicity algorithm bit ranges ({conn.name}, bits {etl.startbit}-{etl.endbit}). Check the bit mapping and algorithm alignments!')
