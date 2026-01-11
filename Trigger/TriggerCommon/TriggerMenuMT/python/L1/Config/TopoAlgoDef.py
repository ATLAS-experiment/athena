# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# *** IMPORTANT ***
# Menu parameter ordering must match that in the L1Topo
# firmware generation, document this for every algorithm
# and ensure that addvariable order matches
# Refer to https://gitlab.cern.ch/atlas-l1calo/l1topo/ph1topo/-/tree/master/src/algo

# algorithm python base classes generated from C++ code
import L1TopoAlgorithms.L1TopoAlgConfig as AlgConf
import L1TopoHardware.L1TopoHardware as HW
from .L1CaloThresholdMapping import get_threshold_cut
from .L1TopoKFMETweights import KFMETweightParameters

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from collections import namedtuple

class TopoAlgoDef:

    @staticmethod
    def registerTopoAlgos(tm):

        # constants and conversions:
        _no_m_upper_threshold = 1024*1024*10*10*10 # a check against this number in L1Topo FW and sim will disable the upper threshold on MaxMSqr
        _dr_conversion = 4   # factor 10 already included to remove . from name
        _et_conversion = 10  # 1 GeV -> 100 MeV. This is to be applied also on the pt threholds for muons. A correction is then done L1/Base/TopoAlgos.py 
        _eta_conversion = 4  # factor 10 already included to remove . from name
        _phi_conversion = 2  # factor 10 already included to remove . from name

        # eEM inputs
        # ALL
        # Parameter ordering:
        # 1. REtaMin
        # 2. RHadMin
        # 3. WsTotMin
        alg = AlgConf.eEmNoSort( name = 'eEMall', inputs = 'eEmTobs', outputs = 'eEMall' )
        alg.addgeneric('InputWidth', HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmInputWidth)
        alg.addvariable('REtaMin',   0)
        alg.addvariable('RHadMin',   0)
        alg.addvariable('WsTotMin',  0)
        tm.registerTopoAlgo(alg)  

        # SORT
        # Inherited from NoSort
        alg = AlgConf.eEmSort( name = 'eEMs', inputs = 'eEmTobs', outputs = 'eEMs' )
        alg.addgeneric('InputWidth', HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmOutputWidthSort)
        alg.addvariable('REtaMin',   0)
        alg.addvariable('RHadMin',   0)
        alg.addvariable('WsTotMin',  0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.eEmSort( name = 'eEMsl', inputs = 'eEmTobs', outputs = 'eEMsl' )
        alg.addgeneric('InputWidth', HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmOutputWidthSort)
        alg.addvariable('REtaMin',   1)
        alg.addvariable('RHadMin',   1)
        alg.addvariable('WsTotMin',  1)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.eEmSort( name = 'eEMsm', inputs = 'eEmTobs', outputs = 'eEMsm' )
        alg.addgeneric('InputWidth', HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmOutputWidthSort)
        alg.addvariable('REtaMin',   2)
        alg.addvariable('RHadMin',   2)
        alg.addvariable('WsTotMin',  2)
        tm.registerTopoAlgo(alg)

        # SELECT
        # Inherited from NoSort
        alg = AlgConf.eEmSelect( name = 'eEMab', inputs = 'eEmTobs', outputs = 'eEMab' )
        alg.addgeneric('InputWidth',  HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmOutputWidthSelect)
        alg.addvariable('MinET',     get_threshold_cut('eEM', 7)*_et_conversion)
        alg.addvariable('REtaMin',   0)
        alg.addvariable('RHadMin',   0)
        alg.addvariable('WsTotMin',  0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.eEmSelect( name = 'eEMabl', inputs = 'eEmTobs', outputs = 'eEMabl' )
        alg.addgeneric('InputWidth',  HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmOutputWidthSelect)
        alg.addvariable('MinET',     get_threshold_cut('eEM', 10)*_et_conversion)
        alg.addvariable('REtaMin',   1)
        alg.addvariable('RHadMin',   1)
        alg.addvariable('WsTotMin',  1)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.eEmSelect( name = 'eEMabm', inputs = 'eEmTobs', outputs = 'eEMabm' )
        alg.addgeneric('InputWidth',  HW.eEmInputWidth)
        alg.addgeneric('OutputWidth', HW.eEmOutputWidthSelect)
        alg.addvariable('MinET',     get_threshold_cut('eEM', 10)*_et_conversion)
        alg.addvariable('REtaMin',   2)
        alg.addvariable('RHadMin',   2)
        alg.addvariable('WsTotMin',  2)
        tm.registerTopoAlgo(alg)

        # eTAU inputs
        # all
        # Parameter ordering:
        # 1. RCoreMin
        # 2. RHadMin
        alg = AlgConf.eTauNoSort( name = 'eTAUall', inputs = 'eTauTobs', outputs = 'eTAUall')
        alg.addgeneric('InputWidth', HW.eTauInputWidth)
        alg.addgeneric('OutputWidth', HW.eTauInputWidth)
        alg.addvariable('RCoreMin', 0)
        alg.addvariable('RHadMin', 0)
        tm.registerTopoAlgo(alg)

        # SORT
        # Inherited from NoSort
        alg = AlgConf.eTauSort( name = 'eTAUs', inputs = 'eTauTobs', outputs = 'eTAUs' )
        alg.addgeneric('InputWidth', HW.eTauInputWidth)
        alg.addgeneric('OutputWidth', HW.eTauOutputWidthSort)
        alg.addvariable('RCoreMin',  0)
        alg.addvariable('RHadMin',   0)
        tm.registerTopoAlgo(alg)

        # SELECT
        # Inherited from NoSort
        alg = AlgConf.eTauSelect( name = 'eTAUab', inputs = 'eTauTobs', outputs = 'eTAUab' )
        alg.addgeneric('InputWidth',  HW.eTauInputWidth)
        alg.addgeneric('OutputWidth', HW.eTauOutputWidthSelect)
        alg.addvariable('MinET',     get_threshold_cut('eTAU', 12)*_et_conversion)
        alg.addvariable('RCoreMin',  0)
        alg.addvariable('RHadMin',   0)
        tm.registerTopoAlgo(alg) 

        alg = AlgConf.eTauSelect( name = 'eTAUabl', inputs = 'eTauTobs', outputs = 'eTAUabl' )
        alg.addgeneric('InputWidth',  HW.eTauInputWidth)
        alg.addgeneric('OutputWidth', HW.eTauOutputWidthSelect)
        alg.addvariable('MinET',     get_threshold_cut('eTAU', 12)*_et_conversion)
        alg.addvariable('RCoreMin',  1)
        alg.addvariable('RHadMin',   0)
        tm.registerTopoAlgo(alg) 

        alg = AlgConf.eTauSelect( name = 'eTAUabm', inputs = 'eTauTobs', outputs = 'eTAUabm' )
        alg.addgeneric('InputWidth',  HW.eTauInputWidth)
        alg.addgeneric('OutputWidth', HW.eTauOutputWidthSelect)
        alg.addvariable('MinET',     get_threshold_cut('eTAU', 12)*_et_conversion)
        alg.addvariable('RCoreMin',  2)
        alg.addvariable('RHadMin',   0)
        tm.registerTopoAlgo(alg) 

        # Muon inputs 
        
        # SELECT
        # Parameter order:
        # 1. MinEtRPC
        # 2. MinEtTGC
        # 3. MinEta
        # 4. MaxEta
        # 5. InnerCoinCut
        # 6. FullStationCut
        # 7. GoodMFieldCut

        alg = AlgConf.MuonSelect( name = 'MU3Vab', inputs = 'MuonTobs', outputs = 'MU3Vab' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', HW.muonOutputWidthSelect)
        alg.addvariable('MinEtRPC', 4*_et_conversion)
        alg.addvariable('MinEtTGC', 3*_et_conversion)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta', 25*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut',0)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.MuonSelect( name = 'MU3VFab', inputs = 'MuonTobs', outputs = 'MU3VFab' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', HW.muonOutputWidthSelect)
        alg.addvariable('MinEtRPC', 4*_et_conversion)
        alg.addvariable('MinEtTGC', 3*_et_conversion)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta', 25*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut',1)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.MuonSelect( name = 'MU5VFab', inputs = 'MuonTobs', outputs = 'MU5VFab' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', HW.muonOutputWidthSelect)
        alg.addvariable('MinEtRPC', 6*_et_conversion)
        alg.addvariable('MinEtTGC', 5*_et_conversion)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta', 25*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut',1)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.MuonSelect( name = 'MU8Fab', inputs = 'MuonTobs', outputs = 'MU8Fab' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', HW.muonOutputWidthSelect)
        alg.addvariable('MinEtRPC', 8*_et_conversion)
        alg.addvariable('MinEtTGC', 8*_et_conversion)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta', 25*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut',1)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.MuonSelect( name = 'CMU3Vab', inputs = 'MuonTobs', outputs = 'CMU3Vab' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', HW.muonOutputWidthSelect)
        alg.addvariable('MinEtRPC', 4*_et_conversion)
        alg.addvariable('MinEtTGC', 3*_et_conversion)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta', 10*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut',0)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.MuonSelect( name = 'CMU5VFab', inputs = 'MuonTobs', outputs = 'CMU5VFab' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', HW.muonOutputWidthSelect)
        alg.addvariable('MinEtRPC', 6*_et_conversion)
        alg.addvariable('MinEtTGC', 5*_et_conversion)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta', 10*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut',1)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)
    
        #Muon sort
        alg = AlgConf.MuonSort( name = 'MUs', inputs = 'MuonTobs', outputs = 'MUs' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', HW.muonOutputWidthSort)
        # alg.addvariable('MinET', 4)
        alg.addvariable('MinEta', 0*_eta_conversion)
        alg.addvariable('MaxEta', 25*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut', 0)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)

        #LATE 
        alg = AlgConf.MuonSort_1BC( name = 'LMUs', inputs = 'LateMuonTobArray', outputs = 'LMUs' )
        alg.addgeneric('InputWidth', HW.muonInputWidth)
        alg.addgeneric('OutputWidth', 1)
        alg.addgeneric('nDelayedMuons', 1)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta', 25*_eta_conversion)
        alg.addvariable('InnerCoinCut', 0)
        alg.addvariable('FullStationCut', 1)
        alg.addvariable('GoodMFieldCut', 0)
        tm.registerTopoAlgo(alg)

        #jJets inputs
        alg = AlgConf.jJetNoSort( name = 'AjJall_1BC', inputs = 'jJetTobs', outputs = 'AjJall_1BC' ) 
        alg.addgeneric('InputWidth', HW.jJetInputWidth)
        alg.addgeneric('OutputWidth', HW.jJetInputWidth)
        # reduce number of pipelining registers used in firmware to allow for more
        # latency to be used inside the KFMET decision algorithm
        alg.addgeneric('NumRegisters', 1)
        tm.registerTopoAlgo(alg)

        #jJ lists
        # No additional parameters
        algoList = [
            {"otype" : "jJ",   "ocut" : 50, "olist" : "ab", "etamin" : 0,  "etamax" : 32}, # jJab
            {"otype" : "CjJ",  "ocut" : 20, "olist" : "ab", "etamin" : 0,  "etamax" : 26}, # CjJab
            {"otype" : "SCjJ", "ocut" : 10, "olist" : "ab", "etamin" : 0,  "etamax" : 26}, # SCjJab
            {"otype" : "FjJ",  "ocut" : 40, "olist" : "ab", "etamin" : 30, "etamax" : 49}, # FjJab
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('otype', 'ocut', 'olist', 'etamin', 'etamax')
            for k in x:
                setattr (d, k, x[k])
            listname = "%s%s" % (d.otype,d.olist)
            algoname = AlgConf.jJetSelect
            alg = algoname( name = listname, inputs = 'jJetTobs', outputs = listname )
            alg.addgeneric('InputWidth', HW.jJetInputWidth)
            alg.addgeneric('OutputWidth', HW.jJetOutputWidthSelect)
            if d.olist == "ab":
                alg.addvariable('MinET',  get_threshold_cut(d.otype, d.ocut)*_et_conversion)
            alg.addvariable('MinEta', d.etamin*_eta_conversion)
            alg.addvariable('MaxEta', d.etamax*_eta_conversion)
            tm.registerTopoAlgo(alg)

        algoList = [
            {"otype" : "FjJ", "ocut" : 0,  "olist" : "s",  "etamin" : 30, "etamax" : 49}, # FjJs
            {"otype" : "jJ",  "ocut" : 0,  "olist" : "s",  "etamin" : 0,  "etamax" : 32}, # jJs
            {"otype" : "CjJ", "ocut" : 0,  "olist" : "s",  "etamin" : 0,  "etamax" : 26}, # CjJs
            {"otype" : "AjJ", "ocut" : 0,  "olist" : "s",  "etamin" : 0,  "etamax" : 49}, # AjJs
        ]
        for x in algoList:
            class d: # type: ignore
                __slots__ = ('otype', 'ocut', 'olist', 'etamin', 'etamax')
            for k in x:
                setattr (d, k, x[k])
            listname = "%s%s" % (d.otype,d.olist)
            algoname = AlgConf.jJetSort
            alg = algoname( name = listname, inputs = 'jJetTobs', outputs = listname )
            alg.addgeneric('InputWidth', HW.jJetInputWidth)
            alg.addgeneric('OutputWidth', HW.jJetOutputWidthSort)
            if d.olist == "ab":
                alg.addvariable('MinET',  d.ocut*_et_conversion)
            alg.addvariable('MinEta', d.etamin*_eta_conversion)
            alg.addvariable('MaxEta', d.etamax*_eta_conversion)
            tm.registerTopoAlgo(alg)


        #input list needed for ATR-18824
        # Forward electrons
        alg = AlgConf.jEmSort( name = 'jEMs25ETA49', inputs = 'jEmTobs', outputs = 'jEMs25ETA49' )
        alg.addgeneric('InputWidth', HW.jEmInputWidth)
        alg.addgeneric('OutputWidth', HW.jEmOutputWidthSort)
        alg.addvariable('MinEta', 25*_eta_conversion)
        alg.addvariable('MaxEta', 49*_eta_conversion)
        # Setting no WP, use 0-3 for None/L/M/T 
        alg.addvariable('IsoMin',    0) # see TypeWideThresholdConfig
        alg.addvariable('Frac1Min',  0)
        alg.addvariable('Frac2Min',  0)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.jEmSort( name = 'jEMsm25ETA49', inputs = 'jEmTobs', outputs = 'jEMsm25ETA49' )
        alg.addgeneric('InputWidth', HW.jEmInputWidth)
        alg.addgeneric('OutputWidth', HW.jEmOutputWidthSort)
        alg.addvariable('MinEta', 25*_eta_conversion)
        alg.addvariable('MaxEta', 49*_eta_conversion)
        # Setting medium WP , use 0-3 for None/L/M/T
        alg.addvariable('IsoMin',    2) # see TypeWideThresholdConfig
        alg.addvariable('Frac1Min',  2)
        alg.addvariable('Frac2Min',  2)
        tm.registerTopoAlgo(alg)

        # jTAU inputs # for ATR-29784
        # all
        alg = AlgConf.jTauNoSort( name = 'jTAUall', inputs = 'jTauTobs', outputs = 'jTAUall')
        alg.addgeneric('InputWidth', HW.jTauInputWidth)
        alg.addgeneric('OutputWidth', HW.jTauInputWidth)
        alg.addvariable('Isolation',0)
        alg.addvariable('passIsolation', True) #if true, isolation requirement is ignored
        tm.registerTopoAlgo(alg)

        # SORT
        alg = AlgConf.jTauSort( name = 'jTAUs', inputs = 'jTauTobs', outputs = 'jTAUs' )
        alg.addgeneric('InputWidth', HW.jTauInputWidth)
        alg.addgeneric('OutputWidth', HW.jTauOutputWidthSort)
        alg.addvariable('MinEta',  0*_eta_conversion)
        alg.addvariable('MaxEta',  32*_eta_conversion)
        alg.addvariable('Isolation',0)
        alg.addvariable('passIsolation', True) #if true, isolation requirement is ignored
        tm.registerTopoAlgo(alg)

        # MET
        # No additional parameters
        alg = AlgConf.jXENoSort( name = 'jXENoSort_1BC', inputs = 'jXETobs', outputs = 'jXENoSort_1BC' )
        alg.addgeneric('InputWidth', HW.jMetInputWidth)
        alg.addgeneric('OutputWidth', HW.metOutputWidth)
        # reduce number of pipelining registers used in firmware to allow for more
        # latency to be used inside the KFMET decision algorithm
        alg.addgeneric('NumRegisters', 1)
        tm.registerTopoAlgo(alg)

        alg = AlgConf.jXESort( name = 'jXEs', inputs = 'jXETobs', outputs = 'jXEs' )
        alg.addgeneric('InputWidth', HW.jMetInputWidth)
        alg.addgeneric('OutputWidth', HW.metOutputWidth)
        tm.registerTopoAlgo(alg)

        # jTE - use the same widths as XE as single TE TOB
        alg = AlgConf.jTENoSort( name = 'jTENoSort_1BC', inputs = 'jTETobs', outputs = 'jTENoSort_1BC' )
        alg.addgeneric('InputWidth', HW.jSumEtTotalInputWidth)
        alg.addgeneric('OutputWidth', 1)
        alg.addgeneric('NumRegisters', 1)
        tm.registerTopoAlgo(alg)

        # Decision algorithms

        # LAR  ZEE
        # Parameter ordering:
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinInvariantMassSqr
        # 4. MaxInvariantMassSqr
        algoList = [
            {"otype" : "eEM", "ocut1" : 24,  "ocut2" : 24, "olist" : "sm", "nleading1" : 2, "minInvm" : 60, "maxInvm" : 100, "inputwidth": HW.eEmOutputWidthSort},
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('otype', 'ocut1', 'ocut2', 'olist', 'nleading1', 'minInvm', 'maxInvm', 'inputwidth')
            for k in x:
                setattr (d, k, x[k])
            toponame = 'ZEE-eEM24sm2'
            log.debug("Define %s", toponame)
            inputList = d.otype + d.olist
            alg = AlgConf.InvariantMassInclusive1( name = toponame, inputs = inputList, outputs = toponame)
            alg.addgeneric('InputWidth', d.inputwidth)
            alg.addgeneric('MaxTob', d.nleading1)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',  get_threshold_cut(d.otype, d.ocut1)*_et_conversion )
            alg.addvariable('MinET2',  get_threshold_cut(d.otype, d.ocut2)*_et_conversion )
            alg.addvariable('MinMSqr', d.minInvm*d.minInvm*_et_conversion*_et_conversion )
            alg.addvariable('MaxMSqr', d.maxInvm*d.maxInvm*_et_conversion*_et_conversion )
            tm.registerTopoAlgo(alg)



        #ATR-30179
        #Mu+Tau dR cut
        algolist = [
            {"minDr": 3, "maxDr": 99, "mult": 1, "otype1" : "MU8Fab" ,  "otype2" : "eTAU", "ocut2" : 30, "olist2" : "ab",  }, #3DR99-MU8Fab-eTAU30ab
            {"minDr": 3, "maxDr": 35, "mult": 1, "otype1" : "MU8Fab" ,  "otype2" : "eTAU", "ocut2" : 30, "olist2" : "ab",  }, #3DR35-MU8Fab-eTAU30ab
            {"minDr": 3, "maxDr": 30, "mult": 1, "otype1" : "MU8Fab" ,  "otype2" : "eTAU", "ocut2" : 30, "olist2" : "ab",  }, #3DR30-MU8Fab-eTAU30ab
            {"minDr": 3, "maxDr": 28, "mult": 1, "otype1" : "MU8Fab" ,  "otype2" : "eTAU", "ocut2" : 30, "olist2" : "ab",  }, #3DR28-MU8Fab-eTAU30ab
        ]

        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('minDr', 'maxDr', 'mult', 'otype1', 'otype2', 'ocut2', 'olist2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s%d%s" % (d.otype2, d.ocut2, d.olist2)
            toponame = "%iDR%i-%s%s"  % (d.minDr, d.maxDr, obj1, obj2)
            log.debug("Define %s", toponame)
            inputList = [ d.otype1, 'eTAUs']
            algoname = AlgConf.DeltaRSqrIncl2
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ]) 
            #alg = AlgConf.NotMatch( name = toponame, inputs = [ d.otype1, 'eTAUs'], outputs = [ toponame ] )
            alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
            alg.addgeneric('InputWidth2', HW.eTauOutputWidthSort) 
            alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob2', 1)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',    0*_et_conversion)
            alg.addvariable('MinET2',    get_threshold_cut('eTAU', d.ocut2)*_et_conversion)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion)
            tm.registerTopoAlgo(alg)

        
        # dimu DR items
        # Parameter ordering:
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinDeltaRSqr
        # 4. MaxDeltaRSqr
        listofalgos=[
            {"minDr": 0, "maxDr": 15, "mult": 2, "otype1" : "MU3Vab" ,  "otype2" : "",       }, #0DR15-2MU3Vab 
            {"minDr": 0, "maxDr": 24, "mult": 2, "otype1" : "MU3Vab" ,  "otype2" : "",       }, #0DR24-2MU3Vab 
            # a min dR cut is needed when using muons from two lists. Maybe 0.1 is too large
            {"minDr": 1, "maxDr": 15, "mult": 1, "otype1" : "MU5VFab",  "otype2" : "MU3Vab", }, #1DR15-MU5VFab-MU3Vab 
            {"minDr": 1, "maxDr": 22, "mult": 1, "otype1" : "MU5VFab",  "otype2" : "MU3Vab", }, #1DR22-MU5VFab-MU3Vab
            {"minDr": 2, "maxDr": 15, "mult": 2, "otype1" : "MU5VFab",  "otype2" : "",       }, #2DR15-2MU5VFab
            {"minDr": 0, "maxDr": 22, "mult": 2, "otype1" : "MU5VFab",  "otype2" : "",       }, #0DR22-2MU5VFab
            {"minDr": 2, "maxDr": 99, "mult": 2, "otype1" : "MU3Vab" ,  "otype2" : "",       }, #2DR99-2MU3Vab
        ]
        for x in listofalgos:
            class d:  # type: ignore
                __slots__ = ('minDr', 'maxDr', 'mult', 'otype1', 'otype2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iDR%i-%s%s"  % (d.minDr, d.maxDr, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.DeltaRSqrIncl1 if (d.mult>1) else AlgConf.DeltaRSqrIncl2
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ]) 
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect) 
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',    0*_et_conversion)
            alg.addvariable('MinET2',    0*_et_conversion)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion)
            tm.registerTopoAlgo(alg)

        # dimu DR with opposite charge, ATR-23073
        # Parameter ordering:
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinDeltaRSqr
        # 4. MaxDeltaRSqr
        listofalgos=[
            {"minDr": 0, "maxDr": 12, "mult": 2, "otype1" : "MU3Vab", "otype2" : "", }, #0DR12C-2MU3Vab 
        ]
        for x in listofalgos:
            class d:  # type: ignore
                __slots__ = ('minDr', 'maxDr', 'mult', 'otype1', 'otype2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iDR%iC-%s%s"  % (d.minDr, d.maxDr, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.DeltaRSqrIncl1Charge if (d.mult>1) else AlgConf.DeltaRSqrIncl2Charge
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',    0*_et_conversion)
            alg.addvariable('MinET2',    0*_et_conversion)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion)
            tm.registerTopoAlgo(alg)

        # dimu INVM+DR with opposite charge, ATR-23073
        # Parameter ordering:
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinInvariantMassSqr
        # 4. MaxInvariantMassSqr
        # 5. MinDeltaRSqr
        # 6. MaxDeltaRSqr
        listofalgos=[
            {"minInvm":7, "maxInvm":22, "minDr": 0, "maxDr": 20, "mult": 2, "otype1" : "MU3Vab", "otype2" : "", }, #7INVM22-0DR20C-2MU3Vab
        ]
        for x in listofalgos:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'maxInvm', 'minDr', 'maxDr', 'mult', 'otype1', 'otype2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iINVM%i-%iDR%iC-%s%s"  % (d.minInvm, d.maxInvm, d.minDr, d.maxDr, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.InvariantMassInclusiveDeltaRSqrIncl1Charge if (d.mult>1) else AlgConf.InvariantMassInclusiveDeltaRSqrIncl2Charge
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',    0*_et_conversion)
            alg.addvariable('MinET2',    0*_et_conversion)
            alg.addvariable('MinMSqr',   d.minInvm * d.minInvm *_et_conversion*_et_conversion)
            alg.addvariable('MaxMSqr',   d.maxInvm * d.maxInvm *_et_conversion*_et_conversion)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion)
            tm.registerTopoAlgo(alg)
            
        # deta-dphi with ab+ab
        # Parameter ordering:
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinDeltaEta
        # 4. MaxDeltaEta
        # 5. DeltaPhiMin
        # 6. DeltaPhiMax
        algolist=[
            {"minDeta": 5, "maxDeta": 99, "minDphi": 5, "maxDphi": 99, "mult": 1, 
             "otype1" : "MU5VFab", "ocut1": "", "olist1": "", "nleading1": HW.muonOutputWidthSelect, 
             "otype2" : "MU3Vab" , "ocut2": "", "olist2": "", "nleading2": HW.muonOutputWidthSelect}, #5DETA99-5DPHI99-MU5VFab-MU3Vab
            {"minDeta": 5, "maxDeta": 99, "minDphi": 5, "maxDphi": 99, "mult": 2, 
             "otype1" : "MU5VFab", "ocut1": "", "olist1": "", "nleading1": HW.muonOutputWidthSelect, 
             "otype2" : ""       , "ocut2": "", "olist2": "", "nleading2": HW.muonOutputWidthSelect}, #5DETA99-5DPHI99-2MU5VFab
            {"minDeta": 5, "maxDeta": 99, "minDphi": 5, "maxDphi": 99, "mult": 2, 
             "otype1" : "MU3Vab", "ocut1": "", "olist1": "", "nleading1": HW.muonOutputWidthSelect, 
             "otype2" : ""      , "ocut2": "", "olist2": "", "nleading2": HW.muonOutputWidthSelect}, #5DETA99-5DPHI99-2MU3Vab
            {"minDeta": 5, "maxDeta": 99, "minDphi": 5, "maxDphi": 99, "mult": 2, 
             "otype1" : "MU3VFab", "ocut1": "", "olist1": "","nleading1": HW.muonOutputWidthSelect, 
             "otype2" : ""       , "ocut2": "", "olist2": "","nleading2": HW.muonOutputWidthSelect}, #5DETA99-5DPHI99-2MU3VFab
            { "minDeta": 0, "maxDeta": 24, "minDphi": 4, "maxDphi": 99, "mult": 1, 
              "otype1" : "eTAU", "ocut1": 30, "olist1": "ab", "nleading1": HW.eTauOutputWidthSelect, 
              "otype2" : "eTAU", "ocut2": 20, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect},#0DETA24_4DPHI99_eTAU30ab_eTAU20ab
            { "minDeta": 0, "maxDeta": 24, "minDphi": 4, "maxDphi": 99, "mult": 1, 
              "otype1" : "eTAU", "ocut1": 30, "olist1": "ab", "nleading1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 12, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect},#0DETA24_4DPHI99_eTAU30ab_eTAU12ab 
            { "minDeta": 0, "maxDeta": 24, "minDphi": 10, "maxDphi": 99, "mult": 1, 
              "otype1" : "eTAU", "ocut1": 30, "olist1": "ab", "nleading1": HW.eTauOutputWidthSelect, 
              "otype2" : "eTAU", "ocut2": 12, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect},#0DETA24_10DPHI99_eTAU30ab_eTAU12ab
        ]
        for x in algolist:            
            class d:  # type: ignore
                __slots__ = ('minDeta', 'maxDeta', 'minDphi', 'maxDphi', 'mult',
                             'otype1', 'ocut1', 'olist1', 'nleading1',
                             'otype2', 'ocut2', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1+str(d.ocut1)+str(d.olist1))
            obj2 = "-%s" % (d.otype2+str(d.ocut2)+str(d.olist2))
            toponame = "%sDETA%s-%sDPHI%s-%s%s"  % (d.minDeta, d.maxDeta, d.minDphi, d.maxDphi, obj1, "" if d.mult>1 else obj2)            
            log.debug("Define %s", toponame)            
            inputList = [d.otype1+d.olist1] if (d.mult>1) else [d.otype1+d.olist1, d.otype2+d.olist2]
            algoname = AlgConf.DeltaEtaPhiIncl1 if (d.mult>1) else AlgConf.DeltaEtaPhiIncl2
            alg = algoname( name = toponame, inputs = inputList, outputs = [ toponame ])
            alg.addgeneric('NumResultBits', 1)            
            if (d.mult>1):
                alg.addgeneric('InputWidth', d.nleading1)
                alg.addgeneric('MaxTob', d.nleading1)
                alg.addvariable('MinET1',      0*_et_conversion)
                alg.addvariable('MinET2',      0*_et_conversion)
                alg.addvariable('MinDeltaEta', d.minDeta*_eta_conversion)
                alg.addvariable('MaxDeltaEta', d.maxDeta*_eta_conversion)
                alg.addvariable('MinDeltaPhi', d.minDphi*_phi_conversion)
                alg.addvariable('MaxDeltaPhi', d.maxDphi*_phi_conversion)
            else:
                alg.addgeneric('InputWidth1', d.nleading1)
                alg.addgeneric('InputWidth2', d.nleading2)
                alg.addgeneric('MaxTob1', d.nleading1)
                alg.addgeneric('MaxTob2', d.nleading2)
                alg.addvariable('MinET1',      0*_et_conversion)
                alg.addvariable('MinET2',      0*_et_conversion)
                alg.addvariable('DeltaEtaMin', d.minDeta*_eta_conversion)
                alg.addvariable('DeltaEtaMax', d.maxDeta*_eta_conversion)
                alg.addvariable('DeltaPhiMin', d.minDphi*_phi_conversion)
                alg.addvariable('DeltaPhiMax', d.maxDphi*_phi_conversion)
            tm.registerTopoAlgo(alg)

            
        # (ATR-8194) L1Topo HT Trigger
        algoList = [
            {"itemNameMinHT": 150, "minHT": 240, "otype" : "jJ", "ocut" : 50, "olist" : "s",   "nleading" : 5, "inputwidth": HW.jJetOutputWidthSort, "oeta" : 32}, #HT150-jJ50s5pETA32
            {"itemNameMinHT": 190, "minHT": 300, "otype" : "jJ", "ocut" : 40, "olist" : "s",   "nleading" : 5, "inputwidth": HW.jJetOutputWidthSort, "oeta" : 21}, #HT190-jJ40s5pETA21
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('itemNameMinHT', 'minHT', 'otype', 'ocut', 'olist', 'nleading', 'inputwidth', 'oeta')
            for k in x:
                setattr (d, k, x[k])
            toponame = "HT%d-%s%s%s%spETA%s" % (d.itemNameMinHT, d.otype, str(d.ocut), d.olist, str(d.nleading) if d.olist=="s" else "", str(d.oeta))
            log.debug("Define %s", toponame)
            inputList = d.otype + d.olist
            alg = AlgConf.JetHT( name = toponame, inputs = inputList, outputs = [toponame] )
            alg.addgeneric('InputWidth', d.inputwidth)
            alg.addgeneric('MaxTob', d.nleading)
            alg.addgeneric('NumRegisters', 2 if d.olist=="all" else 0)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET',  get_threshold_cut(d.otype, d.ocut)*_et_conversion)
            alg.addvariable('MinEta',      0*_eta_conversion)
            alg.addvariable('MaxEta', d.oeta*_eta_conversion)
            alg.addvariable('MinHt', d.minHT*_et_conversion)
            tm.registerTopoAlgo(alg)

        #DR for 2MU5
        # Covers chains: 0DR15-2MU5VFab
        #               10DR99-2MU5VFab
        DR_2MU5FMap = [
            { 
            "algoname" : "DR_2MU5VFab",
            "minDR"    : [0 ,10],
            "maxDR"    : [15,99],
            "otype1"   : "MU5VFab",
            "mult1"    : [2]*2
            }
        ]


        for x in DR_2MU5FMap:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minDR', 'maxDR', 'otype1', 'mult1')
            for k in x:
                setattr (d, k, x[k])
            inputList = [ d.otype1 ]
            toponames = []
            for bitId in range(len(d.minDR)):
                toponames.append("%iDR%i-%s%s" % (d.minDR[bitId], d.maxDR[bitId], str(d.mult1[bitId]), 
                                                    d.otype1 ) 
                                )
            alg = AlgConf.DeltaRSqrIncl1( name = d.algoname, inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob',     HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', len(toponames) )
            
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1',  0 * _et_conversion, bitId ) 
                alg.addvariable('MinET2',  0 * _et_conversion, bitId )
                alg.addvariable("DeltaRMin", d.minDR[bitId] * d.minDR[bitId] * _dr_conversion * _dr_conversion, bitId)
                alg.addvariable("DeltaRMax", d.maxDR[bitId] * d.maxDR[bitId] * _dr_conversion * _dr_conversion, bitId)

            tm.registerTopoAlgo(alg)


        # DM/DPHI for eEM, ATR-19302 (0INVM70...) and ATR-32259 (2INVM4...)
        # output lines = 0INVM70-27DPHI32-eEM12sm1-eEM12sm6, 0INVM70-27DPHI32-eEM15sm1-eEM15sm6, 2INVM4-0DPHI32-eEM9sm1-eEM6sm6
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        # 5. MinDeltaPhi
        # 6. MaxDeltaPhi
        eINVM_DPHIMap = [
        {  
            "algoname"  : "INVM_DPHI_eEMsm6",
            "minInvm"   : [0, 0, 2],
            "maxInvm"   : [70, 70, 5],
            "minDphi"   : [27, 27, 0],
            "maxDphi"   : [32, 32, 32],
            "otype1"    : "eEM",
            "olist1"    : "sm",
            "ocut1List" : [ 12, 15, 9 ],
            "nleading1" : 1,
            "otype2"    : "eEM",
            "ocut2List" : [ 12, 15, 6 ],
            "olist2"    : "sm",
            "nleading2" : 6

        }
        ]

        for x in eINVM_DPHIMap:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDphi', 'maxDphi',
                             'otype1', 'olist1', 'ocut1List', 'nleading1',
                             'otype2', 'ocut2List', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for bitId, ocut1Value in enumerate(d.ocut1List):
                toponames.append ("%iINVM%i-%iDPHI%i-%s%s%s%s-%s%s%s%s"  % (d.minInvm[bitId], d.maxInvm[bitId], d.minDphi[bitId], d.maxDphi[bitId],
                                                                d.otype1, str(ocut1Value) , d.olist1, str(d.nleading1) if d.olist1=="sm" else "",
                                                                d.otype2, str(d.ocut2List[bitId]) , d.olist2, str(d.nleading2) if d.olist2=="sm" else ""))
            alg = AlgConf.InvariantMassDeltaPhiInclusive2( name = d.algoname, inputs = inputList, outputs = toponames )  
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSort)
            alg.addgeneric('InputWidth2', HW.eEmOutputWidthSort)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype2, d.ocut2List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinMSqr', d.minInvm[bitId] * d.minInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MaxMSqr', d.maxInvm[bitId] * d.maxInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MinDeltaPhi', d.minDphi[bitId] * _phi_conversion, bitId)
                alg.addvariable('MaxDeltaPhi', d.maxDphi[bitId] * _phi_conversion, bitId)
            
            tm.registerTopoAlgo(alg)    

        # ATR-31830
        INVM_DISAMB_Map = [
        {
            "algoname": "INVM-eEMs-2DISAMB-jJs",#"110INVM150-eEM50s-eEM10s-2DISAMB-jJ60s",
            "disamb" :  [2, 2],
            "minInvm"   : [110, 110],
            "maxInvm"   : [150, 150],
            "otype1"  : "eEM",
            "ocut1a"   : [50, 40],
            "ocut1b"   : [10, 10],
            "olist1": "s",
            "nleading1": HW.eEmOutputWidthSort,
            "inputwidth1": HW.eEmOutputWidthSort,
            "otype2"  : "jJ",
            "ocut2"   : [60, 60],
            "nleading2": 3,
            "inputwidth2": HW.jJetOutputWidthSort,
            "olist2": "s",
            "numResultBits": 2,
        }
        ]
        for x in INVM_DISAMB_Map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'disamb', 'minInvm', 'maxInvm',
                             'otype1', 'ocut1a', 'ocut1b', 'olist1', 'nleading1', 'inputwidth1',
                             'otype2', 'ocut2', 'olist2', 'nleading2', 'inputwidth2', 'numResultBits')
            for k in x:
                setattr(d,k,x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2]

            toponames = []
            for bitId in range(d.numResultBits):
                obj1a = "-%s%s%s" % (d.otype1, str(d.ocut1a[bitId]), d.olist1)
                obj1b = "-%s%s%s" % (d.otype1, str(d.ocut1b[bitId]), d.olist1)
                obj2 = "%s%s%s"  % (d.otype2, str(d.ocut2[bitId]), d.olist2)
                toponames.append( "%iINVM%i%s%s-%sDISAMB-%s"  % (d.minInvm[bitId], d.maxInvm[bitId], obj1a, obj1b, str(d.disamb[bitId]) if d.disamb[bitId]>0 else "", obj2) )

            alg = AlgConf.InvariantMassInclusive1Disambiguation2( name = d.algoname, inputs = inputList, outputs = toponames  )
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', d.numResultBits)

            for bitId in range(d.numResultBits):
                alg.addvariable('MinET1a', get_threshold_cut(d.otype1, d.ocut1a[bitId])*_et_conversion, bitId)
                alg.addvariable('MinET1b', get_threshold_cut(d.otype1, d.ocut1b[bitId])*_et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype2, d.ocut2[bitId])*_et_conversion, bitId)
                alg.addvariable('MinMSqr', d.minInvm[bitId] * d.minInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MaxMSqr', d.maxInvm[bitId] * d.maxInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('DisambDRSqrMin', d.disamb[bitId]*d.disamb[bitId]*_dr_conversion*_dr_conversion, bitId)

            tm.registerTopoAlgo(alg)

        # Boosted DM/DPHI for eEM
        # output lines = 0INVM70-0DPHI12-eEM9sl1-eEM9sl6, 
        #                0INVM70-0DPHI12-eEM12sl1-eEM12sl6, 
        #                0INVM70-0DPHI12-eEM15sl1-eEM15sl6
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        # 5. MinDeltaPhi
        # 6. MaxDeltaPhi
        eINVM_DPHIMap = [
        {  
            "algoname"  : "INVM_INVDPHI_eEMsl6",
            "minInvm"   : 0,
            "maxInvm"   : 70,
            "minDphi"   : 0,
            "maxDphi"   : 12,
            "otype1"    : "eEM",
            "olist1"    : "sl",
            "ocut1List" : [ 9, 12],
            "nleading1" : 1,
            "otype2"    : "eEM",
            "ocut2List" : [ 9, 12 ],
            "olist2"    : "sl",
            "nleading2" : 6
        }
        ]

        for x in eINVM_DPHIMap:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDphi', 'maxDphi',
                             'otype1', 'olist1', 'ocut1List', 'nleading1',
                             'otype2', 'ocut2List', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for bitId, ocut1Value in enumerate(d.ocut1List):
                toponames.append ("%iINVM%i-%iDPHI%i-%s%s%s%s-%s%s%s%s"  % (d.minInvm, d.maxInvm, d.minDphi, d.maxDphi,
                                                                d.otype1, str(ocut1Value) , d.olist1, str(d.nleading1) if d.olist1=="sl" else "",
                                                                d.otype2, str(d.ocut2List[bitId]) , d.olist2, str(d.nleading2) if d.olist2=="sl" else ""))
            alg = AlgConf.InvariantMassDeltaPhiInclusive2( name = d.algoname, inputs = inputList, outputs = toponames )  
            
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSort)
            alg.addgeneric('InputWidth2', HW.eEmOutputWidthSort)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype1, d.ocut2List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinMSqr', d.minInvm * d.minInvm * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MaxMSqr', d.maxInvm * d.maxInvm * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MinDeltaPhi', d.minDphi * _phi_conversion, bitId)
                alg.addvariable('MaxDeltaPhi', d.maxDphi * _phi_conversion, bitId)
            
            tm.registerTopoAlgo(alg)    


        # DM/DR for eEM
         # output lines = 0INVM70-2DR15-eEM9sl1-eEM9sl6, 
        #                 0INVM70-2DR15-eEM12sl1-eEM12sl6, 
        #                
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        # 5. MinDeltaR
        # 6. MaxDeltaR
        eINVM_DRMap = [
        {  
            "algoname"  : "INVM_BOOSTDR_eEMsl6",
            "minInvm"   : 0,
            "maxInvm"   : 70,
            "minDR"   : 2,
            "maxDR"   : 15,
            "otype1"    : "eEM",
            "olist1"    : "sl",
            "ocut1List" : [ 9, 12 ],
            "nleading1" : 1,
            "otype2"    : "eEM",
            "ocut2List" : [ 9, 12 ],
            "olist2"    : "sl",
            "nleading2" : 6
        }
        ]

        for x in eINVM_DRMap:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDR', 'maxDR',
                             'otype1', 'olist1', 'ocut1List', 'nleading1',
                             'otype2', 'ocut2List', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for bitId, ocut1Value in enumerate(d.ocut1List):
                toponames.append ("%iINVM%i-%iDR%i-%s%s%s%s-%s%s%s%s"  % (d.minInvm, d.maxInvm, d.minDR, d.maxDR,
                                                                d.otype1, str(ocut1Value) , d.olist1, str(d.nleading1) if d.olist1=="sl" else "",
                                                                d.otype2, str(d.ocut2List[bitId]) , d.olist2, str(d.nleading2) if d.olist2=="sl" else ""))

            alg = AlgConf.InvariantMassInclusiveDeltaRSqrIncl2( name = d.algoname, inputs = inputList, outputs =  toponames )
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSort)
            alg.addgeneric('InputWidth2', HW.eEmOutputWidthSort)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype1, d.ocut2List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinMSqr', d.minInvm * d.minInvm * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MaxMSqr', d.maxInvm * d.maxInvm * _et_conversion * _et_conversion, bitId)
                alg.addvariable('DeltaRMin', d.minDR*d.minDR*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax', d.maxDR*d.maxDR*_dr_conversion*_dr_conversion, bitId)
            
            tm.registerTopoAlgo(alg)    

        # DM/DR Ranges for eEM12
         # output lines = 0INVM30-2DR15-eEM12sl1-eEM12sl6, 
        #                 25INVM70-13DR25-eEM12sl1-eEM12sl6, 
        #                
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        # 5. MinDeltaR
        # 6. MaxDeltaR
        eINVM_Ranges_DRMap = [
        {  
            "algoname"  : "INVM_BOOSTDR_Ranges_eEM12sl6",
            "minInvm"   : [0, 25],
            "maxInvm"   : [30,70],
            "minDR"     : [2,13],
            "maxDR"     : [15,25],
            "otype1"    : "eEM",
            "olist1"    : "sl",
            "ocut1List" : [12,12],
            "nleading1" : 1,
            "otype2"    : "eEM",
            "ocut2List" : [12,12],
            "olist2"    : "sl",
            "nleading2" : 6
        }
        ]

        for x in eINVM_Ranges_DRMap:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDR', 'maxDR',
                             'otype1', 'olist1', 'ocut1List', 'nleading1',
                             'otype2', 'ocut2List', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for bitId, ocut1Value in enumerate(d.ocut1List):
                toponames.append ("%iINVM%i-%iDR%i-%s%s%s%s-%s%s%s%s"  % (d.minInvm[bitId], d.maxInvm[bitId], d.minDR[bitId], d.maxDR[bitId],
                                                                d.otype1, str(ocut1Value) , d.olist1, str(d.nleading1) if d.olist1=="sl" else "",
                                                                d.otype2, str(d.ocut2List[bitId]) , d.olist2, str(d.nleading2) if d.olist2=="sl" else ""))
           
            alg = AlgConf.InvariantMassInclusiveDeltaRSqrIncl2( name = d.algoname, inputs = inputList, outputs =  toponames )
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSort)
            alg.addgeneric('InputWidth2', HW.eEmOutputWidthSort)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype1, d.ocut2List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinMSqr', d.minInvm[bitId] * d.minInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MaxMSqr', d.maxInvm[bitId] * d.maxInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('DeltaRMin', d.minDR[bitId]*d.minDR[bitId]*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax', d.maxDR[bitId]*d.maxDR[bitId]*_dr_conversion*_dr_conversion, bitId)
            
            tm.registerTopoAlgo(alg)    


        # DM/DR Ranges for asymmetric eEM12LeEM9L
         # output lines = 0INVM30-2DR15-eEM12sl1-eEM9sl6, 
        #                 25INVM70-13DR25-eEM12sl1-eEM9sl6, 
        #                
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        # 5. MinDeltaR
        # 6. MaxDeltaR
        eINVM_Ranges_Asymm_DRMap = [
        {  
            "algoname"  : "INVM_BOOSTDR_Ranges_Asymm_eEMsl6",
            "minInvm"   : [0, 25],
            "maxInvm"   : [30,70],
            "minDR"     : [2,13],
            "maxDR"     : [15,25],
            "otype1"    : "eEM",
            "olist1"    : "sl",
            "ocut1List" : [12,12],
            "nleading1" : 1,
            "otype2"    : "eEM",
            "ocut2List" : [9,9],
            "olist2"    : "sl",
            "nleading2" : 6
        }
        ]

        for x in eINVM_Ranges_Asymm_DRMap:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDR', 'maxDR',
                             'otype1', 'olist1', 'ocut1List', 'nleading1',
                             'otype2', 'ocut2List', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for bitId, ocut1Value in enumerate(d.ocut1List):
                toponames.append ("%iINVM%i-%iDR%i-%s%s%s%s-%s%s%s%s"  % (d.minInvm[bitId], d.maxInvm[bitId], d.minDR[bitId], d.maxDR[bitId],
                                                                d.otype1, str(ocut1Value) , d.olist1, str(d.nleading1) if d.olist1=="sl" else "",
                                                                d.otype2, str(d.ocut2List[bitId]) , d.olist2, str(d.nleading2) if d.olist2=="sl" else ""))

            alg = AlgConf.InvariantMassInclusiveDeltaRSqrIncl2( name = d.algoname, inputs = inputList, outputs =  toponames )
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSort)
            alg.addgeneric('InputWidth2', HW.eEmOutputWidthSort)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype1, d.ocut2List[bitId]) * _et_conversion, bitId)
                alg.addvariable('MinMSqr', d.minInvm[bitId] * d.minInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('MaxMSqr', d.maxInvm[bitId] * d.maxInvm[bitId] * _et_conversion * _et_conversion, bitId)
                alg.addvariable('DeltaRMin', d.minDR[bitId]*d.minDR[bitId]*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax', d.maxDR[bitId]*d.maxDR[bitId]*_dr_conversion*_dr_conversion, bitId)
            
            tm.registerTopoAlgo(alg)    


        # INVM_DR for 2MU5VFab
        # output lines = 2INVM9-2DR15-2MU5VFab and 8INVM15-0DR22-2MU5VFab
        INVM_DR_2MU5VFabMap = [
        {
            "algoname": "INVM_DR_2MU5VFab",
            "minInvm" : [2,8],
            "maxInvm" : [9,15],
            "minDR"   : [2,0],
            "maxDR"   : [15,22],
            "otype1"  : "MU5VFab",
            "mult1"   : 2
        }
        ]
        for x in INVM_DR_2MU5VFabMap:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDR', 'maxDR',
                             'otype1', 'mult1')
            for k in x:
                setattr(d,k,x[k])
            inputList = d.otype1 
            toponames = []
            for bitId in range(len(d.minDR)):
                toponames.append("%iINVM%i-%iDR%i-%i%s" % ( d.minInvm[bitId], d.maxInvm[bitId],
                                                                d.minDR[bitId] , d.maxDR[bitId], 
                                                                d.mult1, d.otype1
                                                                )
                )
            alg = AlgConf.InvariantMassInclusiveDeltaRSqrIncl1( name = d.algoname, inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1',    0*_et_conversion, bitId)
                alg.addvariable('MinET2',    0*_et_conversion, bitId)
                alg.addvariable('MinMSqr',   d.minInvm[bitId]*d.minInvm[bitId]*_et_conversion*_et_conversion, bitId)
                alg.addvariable('MaxMSqr',   d.maxInvm[bitId]*d.maxInvm[bitId]*_et_conversion*_et_conversion, bitId)
                alg.addvariable('DeltaRMin', d.minDR[bitId]*d.minDR[bitId]*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax', d.maxDR[bitId]*d.maxDR[bitId]*_dr_conversion*_dr_conversion, bitId)
            tm.registerTopoAlgo(alg)


        # INVM for 2MU3VFab
        # output lines = '7INVM14-2MU3VFab', '7INVM22-2MU3VFab'
        INVM_2MU3VFab_Map = [
            {
            "algoname": "INVM_2MU3VFab",
            "minInvm" : 7,
            "maxInvm" : [14,22],
            "otype1"  : "MU3VFab",
            "mult1"   : 2
        }
        ]
        for x in INVM_2MU3VFab_Map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'mult1', 'otype1')
            for k in x:
                setattr(d,k,x[k])
            inputList = d.otype1 
            toponames = []
            for bitId in range(len(d.maxInvm)):
                toponames.append("%iINVM%i-%i%s" % ( d.minInvm, d.maxInvm[bitId],
                                                                d.mult1, d.otype1
                                                                )
                )
            alg = AlgConf.InvariantMassInclusive1( name = d.algoname, inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1',    0*_et_conversion, bitId)
                alg.addvariable('MinET2',    0*_et_conversion, bitId)
                alg.addvariable('MinMSqr',   d.minInvm*d.minInvm*_et_conversion*_et_conversion, bitId)
                alg.addvariable('MaxMSqr',   d.maxInvm[bitId]*d.maxInvm[bitId]*_et_conversion*_et_conversion, bitId)
            tm.registerTopoAlgo(alg)



        # INVM_EM for Jpsi
        invm_map = { "algoname": 'INVM_eEMs6' , "ocutlist": [ 9, 15 ], "minInvm": 1, "maxInvm": 5, "otype" : "eEM", "olist" : "s",
                     "nleading" : 1, "inputwidth": HW.eEmOutputWidthSort}
        for x in [ invm_map ]:
            class d:  # type: ignore
                __slots__ = ('algoname', 'ocutlist', 'minInvm', 'maxInvm',
                             'otype', 'olist', 'nleading', 'inputwidth')
            for k in x:
                setattr (d, k, x[k])
            inputList = d.otype + d.olist
            toponames=[]
            for ocut in d.ocutlist:
                toponame = "%iINVM%i-%s%s%s%s-eEMs6" % (d.minInvm, d.maxInvm, d.otype, str(ocut) if ocut > 0 else "", d.olist, str(d.nleading) if d.olist=="s" else "")
                toponames.append(toponame)
            alg = AlgConf.InvariantMassInclusive2( name = d.algoname, inputs = [inputList, 'eEMs'], outputs = toponames)
            alg.addgeneric('InputWidth1', d.inputwidth)
            alg.addgeneric('InputWidth2', HW.eEmOutputWidthSort)
            alg.addgeneric('MaxTob1', d.nleading)
            alg.addgeneric('MaxTob2', HW.eEmOutputWidthSort)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitid, ocut in enumerate(d.ocutlist):
                alg.addvariable('MinET1', get_threshold_cut('eEM', ocut)*_et_conversion, bitid)
                alg.addvariable('MinET2',    0*_et_conversion, bitid)
                alg.addvariable('MinMSqr', d.minInvm*d.minInvm*_et_conversion*_et_conversion, bitid)
                alg.addvariable('MaxMSqr', d.maxInvm*d.maxInvm*_et_conversion*_et_conversion, bitid)
            tm.registerTopoAlgo(alg)

           
        # added for muon-jet:
        algoList = [
            {"minDr": 0, "maxDr": 4, "otype1" : "MU5VFab", "otype2" : "CjJ", "ocut2": 40, "olist2" : "ab"}, #0DR04-MU5VFab-CjJ40ab
            {"minDr": 0, "maxDr": 4, "otype1" : "MU3Vab" , "otype2" : "CjJ", "ocut2": 40, "olist2" : "ab"}, #0DR04-MU3Vab-CjJ40ab
            {"minDr": 0, "maxDr": 4, "otype1" : "MU5VFab", "otype2" : "CjJ", "ocut2": 80, "olist2" : "ab"}, #0DR04-MU5VFab-CjJ80ab
            {"minDr": 0, "maxDr": 4, "otype1" : "MU3VFab", "otype2" : "CjJ", "ocut2": 40, "olist2" : "ab"}, #0DR04_MU3VFab_CjJ40ab
            {"minDr": 0, "maxDr": 4, "otype1" : "MU3VFab", "otype2" : "CjJ", "ocut2": 20, "olist2" : "ab"}, #0DR04_MU3VFab_CjJ20ab
            {"minDr": 0, "maxDr": 4, "otype1" : "MU5VFab", "otype2" : "CjJ", "ocut2": 20, "olist2" : "ab"}, #0DR04_MU5VFab_CjJ20ab
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('minDr', 'maxDr', 'otype1', 'otype2', 'ocut2', 'olist2')
            for k in x:
                setattr (d, k, x[k])
            toponame = "%iDR%02d-%s-%s%s%s"  % (d.minDr, d.maxDr, d.otype1, d.otype2, str(d.ocut2), d.olist2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1, d.otype2 + d.olist2]
            alg = AlgConf.DeltaRSqrIncl2( name = toponame, inputs = inputList, outputs = [ toponame ])
            alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
            alg.addgeneric('InputWidth2', HW.jJetOutputWidthSelect)
            alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob2', HW.jJetOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)                        
            alg.addvariable('MinET1',    0*_et_conversion, 0)
            alg.addvariable('MinET2',    get_threshold_cut(d.otype2, d.ocut2)*_et_conversion, 0)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion, 0)
            tm.registerTopoAlgo(alg)

        # dimu INVM items
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        listofalgos = [
            {"minInvm": 2, "maxInvm": 8,  "mult": 2, "otype1" : "MU3Vab",  "otype2" : "",      }, #2INVM8-2MU3Vab 
            {"minInvm": 2, "maxInvm": 9,  "mult": 2, "otype1" : "MU3Vab",  "otype2" : "",      }, #2INVM9-2MU3Vab 
            {"minInvm": 8, "maxInvm": 15, "mult": 1, "otype1" : "MU5VFab", "otype2" : "MU3Vab",}, #8INVM15-MU5VFab-MU3Vab
            {"minInvm": 2, "maxInvm": 8,  "mult": 1, "otype1" : "MU5VFab", "otype2" : "MU3Vab",}, #2INVM8-MU5VFab-MU3Vab
            {"minInvm": 2, "maxInvm": 9,  "mult": 1, "otype1" : "MU5VFab", "otype2" : "MU3Vab",}, #2INVM9-MU5VFab-MU3Vab
            {"minInvm": 8, "maxInvm": 15, "mult": 2, "otype1" : "MU5VFab", "otype2" : "",      }, #8INVM15-2MU5VFab
            {"minInvm": 2, "maxInvm": 9,  "mult": 2, "otype1" : "MU5VFab", "otype2" : "",      }, #2INVM9-2MU5VFab 
            {"minInvm": 7, "maxInvm": 15, "mult": 2, "otype1" : "MU3Vab",  "otype2" : "",      }, #7INVM15-2MU3Vab 
            {"minInvm": 7, "maxInvm": 22, "mult": 1, "otype1" : "MU5VFab", "otype2" : "MU3VFab",}, #7INVM22-MU5VFab-MU3VFab, ATR-21566
            {"minInvm": 7, "maxInvm": 14, "mult": 1, "otype1" : "MU5VFab", "otype2" : "MU3VFab",}, #7INVM14-MU5VFab-MU3VFab, ATR-22782
            {"minInvm": 7, "maxInvm": 14, "mult": 2, "otype1" : "MU3Vab",  "otype2" : "",      }, #7INVM14-2MU3Vab, ATR-22782
        ]
        for x in listofalgos:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'maxInvm', 'mult', 'otype1', 'otype2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iINVM%i-%s%s"  % (d.minInvm, d.maxInvm, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.InvariantMassInclusive1 if (d.mult>1) else AlgConf.InvariantMassInclusive2
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1', 0*_et_conversion)
            alg.addvariable('MinET2', 0*_et_conversion)
            alg.addvariable('MinMSqr', d.minInvm * d.minInvm *_et_conversion*_et_conversion)
            alg.addvariable('MaxMSqr', d.maxInvm * d.maxInvm *_et_conversion*_et_conversion)
            tm.registerTopoAlgo(alg)

        toponame = "8INVM15-2CMU3Vab"
        log.debug("Define %s", toponame)        
        inputList = ['CMU3Vab']
        alg = AlgConf.InvariantMassInclusive1( name = toponame, inputs = inputList, outputs = toponame )
        alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
        alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
        alg.addgeneric('NumResultBits', 1)
        alg.addvariable('MinET1',      0*_et_conversion)
        alg.addvariable('MinET2',      0*_et_conversion)  
        alg.addvariable('MinMSqr',   8*8*_et_conversion*_et_conversion)
        alg.addvariable('MaxMSqr', 15*15*_et_conversion*_et_conversion)
        tm.registerTopoAlgo(alg)

        algolist=[
            {"minInvm": 2, "maxInvm": 8, "mult": 1, "otype1" : "CMU3Vab", "otype2" :"MU3Vab"}, # 2INVM8-CMU3Vab-MU3Vab
        ]
        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'maxInvm', 'mult', 'otype1', 'otype2', 'onebarrel')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iINVM%i-%s%s"  % (d.minInvm, d.maxInvm, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.InvariantMassInclusive1 if (d.mult>1) else AlgConf.InvariantMassInclusive2
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
                alg.addgeneric('RequireOneBarrel', d.onebarrel)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1', 0*_et_conversion)
            alg.addvariable('MinET2', 0*_et_conversion)
            alg.addvariable('MinMSqr', d.minInvm * d.minInvm *_et_conversion*_et_conversion)
            alg.addvariable('MaxMSqr', d.maxInvm * d.maxInvm *_et_conversion*_et_conversion)
            tm.registerTopoAlgo(alg)

        # dimu DR items
        algolist = [
            {"minDr": 0,  "maxDr": 24, "mult": 2, "otype1" : "CMU3Vab", "otype2" : "",       }, #0DR24-2CMU3Vab
            {"minDr": 1,  "maxDr": 24, "mult": 1, "otype1" : "CMU3Vab", "otype2" : "MU3Vab", }, #1DR24-CMU3Vab-MU3Vab 
        ]
        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('minDr', 'maxDr', 'mult', 'otype1', 'otype2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iDR%i-%s%s"  % (d.minDr, d.maxDr, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.DeltaRSqrIncl1 if (d.mult>1) else AlgConf.DeltaRSqrIncl2
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1', 0*_et_conversion)
            alg.addvariable('MinET2', 0*_et_conversion)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr *_dr_conversion*_dr_conversion)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr *_dr_conversion*_dr_conversion)
            tm.registerTopoAlgo(alg)
            

        # RATIO MATCH dedicated to Exotic #TODO: are eTAU correct to use here (and below)?
        toponame = '100RATIO-0MATCH-eTAU40si2-eEMall'
        alg = AlgConf.RatioMatch( name = toponame, inputs = [ 'eTAUs', 'eEMall'], outputs = [ toponame ] )
        alg.addgeneric('InputWidth1', HW.eTauOutputWidthSort)
        alg.addgeneric('InputWidth2', HW.eEmInputWidth)      
        alg.addgeneric('MaxTob1', 2)
        alg.addgeneric('MaxTob2', HW.eEmInputWidth)
        alg.addgeneric('NumResultBits', 1)
        alg.addvariable('MinET1', get_threshold_cut('eTAU', 40)*_et_conversion)
        alg.addvariable('MinET2',  0*_et_conversion)
        alg.addvariable('Ratio', 100, 0)
        tm.registerTopoAlgo(alg)        

        # NOT MATCH dedicated to Exotic
        toponame = 'NOT-0MATCH-eTAU40si1-eEMall'
        alg = AlgConf.NotMatch( name = toponame, inputs = [ 'eTAUs', 'eEMall'], outputs = [ toponame ] )
        alg.addgeneric('InputWidth1', HW.eTauOutputWidthSort)
        alg.addgeneric('InputWidth2', HW.eEmInputWidth)
        alg.addgeneric('MaxTob1', 1)
        alg.addgeneric('MaxTob2', HW.eEmInputWidth)
        alg.addgeneric('NumResultBits', 1)
        alg.addvariable('MinET1',  get_threshold_cut('eTAU', 40)*_et_conversion)
        alg.addvariable('MinET2',   0*_et_conversion)
        alg.addvariable('EtaMin1',  0*_eta_conversion)
        alg.addvariable('EtaMax1', 49*_eta_conversion)
        alg.addvariable('EtaMin2',  0*_eta_conversion)
        alg.addvariable('EtaMax2', 49*_eta_conversion)
        alg.addvariable('DRCut', 0) #TODO: conversion needed here?
        tm.registerTopoAlgo(alg)        

        # TODO: to be updated with phase1 met, jets
        xemap = [{"etcut": 0, "Threlist": [55, 60, 65, 75 ]}]
        for x in xemap:                
            class d:  # type: ignore
                __slots__ = ('etcut', 'Threlist')
            for k in x:
                setattr (d, k, x[k])            
            log.debug("Define %s", toponame)            
            inputList = ['jXENoSort_1BC', 'AjJall_1BC']
            toponames=[]
            for minxe in d.Threlist:
                toponames.append("KF-jXE%s-AjJall"  % (minxe))            
            alg = AlgConf.KalmanMETCorrection( name = "KF-jXE-AjJall", inputs = inputList, outputs = toponames )
            alg.addgeneric('InputWidth', HW.jJetInputWidth)
            alg.addgeneric('NumResultBits', len(toponames))
            for key, value in KFMETweightParameters.items():
                # add weight (scale factor) parameters
                # these weights are compacted into 32bit words in order to limit the number
                # of registers needed to load them in firmware
                alg.addvariable(key, value)
            alg.addvariable('MinET', 0)
            for bitid,minxe in enumerate(d.Threlist):
                alg.addvariable('KFXE', str(minxe*_et_conversion), bitid) 
            tm.registerTopoAlgo(alg)

                
        # LATE MUON : LATE-MU10s1
        for x in [     
            #{"otype" : "LATE-MU", "ocut" : 10, "inputwidth": HW.muonOutputWidthSort},
            {"otype" : "LATE-MU", "ocut" : 10, "inputwidth": HW.NumberOfDelayedMuons},
            ]:

            class d:  # type: ignore
                __slots__ = ('otype', 'ocut', 'inputwidth')
            for k in x:
                setattr (d, k, x[k])

            toponame = "%s%ss1"  % ( d.otype, str(d.ocut) )

            log.debug("Define %s", toponame)

            inputList = 'LMUs'

            alg = AlgConf.EtCut( name = toponame, inputs = inputList, outputs = toponame )
            alg.addgeneric('InputWidth', d.inputwidth)
            alg.addgeneric('MaxTob', 1)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET', str(d.ocut*_et_conversion))
            tm.registerTopoAlgo(alg)
            

        # (ATR-12748) fat jet trigger with Simple Cone algo
        algoList = [ #name (SC175) is rounded on purpose to avoid more name changes in case the actual threshold is slightly tweaked later on
            {"itemNameMinHT": 175, "minHT": 176, "otype" : "SCjJ", "ocut" : 10, "olist" : "ab", "nleading" : HW.jJetOutputWidthSelect, "inputwidth": HW.jJetOutputWidthSelect, "oeta" : 26}, #SC175-SCjJ10abpETA26
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('itemNameMinHT', 'minHT', 'otype', 'ocut', 'olist', 'nleading', 'inputwidth', 'oeta')
            for k in x:
                setattr (d, k, x[k])
            toponame = "SC%d-%s%s%s%spETA%s" % (d.itemNameMinHT, d.otype, str(d.ocut), d.olist, str(d.nleading) if d.olist=="s" else "", str(d.oeta))
            log.debug("Define %s", toponame)
            inputList = d.otype + d.olist
            alg = AlgConf.SimpleCone( name = toponame, inputs = inputList, outputs = [toponame] )
            alg.addgeneric('InputWidth', d.inputwidth)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET', d.ocut*_et_conversion)
            alg.addvariable('MinSumET', d.minHT*_et_conversion)
            alg.addvariable('MaxRSqr', 15*15*_dr_conversion*_dr_conversion)                        
            tm.registerTopoAlgo(alg)  
 
        #  0INVM9-eEM9ab-eEMab 
        algoList = [
            {"minInvm" : 0, "maxInvm": 9, "otype" : "eEM", "ocut1" : 9, "olist" : "ab", "inputwidth": HW.eEmOutputWidthSelect, "ocut2" : 0},
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'maxInvm', 'otype', 'ocut1', 'olist', 'inputwidth', 'ocut2')
            for k in x:
                setattr (d, k, x[k])
            inputList = d.otype + d.olist
            toponame = "%iINVM%i-%s%s%s-%s%s"  % (d.minInvm, d.maxInvm,
                                                  d.otype, str(d.ocut1) , d.olist,
                                                  d.otype, d.olist)
            alg = AlgConf.InvariantMassInclusive1( name = toponame, inputs = inputList, outputs = toponame)
            alg.addgeneric('InputWidth', d.inputwidth)
            alg.addgeneric('MaxTob', HW.eEmOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',  get_threshold_cut(d.otype, d.ocut1)*_et_conversion)
            alg.addvariable('MinET2',  get_threshold_cut(d.otype, d.ocut2)*_et_conversion)
            alg.addvariable('MinMSqr', d.minInvm*d.minInvm*_et_conversion*_et_conversion)
            alg.addvariable('MaxMSqr', d.maxInvm*d.maxInvm*_et_conversion*_et_conversion)
            tm.registerTopoAlgo(alg)


        # added for b-phys, 0DR03-eEM9ab-CjJ40ab
        algoList = [  
            {"minDr": 0, "maxDr": 3, "otype1" : "eEM" ,"ocut1": 9,  "olist1" : "ab", "otype2" : "CjJ", "ocut2": 40, "olist2" : "ab"} 
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('minDr', 'maxDr', 'otype1', 'ocut1', 'olist1', 'otype2', 'ocut2', 'olist2')
            for k in x:
                setattr (d, k, x[k])
            toponame = "%iDR%02d-%s%s%s-%s%s%s"  % (d.minDr, d.maxDr, d.otype1, str(d.ocut1), d.olist1, d.otype2, str(d.ocut2), d.olist2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2]
            alg = AlgConf.DeltaRSqrIncl2( name = toponame, inputs = inputList, outputs = [ toponame ])
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSelect)
            alg.addgeneric('InputWidth2', HW.jJetOutputWidthSelect)
            alg.addgeneric('MaxTob1', HW.eEmOutputWidthSelect)
            alg.addgeneric('MaxTob2', HW.jJetOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)                        
            alg.addvariable('MinET1',    get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, 0)
            alg.addvariable('MinET2',    get_threshold_cut(d.otype2,  d.ocut2)*_et_conversion, 0)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion, 0)
            tm.registerTopoAlgo(alg)


        # Axion 2EM DPHI  
        #27DPHI32-eEMs1-eEMs6
        algoList = [
            {"minDphi": 27,  "maxDphi": 32, "otype" : "eEM",  "ocut1" : 0,  "olist" : "s", "nleading1" : 1, "inputwidth1": HW.eEmOutputWidthSort, "ocut2" : 0, "nleading2": 6},
        ]
        for x in algoList:                 
            class d:  # type: ignore
                __slots__ = ('minDphi', 'maxDphi', 'otype', 'ocut1', 'olist', 'nleading1', 'inputwidth1',
                             'ocut2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            toponame = "%iDPHI%i-%s%s%s%s-%s%s%s%s"  % (d.minDphi, d.maxDphi,
                                                        d.otype, str(d.ocut1) if d.ocut1 > 0 else "", d.olist, str(d.nleading1) if d.olist=="s" else "",
                                                        d.otype, str(d.ocut2) if d.ocut2 > 0 else "", d.olist, str(d.nleading2) if d.olist=="s" else "")
            log.debug("Define %s", toponame)
            inputList = d.otype + d.olist            
            alg = AlgConf.DeltaPhiIncl1( name = toponame, inputs = inputList, outputs = toponame )
            alg.addgeneric('InputWidth', d.inputwidth1)
            alg.addgeneric('MaxTob', d.nleading2)
            alg.addgeneric('NumResultBits', 1)                        
            alg.addvariable('MinET1',      get_threshold_cut(d.otype, d.ocut1)*_et_conversion if d.ocut1 > 0 else get_threshold_cut(d.otype, 5)*_et_conversion, 0)
            alg.addvariable('MinET2',      get_threshold_cut(d.otype, d.ocut2)*_et_conversion if d.ocut2 > 0 else get_threshold_cut(d.otype, 5)*_et_conversion, 0)
            alg.addvariable('MinDeltaPhi', d.minDphi*_phi_conversion, 0)
            alg.addvariable('MaxDeltaPhi', d.maxDphi*_phi_conversion, 0)
            tm.registerTopoAlgo(alg)

        # Tau dR chains
        algolist=[
            { "minDr": 0, "maxDr": 28, "otype1" : "eTAU" ,"ocut1": 30, "olist1" : "abl",
              "nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,"otype2" : "eTAU", "ocut2": 20, "olist2" : "abl",
              "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect}, # 0DR28-eTAU30abl-eTAU20abl
            { "minDr": 0, "maxDr": 28, "otype1" : "eTAU" ,"ocut1": 30, "olist1" : "abm",
              "nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,"otype2" : "eTAU", "ocut2": 20, "olist2" : "abm",
              "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect}, # 0DR28-eTAU30abm-eTAU20abm
        ]
        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('minDr', 'maxDr', 'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth1',
                             'otype2', 'ocut2', 'olist2', 'nleading2', 'inputwidth2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s%s" % (d.otype1, str(d.ocut1), d.olist1)
            obj2 = "-%s%s%s" % (d.otype2, str(d.ocut2), d.olist2)
            toponame = "%iDR%i-%s%s"  % (d.minDr, d.maxDr, obj1, obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1 + d.olist1] if d.otype1==d.otype2 else [d.otype1 + d.olist1, d.otype2 + d.olist2]
            algoname = AlgConf.DeltaRSqrIncl1 if d.otype1==d.otype2 else AlgConf.DeltaRSqrIncl2
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if d.otype1==d.otype2:
                alg.addgeneric('InputWidth', d.inputwidth1)
                alg.addgeneric('MaxTob', d.nleading1)
            else:
                alg.addgeneric('InputWidth1', d.inputwidth1)
                alg.addgeneric('InputWidth2', d.inputwidth2)
                alg.addgeneric('MaxTob1', d.nleading1)
                alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', 1)
            if d.otype1==d.otype2:
                alg.addvariable('MinET1',    get_threshold_cut(d.otype1, d.ocut1)*_et_conversion )
                alg.addvariable('MinET2',    get_threshold_cut(d.otype2, d.ocut2)*_et_conversion )
                alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion)
                alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion)
            else:
                alg.addvariable('MinET1',    get_threshold_cut(d.otype1, d.ocut1)*_et_conversion , 0)
                alg.addvariable('MinET2',    get_threshold_cut(d.otype2, d.ocut2)*_et_conversion , 0)
                alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion, 0)
                alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion, 0)
            tm.registerTopoAlgo(alg)


        DISAMB_DR_jJ_eTau_eTau_Map = [
        {
            "algoname": "2DISAMB_jJ55ab_DR_eTAU_eTAU",
            "disamb" :  2,
            "minDR"   : 0,
            "maxDR"   : [28],
            "otype1"  : "eTAU",
            "ocut1"   : 30,
            "olist1": "ab",
            "nleading1": HW.eTauOutputWidthSelect,
            "inputwidth1": HW.eTauOutputWidthSelect,
            "otype2"  : "eTAU",
            "ocut2"   : 20,
            "nleading2": HW.eTauOutputWidthSelect,
            "inputwidth2": HW.eTauOutputWidthSelect,
            "olist2": "ab",
            "otype3"  : "jJ",
            "ocut3"   : 55,
            "nleading3": HW.jJetOutputWidthSelect,
            "inputwidth3": HW.jJetOutputWidthSelect,
            "olist3": "ab",
        },
        {
            "algoname": "2DISAMB_jJ50ab_DR_eTAU_eTAU",
            "disamb" :  2,
            "minDR"   : 0,
            "maxDR"   : [28],
            "otype1"  : "eTAU",
            "ocut1"   : 30,
            "olist1": "ab",
            "nleading1": HW.eTauOutputWidthSelect,
            "inputwidth1": HW.eTauOutputWidthSelect,
            "otype2"  : "eTAU",
            "ocut2"   : 20,
            "nleading2": HW.eTauOutputWidthSelect,
            "inputwidth2": HW.eTauOutputWidthSelect,
            "olist2": "ab",
            "otype3"  : "jJ",
            "ocut3"   : 50,
            "nleading3": HW.jJetOutputWidthSelect,
            "inputwidth3": HW.jJetOutputWidthSelect,
            "olist3": "ab",
        },
        {
            "algoname": "2DISAMB_jJ40ab_DR_eTAU_eTAU",
            "disamb" :  2,
            "minDR"   : 0,
            "maxDR"   : [28],
            "otype1"  : "eTAU",
            "ocut1"   : 30,
            "olist1": "ab",
            "nleading1": HW.eTauOutputWidthSelect,
            "inputwidth1": HW.eTauOutputWidthSelect,
            "otype2"  : "eTAU",
            "ocut2"   : 20,
            "nleading2": HW.eTauOutputWidthSelect,
            "inputwidth2": HW.eTauOutputWidthSelect,
            "olist2": "ab",
            "otype3"  : "jJ",
            "ocut3"   : 40,
            "nleading3": HW.jJetOutputWidthSelect,
            "inputwidth3": HW.jJetOutputWidthSelect,
            "olist3": "ab",
        },
        {
            "algoname": "2DISAMB_jJ30ab_DR_eTAU_eTAU",
            "disamb" :  2,
            "minDR"   : 0,
            "maxDR"   : [28],
            "otype1"  : "eTAU",
            "ocut1"   : 30,
            "olist1": "ab",
            "nleading1": HW.eTauOutputWidthSelect,
            "inputwidth1": HW.eTauOutputWidthSelect,
            "otype2"  : "eTAU",
            "ocut2"   : 20,
            "nleading2": HW.eTauOutputWidthSelect,
            "inputwidth2": HW.eTauOutputWidthSelect,
            "olist2": "ab",
            "otype3"  : "jJ",
            "ocut3"   : 30,
            "nleading3": HW.jJetOutputWidthSelect,
            "inputwidth3": HW.jJetOutputWidthSelect,
            "olist3": "ab",
        }
        ]
        for x in DISAMB_DR_jJ_eTau_eTau_Map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'disamb', 'minDR', 'maxDR',
                             'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth1',
                             'otype2', 'ocut2', 'olist2', 'nleading2', 'inputwidth2',
                             'otype3', 'ocut3', 'olist3', 'nleading3', 'inputwidth3')
            for k in x:
                setattr(d,k,x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2, d.otype3 + d.olist3]
            toponames = []
            for bitId in range(len(d.maxDR)):
                obj1 = "-%s%s%s" % (d.otype1, str(d.ocut1), d.olist1)
                obj2 = "-%s%s%s" % (d.otype2, str(d.ocut2), d.olist2)
                obj3 = "%s%s%s"  % (d.otype3, str(d.ocut3), d.olist3)
                toponames.append("%sDISAMB-%s-%dDR%d%s%s"  % ( str(d.disamb) if d.disamb>0 else "", 
                                                               obj3, d.minDR, d.maxDR[bitId], obj1, obj2))
            
            alg = AlgConf.DisambiguationDRIncl3( name = d.algoname, inputs = inputList, outputs =  toponames )
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('InputWidth3', d.inputwidth3)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('MaxTob3', d.nleading3)
            alg.addgeneric('NumResultBits', len(toponames))

            for bitId in range(len(toponames)):
                alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype2, d.ocut2)*_et_conversion, bitId)
                alg.addvariable('MinET3', get_threshold_cut(d.otype3, d.ocut3)*_et_conversion, bitId)
                alg.addvariable('DisambDRSqrMin', d.minDR*d.minDR*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DisambDRSqrMax', d.maxDR[bitId]*d.maxDR[bitId]*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DisambDRSqr', d.disamb*d.disamb*_dr_conversion*_dr_conversion, bitId)
                
            tm.registerTopoAlgo(alg)
      

        # DR for eTAU30ab_eTAU20ab
        # output lines : '0DR25-eTAU30ab-eTAU20ab' 
        #                '0DR28-eTAU30ab-eTAU20ab' 
        DR_eTau30_eTau20_Map = [
        {
            "algoname": "DR_eTAU30ab_eTAU20ab",
            "minDR"   : 0,
            "maxDR"   : [28],
            "otype1"  : "eTAU",
            "ocut1"   : 30,
            "olist1": "ab",
            "otype2"  : "eTAU",
            "ocut2"   : 20,
            "inputwidth": HW.eTauOutputWidthSelect,
            "olist2": "ab",
        }
        ]
        for x in DR_eTau30_eTau20_Map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minDR', 'maxDR',
                             'otype1', 'ocut1', 'olist1',
                             'otype2', 'ocut2', 'olist2', 'inputwidth')
            for k in x:
                setattr(d,k,x[k])
            inputList = [d.otype1 + d.olist1]
            toponames = []
            for bitId in range(len(d.maxDR)):
                obj1 = "-%s%s%s" % (d.otype1, str(d.ocut1), d.olist1)
                obj2 = "-%s%s%s" % (d.otype2, str(d.ocut2), d.olist2)
                toponames.append("%dDR%d%s%s"  % (  d.minDR, d.maxDR[bitId], obj1, obj2))
            
            alg = AlgConf.DeltaRSqrIncl1( name = d.algoname, inputs = inputList, outputs =  toponames )
            
            alg.addgeneric('InputWidth', d.inputwidth)
            alg.addgeneric('MaxTob', HW.eTauOutputWidthSelect)
            alg.addgeneric('NumResultBits', len(toponames) )

            for bitId in range(len(toponames)):
                alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, bitId)
                alg.addvariable('MinET2', get_threshold_cut(d.otype2, d.ocut2)*_et_conversion, bitId)
                alg.addvariable('DeltaRMin', d.minDR*d.minDR*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax', d.maxDR[bitId]*d.maxDR[bitId]*_dr_conversion*_dr_conversion, bitId)
               
                
            tm.registerTopoAlgo(alg)
      
        # DISAMB 3 lists with DR cut to 2nd and 3rd lists
        algolist=[
            { "disamb": 2,
              "otype1" : "eTAU", "ocut1": 30, "olist1": "abl", "nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 20, "olist2": "abl", "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect,
              "otype3" : "jJ"  , "ocut3": 55, "olist3": "ab" , "nleading3": HW.jJetOutputWidthSelect, "inputwidth3": HW.jJetOutputWidthSelect,
              "drcutmin": 0, "drcutmax": 28}, # 2DISAMB-jJ55ab-0DR28-eTAU30abl-eTAU20abl
            { "disamb": 2,
              "otype1" : "eTAU", "ocut1": 30, "olist1": "abm", "nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 20, "olist2": "abm", "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect,
              "otype3" : "jJ"  , "ocut3": 55, "olist3": "ab" , "nleading3": HW.jJetOutputWidthSelect, "inputwidth3": HW.jJetOutputWidthSelect,
              "drcutmin": 0, "drcutmax": 28}, # 2DISAMB-jJ55ab-0DR28-eTAU30abm-eTAU20abm     
            { "disamb" : 2, 
              "otype1" : "eTAU", "ocut1": 30, "olist1": "ab", "nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 20, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect,
              "otype3" : "jJ"  , "ocut3": 55, "olist3": "ab" , "nleading3": HW.jJetOutputWidthSelect, "inputwidth3": HW.jJetOutputWidthSelect,
              "drcutmin": 4    , "drcutmax": 28}, # 2DISAMB-jJ55ab-4DR28-eTAU30ab-eTAU20ab
            { "disamb" : 2, 
              "otype1" : "eTAU", "ocut1": 30, "olist1": "ab", "nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 20, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect,
              "otype3" : "jJ"  , "ocut3": 55, "olist3": "ab" , "nleading3": HW.jJetOutputWidthSelect, "inputwidth3": HW.jJetOutputWidthSelect,
              "drcutmin": 4    , "drcutmax": 32}, # 2DISAMB-jJ55ab-4DR32-eTAU30ab-eTAU20ab
            { "disamb" : 2, 
              "otype1" : "eTAU", "ocut1": 30, "olist1": "ab", "nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 20, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect,
              "otype3" : "jJ"  , "ocut3": 55, "olist3": "ab" , "nleading3": HW.jJetOutputWidthSelect, "inputwidth3": HW.jJetOutputWidthSelect,
              "drcutmin": 10   , "drcutmax": 32}, # 2DISAMB-jJ55ab-10DR32-eTAU30ab-eTAU20ab
        ]
        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('disamb', 'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth1',
                             'otype2', 'ocut2', 'olist2', 'nleading2', 'inputwidth2',
                             'otype3', 'ocut3', 'olist3', 'nleading3', 'inputwidth3',
                             'drcutmin', 'drcutmax')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "-%s%s%s" % (d.otype1, str(d.ocut1), d.olist1)
            obj2 = "-%s%s%s" % (d.otype2, str(d.ocut2), d.olist2)
            obj3 = "%s%s%s"  % (d.otype3, str(d.ocut3), d.olist3)
            toponame = "%sDISAMB-%s-%dDR%d%s%s"  % ( str(d.disamb) if d.disamb>0 else "", obj3, d.drcutmin, d.drcutmax, obj1, obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2, d.otype3 + d.olist3]
            alg = AlgConf.DisambiguationDRIncl3( name = toponame, inputs = inputList, outputs = [ toponame ])
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('InputWidth3', d.inputwidth3)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('MaxTob3', d.nleading3)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, 0)
            alg.addvariable('MinET2', get_threshold_cut(d.otype2, d.ocut2)*_et_conversion, 0)
            alg.addvariable('MinET3', get_threshold_cut(d.otype3, d.ocut3)*_et_conversion, 0)
            alg.addvariable('DisambDRSqrMin', d.drcutmin*d.drcutmin*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DisambDRSqrMax', d.drcutmax*d.drcutmax*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DisambDRSqr', d.disamb*d.disamb*_dr_conversion*_dr_conversion, 0)
            tm.registerTopoAlgo(alg)


        #TLA deta
        algoList = [
            { "minDeta": 0,  "maxDeta": 20, "otype" : "jJ",  "ocut1" : 90,  "olist" : "s",
              "nleading1" : 1, "inputwidth1": HW.jJetOutputWidthSort, "ocut2" : 0, "nleading2": 2}, #0DETA20-jJ90s1-jJs2                        
            { "minDeta":  0, "maxDeta"  : 24, "otype" : "eTAU" , "olist"  : "s", "inputwidth1": HW.eTauOutputWidthSort, 
              "ocut1"  : 30, "nleading1":  2,
              "ocut2"  : 12, "nleading2":  2}, #0DETA24_eTAU30s2_eTAU12s2
        ]        
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('minDeta', 'maxDeta', 'otype', 'ocut1', 'olist', 'nleading1', 'inputwidth1', 'ocut2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            toponame = "%iDETA%i-%s%s%s%s-%s%s%s%s"  % (d.minDeta, d.maxDeta,
                                                        d.otype, d.ocut1 if d.ocut1 > 0 else "", d.olist, d.nleading1,
                                                        d.otype, d.ocut2 if d.ocut2 > 0 else "", d.olist, d.nleading2)
            log.debug("Define %s", toponame)
            inputList = d.otype + d.olist
            alg = AlgConf.DeltaEtaIncl1( name = toponame, inputs = inputList, outputs = toponame )
            alg.addgeneric('InputWidth', d.inputwidth1)
            alg.addgeneric('MaxTob', d.nleading2)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1', get_threshold_cut(d.otype, d.ocut1)*_et_conversion, 0)
            alg.addvariable('MinET2', get_threshold_cut(d.otype, d.ocut2)*_et_conversion, 0)
            alg.addvariable('MinDeltaEta', d.minDeta*_eta_conversion, 0)
            alg.addvariable('MaxDeltaEta', d.maxDeta*_eta_conversion, 0)
            tm.registerTopoAlgo(alg)

        # ATR-30401 and ATR-32084
        # topoitems will be the following:
        # 0DPHI10_jXE40delay_jJ40s
        # 0DPHI99_jXE40delay_jJ40s
        DPHI_jXE40delay_jJ40s_map = [
        {  
            "algoname"  : "DPHI_jXE40delay%s_jJ40s",
            "Delay1"    : [1,2,3],
            "Delay2"    : 0,
            "InputWidth1": HW.metOutputWidth,
            "InputWidth2": HW.jJetOutputWidthSort,
            "MaxTob1"   : 1,
            "MaxTob2"   : 6,
            "MinET1"    : 40*_et_conversion,
            "MinET2"    : 40*_et_conversion,
            "MinDeltaPhi": 0*_phi_conversion,
            "phi_thresholds": [10, 99],
        }
        ]

        for x in DPHI_jXE40delay_jJ40s_map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'Delay1', 'Delay2', 'InputWidth1', 'InputWidth2',
                             'MaxTob1', 'MaxTob2', 'MinET1', 'MinET2', 'MinDeltaPhi', 'phi_thresholds')
            for k in x:
                setattr(d,k,x[k])
            inputList = ['jXEs', 'jJs']

            for delay1 in d.Delay1:
                toponames = []
                delaySuffix =  str(delay1) if delay1 > 1 else ""
                for bitId in range(len(d.phi_thresholds)):
                    toponames.append("0DPHI%d-jXE40delay%s-jJ40s"  % (d.phi_thresholds[bitId], delaySuffix) )

                algName = d.algoname %  delaySuffix
                alg = AlgConf.DeltaPhiIncl2( name = algName, inputs = inputList, outputs =  toponames )

                alg.addgeneric('InputWidth1', d.InputWidth1)
                alg.addgeneric('InputWidth2', d.InputWidth2)
                alg.addgeneric('MaxTob1', d.MaxTob1)
                alg.addgeneric('MaxTob2', d.MaxTob2)
                alg.addgeneric('NumResultBits', len(toponames) )
                alg.addgeneric('Delay1', delay1)
                alg.addgeneric('Delay2', d.Delay2)


                for bitId in range(len(toponames)):
                    alg.addvariable('MinET1', d.MinET1, bitId)
                    alg.addvariable('MinET2', d.MinET2, bitId)
                    alg.addvariable('MinDeltaPhi', d.MinDeltaPhi, bitId)
                    alg.addvariable('MaxDeltaPhi', d.phi_thresholds[bitId]*_phi_conversion, bitId)
                tm.registerTopoAlgo(alg)

        # ATR-31097
        TeAsymmetry_map = [
        {  
            "algoname"  : "TeAsymmetry-jTENoSort",
            "deltaAbsMin" : [5, 5, 3, 7],
            "asymFactor" : [1, 1.05, 1, 0.9],
            "asymOffset" : [-5, -24, -5, -7],
            "maxTeProduct": [200, 200, 200, 200],
        }
        ]

        for x in TeAsymmetry_map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'deltaAbsMin', 'asymFactor', 'asymOffset', 'maxTeProduct')
            for k in x:
                setattr(d,k,x[k])
            inputList = ['jTENoSort_1BC']
            # To update with correct names
            toponames = []
            for bitId in range(len(d.deltaAbsMin)):
                toponames.append("TeAsymmetry_jTENoSort_ParamSet%d"  % (bitId))
            
            alg = AlgConf.TeAsymmetry( name = d.algoname, inputs = inputList, outputs =  toponames )
            alg.addgeneric('InputWidth', 1)
            alg.addgeneric('NumResultBits', len(toponames))
            alg.addgeneric("Delay", 1)

            for bitId in range(len(toponames)):
                #Update with the correct algo parameters
                alg.addvariable('deltaAbsMin', d.deltaAbsMin[bitId]*_et_conversion, bitId)  #Units of 100MeV
                alg.addvariable('asymFactor', d.asymFactor[bitId], bitId)
                alg.addvariable('asymOffset', d.asymOffset[bitId]*_et_conversion, bitId)   #Units of 100MeV
                alg.addvariable('maxTeProduct', d.maxTeProduct[bitId]*_et_conversion*_et_conversion, bitId)  #Units of (100MeV)^2
            tm.registerTopoAlgo(alg)
        
        #TeATIME
        TeATIME_map = [
        {  
            "algoname"     : "TeATIME-jTENoSort",
            "teFlavor"     : [2,1,2,3], # 0 = off, 1 = full jTE, 2 = central jTE, 3 = forward jTE (A+C side)
            "combination"  : [0,0,0,1], # 0 = require both, 1 = require factor, 2 = require offset, 3 = require any of the two criteria
            "nextBcOffset" : [-2.5,10,10,10], # offset added to upcoming BC's jTE (in GeV, fractional values down to 100 MeV are possible). 
                                              # The 'offset' criterion is considered as passed if this sum exceeds the current BC's jTE value,
                                              # i.e., to require a minimum *increase* in jTE in the next BC this should be a *negative* value!
            "nextBcFactor" : [0.9,2.5,2.5,2.5] # factor by which upcoming BC's jTE must be larger than current BC's jTE value
        }
        ]

        for x in TeATIME_map:
            class d: # type: ignore
                __slots__ = ('algoname', 'teFlavor', 'combination', 'nextBcOffset', 'nextBcFactor')
            for k in x:
                setattr(d,k,x[k])
            inputList = ['jTENoSort_1BC']
            # To update with correct names
            toponames = []
            for bitId in range(4):
                toponames.append("TeATIME_jTENoSort_ParamSet%d"  % (bitId))
            
            
            alg = AlgConf.TeATIME( name = d.algoname, inputs = inputList, outputs =  toponames )
            alg.addgeneric('InputWidth', 1)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(d.teFlavor)):
                alg.addvariable('algoLogic', d.teFlavor[bitId] + ( d.combination[bitId] << 2 ) , bitId)  #two bit fields (of 2 bits each) merged into on FW register
                alg.addvariable('nextBcFactor', round(d.nextBcFactor[bitId] * pow(2,8)), bitId) #integer representation of factor corresponding to 8 fractional bits
                alg.addvariable('nextBcOffset', round(d.nextBcOffset[bitId] * _et_conversion), bitId) #converted to units of 100MeV
            tm.registerTopoAlgo(alg)



        # DISAMB 3 lists with DR cut to 2nd and 3rd lists
        algolist=[
            { "disamb": 2,
              "otype1" : "eTAU",  "ocut1": 20, "olist1": "ab","nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 12, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect,
              "otype3" : "jJ", "ocut3": 40, "olist3": "ab", "nleading3": HW.jJetOutputWidthSelect, "inputwidth3": HW.jJetOutputWidthSelect,
              "drcutmin": 0, "drcutmax": 10}, # 2DISAMB-jJ40ab-0DR10-eTAU20ab-eTAU12ab
        ]
        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('disamb', 'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth1',
                             'otype2', 'ocut2', 'olist2', 'nleading2', 'inputwidth2',
                             'otype3', 'ocut3', 'olist3', 'nleading3', 'inputwidth3',
                             'drcutmin', 'drcutmax')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "-%s%s%s" % (d.otype1, str(d.ocut1), d.olist1)
            obj2 = "-%s%s%s" % (d.otype2, str(d.ocut2), d.olist2)
            obj3 = "%s%s%s"  % (d.otype3, str(d.ocut3), d.olist3)
            toponame = "%sDISAMB-%s-%dDR%d%s%s"  % ( str(d.disamb) if d.disamb>0 else "", obj3, d.drcutmin, d.drcutmax, obj1, obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2, d.otype3 + d.olist3]
            alg = AlgConf.DisambiguationDRIncl3( name = toponame, inputs = inputList, outputs = [ toponame ])
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('InputWidth3', d.inputwidth3)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('MaxTob3', d.nleading3)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, 0)
            alg.addvariable('MinET2', get_threshold_cut(d.otype2, d.ocut2)*_et_conversion, 0)
            alg.addvariable('MinET3', get_threshold_cut(d.otype3, d.ocut3)*_et_conversion, 0)
            alg.addvariable('DisambDRSqrMin', d.drcutmin*d.drcutmin*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DisambDRSqrMax', d.drcutmax*d.drcutmax*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DisambDRSqr', d.disamb*d.disamb*_dr_conversion*_dr_conversion, 0)
            tm.registerTopoAlgo(alg)

        # AnomalyDetectionBDT muon-only anomaly trigger
        algo = {
            "algoname": "ADBDT-MU0s",
            "otype1": "MU", "olist1": "s", "inputwidth1": 6, "nleading1": 3,
            "WPList": ["Tight", "Loose"],
            "MinET1": 0,#min muon1 threshold in GeV, the 0 sets the muon to the min TOB threshold avaiable in Topo 
            "MinET2": 0,#min muon2 threshold in Gev 
            "ScoreThreshold": [40, 25]
        }
        class d:  # type: ignore
            __slots__ = ('algoname', 'otype1', 'olist1', 'inputwidth1', 'nleading1',
                         'WPList',
                         'MinET1', 'MinET2',
                         'ScoreThreshold')
        
        for k in algo:
            setattr(d, k, algo[k])
            
        toponames = [f"ADBDT-{d.nleading1}{d.otype1}0{d.olist1}-{wp}" for wp in d.WPList]
            
        alg = AlgConf.AnomalyDetectionBDT(
            name=d.algoname,
            inputs=[d.otype1 + d.olist1],
            outputs=toponames
        )
        alg.addgeneric('NumResultBits', 2)
        alg.addgeneric('MaxTob',d.nleading1) #Set the number of Tobs to 3 (leading pT 3 muons)
        alg.addvariable('MinET1',d.MinET1*_et_conversion)
        alg.addvariable('MinET2',d.MinET2*_et_conversion)
        for bitId in range(len(toponames)):
            alg.addvariable('ScoreThreshold', d.ScoreThreshold[bitId], bitId)
        tm.registerTopoAlgo(alg)
            
        #VAE-based anomaly trigger
        # output lines: 'ADVAE2A-6jJ0s-4eTAU0s-4MU0s-jXE0s-Tight', 
        #               'ADVAE2A-6jJ0s-4eTAU0s-4MU0s-jXE0s-Loose'
        #Ordering -
        #1. jJetx6
        #2. eTAUx4
        #3. MUx4
        #4. jXEx1
        algo = {
              "algoname" : "ADVAE2A-jJ0s-eTAU0s-MU0s-jXE0s",
              "otype1" : "jJ", "olist1": "s", "inputwidth1": 6, "nleading1": 6,
              "otype2" : "eTAU", "olist2": "s", "inputwidth2": 6, "nleading2": 4,
              "otype3" : "MU", "olist3": "s", "inputwidth3": 6, "nleading3": 4,
              "otype4" : "jXE", "olist4": "s", "inputwidth4": 1, "nleading4": 1,
              "WPList" : ["Tight", "Loose"],
              "MinET1" : 15, # jJets in GeV (= 100MeV steps after conversion)
              "MinET2" : 0, # eTaus
              "MinET3" : 0, # muons
              "MinET4" : 0, # jXE
              "ScaleSqr1" : [128, 128], #corresponds to Tight and Loose WPs
              "ScaleSqr2" : [128, 128], #corresponds to Tight and Loose WPs
              "ScaleSqr3" : [128, 128], #corresponds to Tight and Loose WPs
              "AnomalyScoreThresh" : [1521991, 1333204], #corresponds to Tight and Loose WPs (500Hz,1kHz est.)
        }
        class d:  # type: ignore
            __slots__ = ('algoname', 'otype1', 'olist1', 'inputwidth1', 'nleading1',
                         'otype2', 'olist2', 'inputwidth2', 'nleading2',
                         'otype3', 'olist3', 'inputwidth3', 'nleading3',
                         'otype4', 'olist4', 'inputwidth4', 'nleading4',
                         'WPList',
                         'MinET1', 'MinET2', 'MinET3', 'MinET4',
                         'ScaleSqr1', 'ScaleSqr2', 'ScaleSqr3',
                         'AnomalyScoreThresh')
        for k in algo:
            setattr (d, k, algo[k])
        toponames = []
        for WP in d.WPList:
            toponames.append("ADVAE2A-%s%s0%s-%s%s0%s-%s%s0%s-%s0%s-%s" %(str(d.nleading1), d.otype1, d.olist1, 
                                                                          str(d.nleading2), d.otype2, d.olist2, 
                                                                          str(d.nleading3), d.otype3, d.olist3, 
                                                                          d.otype4, d.olist4, 
                                                                          WP) )
        inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2, d.otype3 + d.olist3, d.otype4 + d.olist4]
        alg = AlgConf.ADVAE_2A( name = d.algoname, inputs = inputList, outputs = toponames)
        alg.addgeneric('InputWidth1', d.inputwidth1)
        alg.addgeneric('InputWidth2', d.inputwidth2)
        alg.addgeneric('InputWidth3', d.inputwidth3)
        alg.addgeneric('InputWidth4', d.inputwidth4)
        alg.addgeneric('MaxTob1', d.nleading1)
        alg.addgeneric('MaxTob2', d.nleading2)
        alg.addgeneric('MaxTob3', d.nleading3)
        alg.addgeneric('MaxTob4', d.nleading4)
        alg.addgeneric('NumResultBits', len(toponames))
        alg.addgeneric('ADVAEVersion', 1)
        alg.addvariable('MinET1', d.MinET1 * _et_conversion)
        alg.addvariable('MinET2', d.MinET2 * _et_conversion)
        alg.addvariable('MinET3', d.MinET3 * _et_conversion)
        alg.addvariable('MinET4', d.MinET4 * _et_conversion)
        for bitId in range(len(toponames)):
            alg.addvariable('ScaleSqr1', d.ScaleSqr1[bitId], bitId)
            alg.addvariable('ScaleSqr2', d.ScaleSqr2[bitId], bitId)
            alg.addvariable('ScaleSqr3', d.ScaleSqr3[bitId], bitId)
            alg.addvariable('AnomalyScoreThresh', d.AnomalyScoreThresh[bitId], bitId)
        tm.registerTopoAlgo(alg)


        #ARTEMIS (VAE-based anomaly trigger)
        # output lines: 'ARTEMIS-6jJ0s-4eTAU0s-4MU0s-jXE0s-Tight', 
        #               'ARTEMIS-6jJ0s-4eTAU0s-4MU0s-jXE0s-Loose'
        #Ordering -
        #1. jJetx6
        #2. eTAUx4
        #3. MUx4
        #4. jXEx1
        algo = {
              "algoname" : "ARTEMIS-jJ0s-eTAU0s-MU0s-jXE0s",
              "otype1" : "jJ", "olist1": "s", "inputwidth1": 6, "nleading1": 6,
              "otype2" : "eTAU", "olist2": "s", "inputwidth2": 6, "nleading2": 4,
              "otype3" : "MU", "olist3": "s", "inputwidth3": 6, "nleading3": 4,
              "otype4" : "jXE", "olist4": "s", "inputwidth4": 1, "nleading4": 1,
              "WPList" : ["Tight", "Loose"],
              "MinET1" : 15, # jJets in GeV (= 100MeV steps after conversion)
              "MinET2" : 0, # eTaus
              "MinET3" : 0, # muons
              "MinET4" : 0, # jXE
              "MaxET1" : 90, # jJets in GeV (= 100MeV steps after conversion)
              "MaxET2" : 0, # eTaus
              "MaxET3" : 0, # muons
              "MaxET4" : 0, # jXE
              "AnomalyScoreThresh" : [1521991, 1333204], #corresponds to Tight and Loose WPs (500Hz,1kHz est.)
        }
        class d:  # type: ignore
            __slots__ = ('algoname', 'otype1', 'olist1', 'inputwidth1', 'nleading1',
                         'otype2', 'olist2', 'inputwidth2', 'nleading2',
                         'otype3', 'olist3', 'inputwidth3', 'nleading3',
                         'otype4', 'olist4', 'inputwidth4', 'nleading4',
                         'WPList',
                         'MinET1', 'MinET2', 'MinET3', 'MinET4',
                         'MaxET1', 'MaxET2', 'MaxET3', 'MaxET4',
                         'AnomalyScoreThresh')
        for k in algo:
            setattr (d, k, algo[k])
        toponames = []
        for WP in d.WPList:
            toponames.append("ARTEMIS-%s%s0%s-%s%s0%s-%s%s0%s-%s0%s-%s" %(str(d.nleading1), d.otype1, d.olist1, 
                                                                          str(d.nleading2), d.otype2, d.olist2, 
                                                                          str(d.nleading3), d.otype3, d.olist3, 
                                                                          d.otype4, d.olist4, 
                                                                          WP) )
        inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2, d.otype3 + d.olist3, d.otype4 + d.olist4]
        alg = AlgConf.ARTEMIS_2A( name = d.algoname, inputs = inputList, outputs = toponames)
        alg.addgeneric('InputWidth1', d.inputwidth1)
        alg.addgeneric('InputWidth2', d.inputwidth2)
        alg.addgeneric('InputWidth3', d.inputwidth3)
        alg.addgeneric('InputWidth4', d.inputwidth4)
        alg.addgeneric('MaxTob1', d.nleading1)
        alg.addgeneric('MaxTob2', d.nleading2)
        alg.addgeneric('MaxTob3', d.nleading3)
        alg.addgeneric('MaxTob4', d.nleading4)
        alg.addgeneric('NumResultBits', len(toponames))
        alg.addgeneric('Version', 1)
        alg.addvariable('MinET1', d.MinET1 * _et_conversion)
        alg.addvariable('MinET2', d.MinET2 * _et_conversion)
        alg.addvariable('MinET3', d.MinET3 * _et_conversion)
        alg.addvariable('MinET4', d.MinET4 * _et_conversion)
        alg.addvariable('MaxET1', d.MinET1 * _et_conversion)
        alg.addvariable('MaxET2', d.MinET2 * _et_conversion)
        alg.addvariable('MaxET3', d.MinET3 * _et_conversion)
        alg.addvariable('MaxET4', d.MinET4 * _et_conversion)
        for bitId in range(len(toponames)):
            alg.addvariable('AnomalyScoreThresh', d.AnomalyScoreThresh[bitId], bitId)
        tm.registerTopoAlgo(alg)


        algolist=[
            { "disamb": 2,
              "otype1" : "eTAU",  "ocut1": 20, "olist1": "ab","nleading1": HW.eTauOutputWidthSelect, "inputwidth1": HW.eTauOutputWidthSelect,
              "otype2" : "eTAU", "ocut2": 12, "olist2": "ab", "nleading2": HW.eTauOutputWidthSelect, "inputwidth2": HW.eTauOutputWidthSelect,
              "otype3" : "jJ", "ocut3": 30, "olist3": "ab", "nleading3": HW.jJetOutputWidthSelect, "inputwidth3": HW.jJetOutputWidthSelect,
              "drcutmin": 0, "drcutmax": 10}, # 2DISAMB-jJ30ab-0DR10-eTAU20ab-eTAU12ab
        ]
        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('disamb', 'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth1',
                             'otype2', 'ocut2', 'olist2', 'nleading2', 'inputwidth2',
                             'otype3', 'ocut3', 'olist3', 'nleading3', 'inputwidth3',
                             'drcutmin', 'drcutmax')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "-%s%s%s" % (d.otype1, str(d.ocut1), d.olist1)
            obj2 = "-%s%s%s" % (d.otype2, str(d.ocut2), d.olist2)
            obj3 = "%s%s%s"  % (d.otype3, str(d.ocut3), d.olist3)
            toponame = "%sDISAMB-%s-%dDR%d%s%s"  % ( str(d.disamb) if d.disamb>0 else "", obj3, d.drcutmin, d.drcutmax, obj1, obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2, d.otype3 + d.olist3]
            alg = AlgConf.DisambiguationDRIncl3( name = toponame, inputs = inputList, outputs = [ toponame ])
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('InputWidth3', d.inputwidth3)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('MaxTob3', d.nleading3)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1', get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, 0)
            alg.addvariable('MinET2', get_threshold_cut(d.otype2, d.ocut2)*_et_conversion, 0)
            alg.addvariable('MinET3', get_threshold_cut(d.otype3, d.ocut3)*_et_conversion, 0)
            alg.addvariable('DisambDRSqrMin', d.drcutmin*d.drcutmin*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DisambDRSqrMax', d.drcutmax*d.drcutmax*_dr_conversion*_dr_conversion, 0)
            alg.addvariable('DisambDRSqr', d.disamb*d.disamb*_dr_conversion*_dr_conversion, 0)
            tm.registerTopoAlgo(alg)

        # jINVM + DPHI
        NFFDphimap = [
            { "itemNameMinInvm": 400, "minInvm": 640 , "minDphi": 0, "maxDphiList": [26, 24, 22, 20],
                         "otype1" : "AjJ", "ocut1" : 60, "olist1" : "s", "nleading1" : 6, "inputwidth": HW.jJetOutputWidthSort,
                         "otype2" : "AjJ", "ocut2" : 50, "olist2" : "s", "nleading2" : 6 }
        ]
        for x in NFFDphimap:
            class d:  # type: ignore
                __slots__ = ('itemNameMinInvm', 'minInvm', 'minDphi', 'maxDphiList',
                             'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth',
                             'otype2', 'ocut2', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for maxDphi in d.maxDphiList:
                toponames.append ("%iINVM-%iDPHI%i-%s%s%s%s-%s%s%s%s"  % (d.itemNameMinInvm, d.minDphi, maxDphi,
                                                                 d.otype1, str(d.ocut1) , d.olist1, str(d.nleading1) if d.olist1=="s" else "",
                                                                 d.otype2, str(d.ocut2) , d.olist2, str(d.nleading2) if d.olist2=="s" else ""))
            alg = AlgConf.InvariantMassDeltaPhiInclusive2( name = 'jINVM_DPHI', inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth1', d.inputwidth)
            alg.addgeneric('InputWidth2', d.inputwidth)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits',  len(toponames))
            for bitid,maxDphi in enumerate(d.maxDphiList):
                alg.addvariable('MinET1',      get_threshold_cut(d.otype1, d.ocut1)*_et_conversion , bitid)
                alg.addvariable('MinET2',      get_threshold_cut(d.otype2, d.ocut2)*_et_conversion , bitid)
                alg.addvariable('MinMSqr',     d.minInvm*d.minInvm *_et_conversion*_et_conversion , bitid)
                alg.addvariable('MaxMSqr',     _no_m_upper_threshold , bitid)  # no upper threshold
                alg.addvariable('MinDeltaPhi', d.minDphi*_phi_conversion , bitid)
                alg.addvariable('MaxDeltaPhi', maxDphi*_phi_conversion, bitid)
            tm.registerTopoAlgo(alg)


        # CF
        algoList = [
            {  "itemNameMinInvm": 400, "minInvm": 640, "otype1" : "AjJ", "ocut1": 60, "olist1" : "s", "nleading1" : 6, "inputwidth1": HW.jJetOutputWidthSort,
               "otype2" : "AjJ", "ocut2": 50, "olist2" : "s", "nleading2" : 6, "inputwidth2": HW.jJetOutputWidthSort, "applyEtaCut":1,
               "minEta1": 0 ,"maxEta1": 32 , "minEta2": 30 ,"maxEta2": 49 , }, #400INVM-AjJ60s6pETA32-AjJ50s6p30ETA49
        ]
        for x in algoList:
            class d:  # type: ignore
                __slots__ = ('itemNameMinInvm', 'minInvm', 'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth1',
                             'otype2', 'ocut2', 'olist2', 'nleading2', 'inputwidth2',
                             'applyEtaCut', 'minEta1', 'maxEta1', 'minEta2', 'maxEta2')     
            for k in x:  
                setattr (d, k, x[k])
            obj1 = "%s%s%sp%sETA%i"  % (d.otype1, str(d.ocut1), d.olist1 + (str(d.nleading1) if d.olist1.find('s')>=0 else ""),str(d.minEta1) if d.minEta1>0 else "", d.maxEta1)
            obj2 = "-%s%s%sp%sETA%i"  % (d.otype2, str(d.ocut2), d.olist2 + (str(d.nleading2) if d.olist2.find('s')>=0 else ""),str(d.minEta2) if d.minEta2>0 else "", d.maxEta2)
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist2]
            toponame = "%iINVM-%s%s"   % (d.itemNameMinInvm, obj1, obj2)
            alg = AlgConf.InvariantMassInclusive2( name = toponame, inputs = inputList, outputs = toponame)
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', 1)
            if (d.applyEtaCut>0):
                alg.addgeneric('ApplyEtaCut', d.applyEtaCut)
            alg.addvariable('MinET1',  get_threshold_cut(d.otype1, d.ocut1)*_et_conversion )
            alg.addvariable('MinET2',  get_threshold_cut(d.otype2, d.ocut2)*_et_conversion )
            alg.addvariable('MinMSqr', d.minInvm*d.minInvm*_et_conversion*_et_conversion )
            alg.addvariable('MaxMSqr', _no_m_upper_threshold )
            if (d.applyEtaCut>0):
                alg.addvariable('MinEta1', d.minEta1*_eta_conversion )
                alg.addvariable('MaxEta1', d.maxEta1*_eta_conversion )
                alg.addvariable('MinEta2', d.minEta2*_eta_conversion )
                alg.addvariable('MaxEta2', d.maxEta2*_eta_conversion )
            tm.registerTopoAlgo(alg)

        # jINVM
        NFFmap = [
            { "itemNameMinInvmList": [300, 400, 500, 700], "minInvmList": [480, 640, 800, 1100] ,
                         "otype1" : "AjJ", "ocut1" : 60, "olist1" : "s", "nleading1" : 6, "inputwidth": HW.jJetOutputWidthSort,
                         "otype2" : "AjJ", "ocut2" : 50, "olist2" : "s", "nleading2" : 6 }
        ]
        for x in NFFmap:
            class d:  # type: ignore
                __slots__ = ('itemNameMinInvmList', 'minInvmList',
                             'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth',
                             'otype2', 'ocut2', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for minInvm in d.itemNameMinInvmList:
                toponames.append ("%iINVM-%s%s%s%s-%s%s%s%s"  % (minInvm,
                                                                 d.otype1, str(d.ocut1) , d.olist1, str(d.nleading1) if d.olist1=="s" else "",
                                                                 d.otype2, str(d.ocut2) , d.olist2, str(d.nleading2) if d.olist2=="s" else ""))
            alg = AlgConf.InvariantMassInclusive2( name = 'jINVM', inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth1', d.inputwidth)
            alg.addgeneric('InputWidth2', d.inputwidth)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits',  len(toponames))
            for bitid,minInvm in enumerate(d.minInvmList):
                alg.addvariable('MinET1',  get_threshold_cut(d.otype1, d.ocut1)*_et_conversion , bitid)
                alg.addvariable('MinET2',  get_threshold_cut(d.otype2, d.ocut2)*_et_conversion , bitid)
                alg.addvariable('MinMSqr', minInvm*minInvm*_et_conversion*_et_conversion , bitid)
                alg.addvariable('MaxMSqr', _no_m_upper_threshold , bitid)  # no upper threshold
            tm.registerTopoAlgo(alg)

        # jINVM_NFF
        NFFmap = [
            { "itemNameMinInvmList": [300, 400, 500, 700], "minInvmList": [480, 640, 800, 1100] ,
                         "otype1" : "jJ", "ocut1" : 60, "olist1" : "s", "nleading1" : 6, "inputwidth": HW.jJetOutputWidthSort,
                         "otype2" : "AjJ", "ocut2" : 50, "olist2" : "s", "nleading2" : 6 }
        ]
        for x in NFFmap:
            class d:  # type: ignore
                __slots__ = ('itemNameMinInvmList', 'minInvmList',
                             'otype1', 'ocut1', 'olist1', 'nleading1', 'inputwidth',
                             'otype2', 'ocut2', 'olist2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + d.olist1]
            toponames=[]
            for minInvm in d.itemNameMinInvmList:
                toponames.append ("%iINVM-%s%s%s%s-%s%s%s%s"  % (minInvm, 
                                                                 d.otype1, str(d.ocut1) , d.olist1, str(d.nleading1) if d.olist1=="s" else "",
                                                                 d.otype2, str(d.ocut2) , d.olist2, str(d.nleading2) if d.olist2=="s" else ""))
            alg = AlgConf.InvariantMassInclusive2( name = 'jINVM_NFF', inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth1', d.inputwidth)
            alg.addgeneric('InputWidth2', d.inputwidth)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits',  len(toponames))
            for bitid,minInvm in enumerate(d.minInvmList):
                alg.addvariable('MinET1',  get_threshold_cut("CjJ", d.ocut1)*_et_conversion , bitid)
                alg.addvariable('MinET2',  get_threshold_cut(d.otype2, d.ocut2)*_et_conversion , bitid)
                alg.addvariable('MinMSqr', minInvm*minInvm*_et_conversion*_et_conversion , bitid)
                alg.addvariable('MaxMSqr', _no_m_upper_threshold , bitid)  # no upper threshold
            tm.registerTopoAlgo(alg)

        #ATR-19355  
        # Parameter ordering
        # 1. MinEt
        # 2. MinMSqr
        # 3. MaxMSqr
        toponame = "0INVM10-3MU3Vab"
        log.debug("Define %s", toponame)
        inputList = 'MU3Vab'
        alg = AlgConf.InvariantMassThreeTOBsIncl1( name = toponame, inputs = inputList, outputs = toponame )
        alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
        alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
        alg.addgeneric('NumResultBits', 1)
        alg.addvariable('MinET1',      0*_et_conversion)
        alg.addvariable('MinMSqr',     0*_et_conversion*_et_conversion)
        alg.addvariable('MaxMSqr', 10*10*_et_conversion*_et_conversion)
        tm.registerTopoAlgo(alg)

        toponame = "0INVM10-3MU3VFab"
        log.debug("Define %s", toponame)
        inputList = 'MU3VFab'
        alg = AlgConf.InvariantMassThreeTOBsIncl1( name = toponame, inputs = inputList, outputs = toponame )
        alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
        alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
        alg.addgeneric('NumResultBits', 1)
        alg.addvariable('MinET1',      0*_et_conversion)
        alg.addvariable('MinMSqr',     0*_et_conversion*_et_conversion)
        alg.addvariable('MaxMSqr', 10*10*_et_conversion*_et_conversion)
        tm.registerTopoAlgo(alg)

        #ATR-19638, 3muon, not all with the same charge
        toponame = "0INVM10C-3MU3Vab"
        log.debug("Define %s", toponame)
        inputList = 'MU3Vab'
        alg = AlgConf.InvariantMassThreeTOBsIncl1Charge( name = toponame, inputs = inputList, outputs = toponame )
        alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
        alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
        alg.addgeneric('NumResultBits', 1)
        alg.addvariable('MinET1',      0*_et_conversion)
        alg.addvariable('MinMSqr',     0*_et_conversion*_et_conversion)
        alg.addvariable('MaxMSqr', 10*10*_et_conversion*_et_conversion)
        tm.registerTopoAlgo(alg)


        # LFV
        # output lines: 0INVM10-0DR15-eEM10abl-MU8Fab, 0INVM10-0DR15-eEM15abl-MU5VFab
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        # 5. MinEta1
        # 6. MaxEta1
        # 7. MinEta2
        # 8. MaxEta2
        # 9. DeltaRMin
        # 10. DeltaRMax
        INVM_DR_eEM_MU_Map = [{
            "algoname": "INVM_DR_eEM_MU",
            "minInvm" : 0,
            "maxInvm" : 10,
            "minDR"   : 0,
            "maxDR"   : 15,
            "otype1"  : "eEM",
            "ocut1"   : [10,15],
            "olist1": "abl",
            "otype2"  : "MU",
            "olist2": ["Fab","VFab"],
            "ocut2"   : 5,
            "ocut2Offset" : [3,0]
            
        }]

        for x in INVM_DR_eEM_MU_Map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDR', 'maxDR',
                             'otype1', 'ocut1', 'olist1',
                             'otype2', 'ocut2', 'olist2', 'ocut2Offset')
            for k in x:
                setattr(d,k,x[k])
            inputList = [d.otype1 + d.olist1, d.otype2 + str(d.ocut2) + d.olist2[1] ]
            toponames = []
            for bitId in range(len(d.ocut1)):
                obj1 = "-%s%s%s" % (d.otype1, str( d.ocut1[bitId] ) , d.olist1)
                obj2 = "-%s%s%s" % (d.otype2, str( d.ocut2 + d.ocut2Offset[bitId] ) , d.olist2[bitId])
                toponames.append("%dINVM%d-%dDR%d%s%s"  % (  d.minInvm, d.maxInvm, d.minDR, d.maxDR, obj1, obj2))
            
            alg = AlgConf.InvariantMassInclusiveDeltaRSqrIncl2( name = d.algoname, inputs = inputList, outputs =  toponames )
            
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSelect)
            alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob1', HW.eEmOutputWidthSelect)
            alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('ApplyEtaCut',  1)

            alg.addgeneric('NumResultBits', len(toponames) )

            for bitId in range(len(toponames)):
                alg.addvariable('MinET1',      get_threshold_cut(d.otype1, d.ocut1[bitId])*_et_conversion, bitId)
                alg.addvariable('MinET2',      (d.ocut2 + d.ocut2Offset[bitId])*_et_conversion, bitId) 
                alg.addvariable('MinMSqr',     d.minInvm*d.minInvm*_et_conversion*_et_conversion, bitId)
                alg.addvariable('MaxMSqr',     d.maxInvm*d.maxInvm*_et_conversion*_et_conversion, bitId)
                alg.addvariable('MinEta1',     0*_eta_conversion, bitId)
                alg.addvariable('MaxEta1',    49*_eta_conversion, bitId)
                alg.addvariable('MinEta2',     0*_eta_conversion, bitId)
                alg.addvariable('MaxEta2',    49*_eta_conversion, bitId)
                alg.addvariable('DeltaRMin',   d.minDR*d.minDR*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax',   d.maxDR*d.maxDR*_dr_conversion*_dr_conversion, bitId)
                               
                
            tm.registerTopoAlgo(alg)
      
        #ATR-18824 ZAFB-DPHI
        # TODO: update with fwd electrons
        # Parameter ordering
        # 1. MinEt1
        # 2. MinEt2
        # 3. MinMSqr
        # 4. MaxMSqr
        # 5. MinEta1
        # 6. MaxEta1
        # 7. MinEta2
        # 8. MaxEta2
        # 9. MinDeltaPhi
        # 10. MaxDeltaPhi
        ZAFBDphimap = [
            { "minInvm": 60 , "minDphiList": [4, 25], "maxDphi": 32, "minEta2": 25, "maxEta2": 49,
              "inputwidth1": HW.eEmOutputWidthSelect, "otype1" : "eEM", "ocut1" : 18, "olist1" : "abm",
              "nleading1" : HW.eEmOutputWidthSelect, "inputwidth2": HW.jEmOutputWidthSort,  "ocut2" : 20, "ocut3" : 25, "nleading2" : 6 }
        ]
        for x in ZAFBDphimap:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'minDphiList', 'maxDphi', 'minEta2', 'maxEta2',
                             'inputwidth1', 'otype1', 'ocut1', 'olist1', 'nleading1',
                             'inputwidth2', 'ocut2', 'ocut3', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            inputList = [d.otype1 + d.olist1, 'jEMs25ETA49']
            toponames=[]
            for minDphi in d.minDphiList:
                toponames.append ("%iINVM-%02dDPHI%i-%s%s%s%s-jEM%ss%s%iETA%i"  % (d.minInvm, minDphi, d.maxDphi,
                                                                                     d.otype1, str(d.ocut1) , d.olist1, str(d.nleading1) if d.olist1=="s" else "",
                                                                                     str(d.ocut3) , str(d.nleading2) , d.minEta2, d.maxEta2))
            alg = AlgConf.InvariantMassDeltaPhiInclusive2( name = 'ZAFB_DPHI', inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits',  len(toponames))
            alg.addgeneric('ApplyEtaCut', 1)
            for bitid,minDphi in enumerate(d.minDphiList):
                alg.addvariable('MinET1',  get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, bitid)
                alg.addvariable('MinET2',  get_threshold_cut('jEM', d.ocut3)*_et_conversion, bitid)
                alg.addvariable('MinMSqr', d.minInvm*d.minInvm*_et_conversion*_et_conversion, bitid)
                alg.addvariable('MaxMSqr', _no_m_upper_threshold, bitid)
                alg.addvariable('MinEta1',  0*_eta_conversion, bitid)
                alg.addvariable('MaxEta1', 49*_eta_conversion, bitid)
                alg.addvariable('MinEta2', d.minEta2*_eta_conversion, bitid)
                alg.addvariable('MaxEta2', d.maxEta2*_eta_conversion, bitid)
                alg.addvariable('MinDeltaPhi', minDphi*_phi_conversion, bitid)
                alg.addvariable('MaxDeltaPhi', d.maxDphi*_phi_conversion, bitid)
            tm.registerTopoAlgo(alg)

            inputList = [d.otype1 + d.olist1, 'jEMsm25ETA49']
            toponames=[]
            for minDphi in d.minDphiList:
                toponames.append ("%iINVM-%02dDPHI%i-%s%s%s%s-jEM%ssm%s%iETA%i"  % (d.minInvm, minDphi, d.maxDphi,
                                                                                     d.otype1, str(d.ocut1) , d.olist1, str(d.nleading1) if d.olist1=="s" else "",
                                                                                     str(d.ocut2) , str(d.nleading2) , d.minEta2, d.maxEta2))
            alg = AlgConf.InvariantMassDeltaPhiInclusive2( name = 'ZAFB_DPHIM', inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth1', d.inputwidth1)
            alg.addgeneric('InputWidth2', d.inputwidth2)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits',  len(toponames))
            alg.addgeneric('ApplyEtaCut', 1)
            for bitid,minDphi in enumerate(d.minDphiList):
                alg.addvariable('MinET1',  get_threshold_cut(d.otype1, d.ocut1)*_et_conversion, bitid)
                alg.addvariable('MinET2',  get_threshold_cut('jEM', d.ocut2)*_et_conversion, bitid)
                alg.addvariable('MinMSqr', d.minInvm*d.minInvm*_et_conversion*_et_conversion, bitid)
                alg.addvariable('MaxMSqr', _no_m_upper_threshold, bitid)
                alg.addvariable('MinEta1',  0*_eta_conversion, bitid)
                alg.addvariable('MaxEta1', 49*_eta_conversion, bitid)
                alg.addvariable('MinEta2', d.minEta2*_eta_conversion, bitid)
                alg.addvariable('MaxEta2', d.maxEta2*_eta_conversion, bitid)
                alg.addvariable('MinDeltaPhi', minDphi*_phi_conversion, bitid)
                alg.addvariable('MaxDeltaPhi', d.maxDphi*_phi_conversion, bitid)
            tm.registerTopoAlgo(alg)

        # ATR-19302, ATR-21637
        listofalgos=[
            {"minInvm": 0,"maxInvm": 70,"minDphi": 27,"maxDphi": 32,"otype":"eEM","olist":"s","ocut1":9,"nleading1":1,"ocut2":9,"nleading2":6,}, #0INVM70-27DPHI32-eEM9s1-eEM9s6
            {"minInvm": 0,"maxInvm": 70,"minDphi": 27,"maxDphi": 32,"otype":"eEM","olist":"sl","ocut1":9,"nleading1":1,"ocut2":9,"nleading2":6,}, #0INVM70-27DPHI32-eEM9sl1-eEM9sl6
        ]
        for x in listofalgos:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'maxInvm', 'minDphi', 'maxDphi',
                             'otype', 'olist', 'ocut1', 'nleading1',
                             'ocut2', 'nleading2')
            for k in x:
                setattr (d, k, x[k])
            toponame = "%iINVM%i-%iDPHI%i-%s%s%s%s-%s%s%s%s"  % (d.minInvm, d.maxInvm, d.minDphi, d.maxDphi, d.otype, str(d.ocut1), d.olist, str(d.nleading1), d.otype, str(d.ocut2), d.olist,str(d.nleading2))
            log.debug("Define %s", toponame)
            inputList = [d.otype + d.olist, d.otype + d.olist]
            alg = AlgConf.InvariantMassDeltaPhiInclusive2( name = toponame, inputs = inputList, outputs = toponame )  
            alg.addgeneric('InputWidth1', HW.eEmOutputWidthSort)
            alg.addgeneric('InputWidth2', HW.eEmOutputWidthSort)
            alg.addgeneric('MaxTob1', d.nleading1)
            alg.addgeneric('MaxTob2', d.nleading2)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',  get_threshold_cut(d.otype, d.ocut1)*_et_conversion)
            alg.addvariable('MinET2',  get_threshold_cut(d.otype, d.ocut2)*_et_conversion)
            alg.addvariable('MinMSqr', d.minInvm*d.minInvm*_et_conversion*_et_conversion)
            alg.addvariable('MaxMSqr', d.maxInvm*d.maxInvm*_et_conversion*_et_conversion)
            alg.addgeneric('ApplyEtaCut',  1)
            alg.addvariable('MinEta1', 0*_eta_conversion)
            alg.addvariable('MaxEta1',49*_eta_conversion)
            alg.addvariable('MinEta2', 0*_eta_conversion)
            alg.addvariable('MaxEta2',49*_eta_conversion)
            alg.addvariable('MinDeltaPhi', d.minDphi*_phi_conversion)
            alg.addvariable('MaxDeltaPhi', d.maxDphi*_phi_conversion)
            tm.registerTopoAlgo(alg)


        # DR Map for INVM_DR_2MU3 Trigger. Output lines:
        #'7INVM22-0DR20-2MU3Vab'
        #'7INVM22-0DR12-2MU3Vab'
        INVM_DR_2MU3Vab_Map = [
        {
            "algoname": "7INVM22_DR_2MU3Vab",
            "minInvm" : 7,
            "maxInvm" : 22,
            "minDR"   : [0,0],
            "maxDR"   : [12,20],
            "otype1"  : "MU3Vab",
            "mult1"   : 2
        }
        ]
        for x in INVM_DR_2MU3Vab_Map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDR', 'maxDR',
                             'otype1', 'mult1')
            for k in x:
                setattr(d,k,x[k])
            inputList = d.otype1 
            toponames = []
            for bitId in range(len(d.minDR)):
                toponames.append("%iINVM%i-%iDR%i-%i%s" % ( d.minInvm, d.maxInvm,
                                                                d.minDR[bitId] , d.maxDR[bitId], 
                                                                d.mult1, d.otype1
                                                                )
                )
            alg = AlgConf.InvariantMassInclusiveDeltaRSqrIncl1( name = d.algoname, inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1',    0*_et_conversion, bitId)
                alg.addvariable('MinET2',    0*_et_conversion, bitId)
                alg.addvariable('MinMSqr',   d.minInvm*d.minInvm*_et_conversion*_et_conversion, bitId)
                alg.addvariable('MaxMSqr',   d.maxInvm*d.maxInvm*_et_conversion*_et_conversion, bitId)
                alg.addvariable('DeltaRMin', d.minDR[bitId]*d.minDR[bitId]*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax', d.maxDR[bitId]*d.maxDR[bitId]*_dr_conversion*_dr_conversion, bitId)
            tm.registerTopoAlgo(alg)


        # INVM_DR_2MU3VFab 
        # Output lines: '2INVM9-0DR15-2MU3VFab' 
                    #   '7INVM11-25DR99-2MU3VFab', 
        

        INVM_DR_2MU3VFab_Map = [
        {
            "algoname": "INVM_DR_2MU3VFab",
            "minInvm" : [2,7],
            "maxInvm" : [9,11,22],
            "minDR"   : [0,25],
            "maxDR"   : [15,99],
            "otype1"  : "MU3VFab",
            "mult1"   : 2
        }
        ]
        for x in INVM_DR_2MU3VFab_Map:
            class d:  # type: ignore
                __slots__ = ('algoname', 'minInvm', 'maxInvm', 'minDR', 'maxDR',
                             'otype1', 'mult1')
            for k in x:
                setattr(d,k,x[k])
            inputList = d.otype1 
            toponames = []
            for bitId in range(len(d.minDR)):
                toponames.append("%iINVM%i-%iDR%i-%i%s" % ( d.minInvm[bitId], d.maxInvm[bitId],
                                                                d.minDR[bitId] , d.maxDR[bitId], 
                                                                d.mult1, d.otype1
                                                                )
                )
            alg = AlgConf.InvariantMassInclusiveDeltaRSqrIncl1( name = d.algoname, inputs = inputList, outputs = toponames)
            alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
            alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', len(toponames))
            for bitId in range(len(toponames)):
                alg.addvariable('MinET1',    0*_et_conversion, bitId)
                alg.addvariable('MinET2',    0*_et_conversion, bitId)
                alg.addvariable('MinMSqr',   d.minInvm[bitId]*d.minInvm[bitId]*_et_conversion*_et_conversion, bitId)
                alg.addvariable('MaxMSqr',   d.maxInvm[bitId]*d.maxInvm[bitId]*_et_conversion*_et_conversion, bitId)
                alg.addvariable('DeltaRMin', d.minDR[bitId]*d.minDR[bitId]*_dr_conversion*_dr_conversion, bitId)
                alg.addvariable('DeltaRMax', d.maxDR[bitId]*d.maxDR[bitId]*_dr_conversion*_dr_conversion, bitId)
            tm.registerTopoAlgo(alg)
    


        #ATR-19720 and ATR-22782, BPH DR+M dimuon
        listofalgos=[
            {"minInvm": 2, "maxInvm": 9,  "minDr": 0,  "maxDr": 15, "mult": 1, "otype1" : "MU5VFab", "otype2": "MU3Vab", }, #2INVM9-0DR15-MU5VFab-MU3Vab 
            {"minInvm": 8, "maxInvm": 15, "minDr": 0,  "maxDr": 22, "mult": 1, "otype1" : "MU5VFab", "otype2": "MU3Vab", }, #8INVM15-0DR22-MU5VFab-MU3Vab
            {"minInvm": 2, "maxInvm": 9,  "minDr": 0,  "maxDr": 15, "mult": 2, "otype1" : "MU3Vab",  "otype2": "",       }, #2INVM9-0DR15-2MU3Vab
           

            {"minInvm": 0, "maxInvm": 16, "minDr": 20, "maxDr": 99, "mult": 2, "otype1" : "MU3Vab",  "otype2": "",},        #0INVM16-20DR99-2MU3Vab
            {"minInvm": 0, "maxInvm": 16, "minDr": 15, "maxDr": 99, "mult": 2, "otype1" : "MU3Vab",  "otype2": "",},        #0INVM16-15DR99-2MU3Vab
            {"minInvm": 8, "maxInvm": 15, "minDr": 20, "maxDr": 99, "mult": 2, "otype1" : "MU3Vab",  "otype2": "",},        #8INVM15-20DR99-2MU3Vab
            {"minInvm": 8, "maxInvm": 15, "minDr": 15, "maxDr": 99, "mult": 2, "otype1" : "MU3Vab",  "otype2": "",},        #8INVM15-15DR99-2MU3Vab

            {"minInvm": 7, "maxInvm": 22, "minDr": 0, "maxDr": 20, "mult": 2, "otype1" : "MU3VFab",  "otype2": "",},        #7INVM22-0DR20-2MU3VFab, ATR-21566

            {"minInvm": 8, "maxInvm": 15, "minDr": 0,  "maxDr": 22, "mult": 1, "otype1" : "CMU5VFab","otype2": "CMU3Vab",}, #8INVM15-0DR22-CMU5VFab-CMU3Vab
 
            {"minInvm": 7, "maxInvm": 14, "minDr": 0,  "maxDr": 25, "mult": 1, "otype1" : "MU5VFab", "otype2": "MU3Vab", }, #7INVM14-0DR25-MU5VFab-MU3Vab
            {"minInvm": 7, "maxInvm": 11, "minDr": 25, "maxDr": 99, "mult": 2, "otype1" : "MU3Vab",  "otype2": "",},        #7INVM11-25DR99-2MU3Vab
            {"minInvm": 7, "maxInvm": 14, "minDr": 0,  "maxDr": 25, "mult": 1, "otype1" : "MU5VFab", "otype2": "MU3VFab", }, #7INVM14-0DR25-MU5VFab-MU3VFab


        ]
        for x in listofalgos:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'maxInvm', 'minDr', 'maxDr', 'mult', 'otype1', 'otype2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iINVM%i-%iDR%i-%s%s"  % (d.minInvm, d.maxInvm, d.minDr, d.maxDr, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.InvariantMassInclusiveDeltaRSqrIncl1 if (d.mult>1) else AlgConf.InvariantMassInclusiveDeltaRSqrIncl2
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',    0*_et_conversion)
            alg.addvariable('MinET2',    0*_et_conversion)
            alg.addvariable('MinMSqr',   d.minInvm*d.minInvm*_et_conversion*_et_conversion)
            alg.addvariable('MaxMSqr',   d.maxInvm*d.maxInvm*_et_conversion*_et_conversion)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion)
            tm.registerTopoAlgo(alg)

        #ATR-19639, BPH DR+M+OS dimuon
        listofalgos=[
            {"minInvm": 2, "maxInvm": 9,  "minDr": 0,  "maxDr": 15, "mult": 1, "otype1" : "MU5VFab", "otype2": "MU3Vab",}, #2INVM9-0DR15-C-MU5VFab-MU3Vab
            {"minInvm": 8, "maxInvm": 15, "minDr": 20, "maxDr": 99, "mult": 2, "otype1" : "MU3Vab",  "otype2": "",},       #8INVM15-20DR99-C-2MU3Vab
        ]
        for x in listofalgos:
            class d:  # type: ignore
                __slots__ = ('minInvm', 'maxInvm', 'minDr', 'maxDr', 'mult', 'otype1', 'otype2')
            for k in x:
                setattr (d, k, x[k])
            obj1 = "%s%s" % ((str(d.mult) if d.mult>1 else ""), d.otype1)
            obj2 = "-%s" % (d.otype2)
            toponame = "%iINVM%i-%iDR%i-C-%s%s"  % (d.minInvm, d.maxInvm, d.minDr, d.maxDr, obj1, "" if d.mult>1 else obj2)
            log.debug("Define %s", toponame)
            inputList = [d.otype1] if (d.mult>1) else [d.otype1, d.otype2]
            algoname = AlgConf.InvariantMassInclusiveDeltaRSqrIncl1Charge if (d.mult>1) else AlgConf.InvariantMassInclusiveDeltaRSqrIncl2Charge
            alg = algoname( name = toponame,  inputs = inputList, outputs = [ toponame ])
            if (d.mult>1):
                alg.addgeneric('InputWidth', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob', HW.muonOutputWidthSelect)
            else:
                alg.addgeneric('InputWidth1', HW.muonOutputWidthSelect)
                alg.addgeneric('InputWidth2', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob1', HW.muonOutputWidthSelect)
                alg.addgeneric('MaxTob2', HW.muonOutputWidthSelect)
            alg.addgeneric('NumResultBits', 1)
            alg.addvariable('MinET1',    0*_et_conversion)
            alg.addvariable('MinET2',    0*_et_conversion)
            alg.addvariable('MinMSqr',   d.minInvm*d.minInvm*_et_conversion*_et_conversion)
            alg.addvariable('MaxMSqr',   d.maxInvm*d.maxInvm*_et_conversion*_et_conversion)
            alg.addvariable('DeltaRMin', d.minDr*d.minDr*_dr_conversion*_dr_conversion)
            alg.addvariable('DeltaRMax', d.maxDr*d.maxDr*_dr_conversion*_dr_conversion)
            tm.registerTopoAlgo(alg)


        # dphi with s+s #ATR-29784
        # Parameter ordering:
        # 1. MinEt1
        # 2. MinEt2
        # 3. DeltaPhiMin
        # 4. DeltaPhiMax
        algolist=[
            { "minDphi": 23, "maxDphi": 32, "mult": 2, "otype1" : "eEM",
                "ocut1": 1, "olist1": "s", "nleading1": HW.eEmOutputWidthSort, "minET1":0.9, "otype2" : "", "minET2":0.8},#23DPHI32_2eEM1s
            { "minDphi": 26, "maxDphi": 32, "mult": 2, "otype1" : "eTAU",
                "ocut1": 1, "olist1": "s", "nleading1": HW.eTauOutputWidthSort,"minET1":0.8,"otype2" : "","minET2":0.8},#26DPHI32_2eTAU1s
            { "minDphi": 23, "maxDphi": 32, "mult": 2, "otype1" : "jTAU",
                "ocut1": 1, "olist1": "s", "nleading1": HW.jTauOutputWidthSort,"minET1":1.4,"otype2" : "","minET2":1.4},#23DPHI32_2jTAU1s
        ]
        for x in algolist:
            class d:  # type: ignore
                __slots__ = ('minDphi', 'maxDphi', 'mult', 'otype1', 'ocut1', 'olist1', 'nleading1', 'minET1', 'otype2', 'minET2')  
            for k in x:
                setattr (d, k, x[k])
            obj = "%s" % (str(d.mult)+d.otype1+str(d.ocut1)+str(d.olist1))
            toponame = "%sDPHI%s-%s"  % (d.minDphi, d.maxDphi, obj)
            log.debug("Define %s", toponame)
            inputList = [d.otype1+d.olist1]
            algoname = AlgConf.DeltaPhiIncl1
            alg = algoname( name = toponame, inputs = inputList, outputs = [ toponame ])
            alg.addgeneric('NumResultBits', 1)
            alg.addgeneric('InputWidth', d.nleading1)
            alg.addgeneric('MaxTob', d.nleading1)
            alg.addvariable('MinET1',      d.minET1*_et_conversion)
            alg.addvariable('MinET2',      d.minET2*_et_conversion)
            alg.addvariable('MinDeltaPhi', d.minDphi*_phi_conversion)
            alg.addvariable('MaxDeltaPhi', d.maxDphi*_phi_conversion)
            tm.registerTopoAlgo(alg)

        # InvMass + DPHI (+ SumET) for ATR-30727 and ATR-30728
        from dataclasses import dataclass

        @dataclass
        class InvMassDPhiSumETAlgoParams:
            """ Parameters for InvariantMassDeltaPhiInclusive1 and InvariantMassDeltaPhiSumEtInclusive1 Topo algorithms """
            MinEt1: float
            MinEt2: float
            DeltaPhiMin: int = 23
            DeltaPhiMax: int = 32
            MinInvMassSq: float = 0  # Algorithm uses MinMSqr
            MaxInvMassSq: float = 1000 ** 2  # Algorithm uses MaxMSqr
            MinSumEt: float = 0
            MaxSumEt: float = 1000

        def AddInvMassDPhiSumETAlgos(otype: str, nleading, list_of_algos: list[InvMassDPhiSumETAlgoParams]):
            for x in list_of_algos:
                # Check which parameters are set and choose correct algorithm
                is_InvMass_set = x.MinInvMassSq != 0 or x.MaxInvMassSq != 1000 ** 2
                is_SumEt_set = x.MinSumEt != 0 or x.MaxSumEt != 1000
                algoname = AlgConf.InvariantMassDeltaPhiSumEtInclusive1 if is_SumEt_set else AlgConf.InvariantMassDeltaPhiInclusive1

                if float(x.MinEt1) != int(x.MinEt1):
                    obj = f'2{otype}0{round(x.MinEt1 * 10)}s'  # e.g. MinEt1 = 0.7 → eTAU07
                else:
                    obj = f'2{otype}{round(x.MinEt1)}s'  # e.g. MinEt1 = 1 → eTAU1
                toponame = f'{x.DeltaPhiMin}DPHI{x.DeltaPhiMax}-{obj}' # Always present
                # Check Min/MaxSumEt only for algorithm which supports it
                if is_SumEt_set:
                    toponame = f'{x.MinSumEt}SUM{x.MaxSumEt}-{toponame}'
                if is_InvMass_set:
                    if float(x.MinInvMassSq) != int(x.MinInvMassSq) or float(x.MaxInvMassSq) != int(x.MaxInvMassSq):
                        toponame = f'{round(x.MinInvMassSq * 10)}INVM{round(x.MaxInvMassSq * 10)}-{toponame}'
                    else:
                        toponame = f'{x.MinInvMassSq}INVM{x.MaxInvMassSq}-{toponame}'
                log.debug('Define %s', toponame)

                alg = algoname(name=toponame, inputs=[f'{otype}s'], outputs=[toponame])
                alg.addgeneric('NumResultBits', 1)
                alg.addgeneric('InputWidth', nleading)
                alg.addgeneric('MaxTob', nleading)
                alg.addvariable('MinET1',       x.MinEt1 * _et_conversion)
                alg.addvariable('MinET2',       x.MinEt2 * _et_conversion)
                alg.addvariable('MinMSqr',      x.MinInvMassSq * (_et_conversion ** 2))  # Algoritm uses InvMass squared
                alg.addvariable('MaxMSqr',      x.MaxInvMassSq * (_et_conversion ** 2))
                alg.addvariable('MinDeltaPhi',  x.DeltaPhiMin * _phi_conversion)
                alg.addvariable('MaxDeltaPhi',  x.DeltaPhiMax * _phi_conversion)
                # Set Min/MaxSumEt only for algorithm which supports it
                if is_SumEt_set:
                    alg.addvariable('MinSumEt', x.MinSumEt * _et_conversion)
                    alg.addvariable('MaxSumEt', x.MaxSumEt * _et_conversion)
                tm.registerTopoAlgo(alg)

        # From ATR-30728
        algo_list = [
            InvMassDPhiSumETAlgoParams(MinEt1=0.7, MinEt2=0.7, DeltaPhiMin=24, MinInvMassSq=2.3, MaxInvMassSq=200 ** 2), # 23INVM400000-24DPHI32-2eTAU07
            InvMassDPhiSumETAlgoParams(MinEt1=0.7, MinEt2=0.7, DeltaPhiMin=24, MinInvMassSq=2.8, MaxInvMassSq=200 ** 2), # 28INVM400000-24DPHI32-2eTAU07
            InvMassDPhiSumETAlgoParams(MinEt1=0.8, MinEt2=0.8, DeltaPhiMin=25, MinInvMassSq=2.3, MaxInvMassSq=200 ** 2), # 23INVM400000-25DPHI32-2eTAU08
            InvMassDPhiSumETAlgoParams(MinEt1=0.7, MinEt2=0.7, DeltaPhiMin=25, MinInvMassSq=3.3, MaxInvMassSq=200 ** 2), # 33INVM400000-25DPHI32-2eTAU07
            InvMassDPhiSumETAlgoParams(MinEt1=0.7, MinEt2=0.7, DeltaPhiMin=27, MinInvMassSq=2.3, MaxInvMassSq=200 ** 2), # 23INVM400000-27DPHI32-2eTAU07
        ]
        AddInvMassDPhiSumETAlgos('eTAU', HW.eTauOutputWidthSort, algo_list)

        # From ATR-30727
        algo_list = []
        AddInvMassDPhiSumETAlgos('jJ', HW.jJetOutputWidthSort, algo_list)

        # g-2 tau (ATR-30638)
        Algo = namedtuple('Algo', ['dPhiMin', 'dPhiMax', 'otype', 'olist', 'ocut1', 'ocut2', 'nTOB'])
        algolist=[
            Algo(dPhiMin=30, dPhiMax=32, otype='eTAU', olist='s', ocut1=50, ocut2=50, nTOB=HW.eTauOutputWidthSort), #30DPHI32-2eTAU50s
        ]
        for x in algolist:
            name = f'{x.dPhiMin}DPHI{x.dPhiMax}-'
            name += f'2{x.otype}{x.ocut1}{x.olist}' if x.ocut1 == x.ocut2 else f'{x.otype}{x.ocut1}{x.olist}-{x.otype}{x.ocut2}{x.olist}'

            alg = AlgConf.DeltaPhiIncl1(name=name, inputs=[x.otype+x.olist], outputs=[name])
            alg.addgeneric('NumResultBits', 1)
            alg.addgeneric('InputWidth', x.nTOB)
            alg.addgeneric('MaxTob', x.nTOB)
            alg.addvariable('MinET1', get_threshold_cut(x.otype, x.ocut1)*_et_conversion)
            alg.addvariable('MinET2', get_threshold_cut(x.otype, x.ocut2)*_et_conversion)
            alg.addvariable('MinDeltaPhi', x.dPhiMin*_phi_conversion)
            alg.addvariable('MaxDeltaPhi', x.dPhiMax*_phi_conversion)
            tm.registerTopoAlgo(alg)
