# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

########################################################################
#
# SliceDef file for muon chains/signatures
#
#########################################################################
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from ..Config.ChainConfigurationBase import ChainConfigurationBase

from .MuonMenuSequences import (
    muFastSequenceGenCfg, muFastCalibSequenceGenCfg, mul2mtSAOvlpRmSequenceGenCfg, 
    muCombSequenceGenCfg, muCombOvlpRmSequenceGenCfg, mul2mtCBOvlpRmSequenceGenCfg, 
    mul2IOOvlpRmSequenceGenCfg, muCombLRTSequenceGenCfg, muEFSASequenceGenCfg, 
    muEFSAFSSequenceGenCfg, efLateMuSequenceGenCfg, muEFCBSequenceGenCfg, 
    muEFCBl2ioSequenceGenCfg, muEFCBl2mtSequenceGenCfg,
    muEFCBIDperfSequenceGenCfg, muEFCBLRTSequenceGenCfg, muEFCBLRTIDperfSequenceGenCfg, 
    muEFCBFSSequenceGenCfg, muEFIDtpSequenceGenCfg, muEFIsoSequenceGenCfg, 
    muEFMSIsoSequenceGenCfg, efLateMuRoISequenceGenCfg, muRoiClusterSequenceGenCfg )

from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFInvMassHypoToolFromDict
from TrigMuonHypo.TrigMuonHypoConfig import TrigMuonEFIdtpInvMassHypoToolFromDict


#############################################
###  Class/function to configure muon chains
#############################################

class MuonChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------

    def assembleChainImpl(self, flags):
        chainSteps = []

        stepDictionary = self.getStepDictionary()

        is_probe_leg = self.chainPart['tnpInfo']=="probe"
        key = self.chainPart['extra']

        steps=stepDictionary[key]                

        for step in steps:
            if step:
                chainstep = getattr(self, step)(flags, is_probe_leg=is_probe_leg)
                chainSteps+=[chainstep]

        myChain = self.buildChain(chainSteps)
        return myChain

    def getStepDictionary(self):

        # --------------------
        # define here the names of the steps and obtain the chainStep configuration
        # --------------------
        doMSonly = 'msonly' in self.chainPart['msonlyInfo']
        muCombStep = 'getmuComb'
        efCBStep = 'getmuEFCB'
        if self.chainPart['isoInfo']:
            isoStep = 'getmuEFIso'
            if doMSonly:
                isoStep = 'getmuEFMSIso'
                #need to align SA and isolation steps between
                # ms-only and standard iso chains
                muCombStep = 'getmuMSEmpty'
                efCBStep = 'getEFCBEmpty'
        else:
            isoStep=None
            if doMSonly:
                #need to align final SA step between ms-only
                #and standard chains
                muCombStep = 'getmuMSEmpty'
                efCBStep = None

        stepDictionary = {            
            "":['getmuFast', muCombStep, 'getmuEFSA',efCBStep, isoStep], #RoI-based triggers
            "noL1":['getFSmuEFSA'] if doMSonly else ['getFSmuEFSA', 'getFSmuEFCB'], #full scan triggers
            "lateMu":['getLateMuRoI','getLateMu'], #late muon triggers
            "muoncalib":['getmuFast'], #calibration
            "vtx":['getmuRoiClu'], #LLP Trigger
            "mucombTag":['getmuFast', muCombStep], #Trigger for alignment 
        }

        return stepDictionary


    # --------------------
    def getmuFast(self, flags, is_probe_leg=False):

        if 'muoncalib' in self.chainPart['extra']:
           return self.getStep(flags, "mufastcalib", [muFastCalibSequenceGenCfg], is_probe_leg=is_probe_leg )
        elif 'l2mt' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags, "mufastl2mt", [mul2mtSAOvlpRmSequenceGenCfg], is_probe_leg=is_probe_leg )
        else:
           return self.getStep(flags, "mufast", [muFastSequenceGenCfg], is_probe_leg=is_probe_leg )


    # --------------------
    def getmuComb(self, flags, is_probe_leg=False):

        doOvlpRm = False
        if self.chainPart['signature'] == 'Bphysics':
           doOvlpRm = False
        elif self.mult>1:
           doOvlpRm = True
        elif len( self.dict['signatures'] )>1 and not self.chainPart['extra']:
           doOvlpRm = True
        else:
           doOvlpRm = False

        if 'l2mt' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags, "muCombl2mt", [mul2mtCBOvlpRmSequenceGenCfg], is_probe_leg=is_probe_leg )
        elif 'l2io' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags, 'muCombIO', [mul2IOOvlpRmSequenceGenCfg], is_probe_leg=is_probe_leg )
        elif doOvlpRm:
           return self.getStep(flags, 'muCombOVR', [muCombOvlpRmSequenceGenCfg], is_probe_leg=is_probe_leg )
        elif "LRT" in self.chainPart['addInfo']:
           return self.getStep(flags, 'muCombLRT', [muCombLRTSequenceGenCfg], is_probe_leg=is_probe_leg )
        else:
           return self.getStep(flags, 'muComb', [muCombSequenceGenCfg], is_probe_leg=is_probe_leg )

    # --------------------
    def getmuCombIO(self, flags, is_probe_leg=False):
        return self.getStep(flags, 'muCombIO', [mul2IOOvlpRmSequenceGenCfg], is_probe_leg=is_probe_leg )

    # --------------------
    def getmuEFSA(self, flags, is_probe_leg=False):
        return self.getStep(flags, 'muEFSA',[ muEFSASequenceGenCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getmuEFCB(self, flags, is_probe_leg=False):
        if 'invm' in self.chainPart['invMassInfo']: # No T&P support, add if needed
            return self.getStep(flags, 'EFCB', [muEFCBSequenceGenCfg], comboTools=[TrigMuonEFInvMassHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif "LRT" in self.chainPart['addInfo']:
            if "idperf" in self.chainPart['addInfo']:
                return self.getStep(flags, 'EFCBLRTIDPERF', [muEFCBLRTIDperfSequenceGenCfg], is_probe_leg=is_probe_leg)
            else:
                return self.getStep(flags, 'EFCBLRT', [muEFCBLRTSequenceGenCfg], is_probe_leg=is_probe_leg)
        elif "idperf" in self.chainPart['addInfo']:
            return self.getStep(flags, 'EFCBIDPERF', [muEFCBIDperfSequenceGenCfg], is_probe_leg=is_probe_leg)
        elif "idtp" in self.chainPart['addInfo']:
            return self.getStep(flags, 'EFIDTP', [muEFIDtpSequenceGenCfg], is_probe_leg=is_probe_leg)
        else:
            if flags.Muon.enableTrigIDtrackReuse:
                if "l2io" in self.chainPart['l2AlgInfo']:
                    return self.getStep(flags, 'EFCBl2io', [muEFCBl2ioSequenceGenCfg], is_probe_leg=is_probe_leg)
                elif "l2mt" in self.chainPart['l2AlgInfo']:
                    return self.getStep(flags, 'EFCBl2mt', [muEFCBl2mtSequenceGenCfg], is_probe_leg=is_probe_leg)
                else:
                    return self.getStep(flags, 'EFCB', [muEFCBSequenceGenCfg], is_probe_leg=is_probe_leg)
            else:
                return self.getStep(flags, 'EFCB', [muEFCBSequenceGenCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getFSmuEFSA(self, flags, is_probe_leg=False):
        return self.getStep(flags, 'FSmuEFSA', [muEFSAFSSequenceGenCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getFSmuEFCB(self, flags, is_probe_leg=False):
        if 'invm' in self.chainPart['invMassInfo']:
            return self.getStep(flags, 'FSmuEFCB', [muEFCBFSSequenceGenCfg],comboTools=[TrigMuonEFInvMassHypoToolFromDict], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags, 'FSmuEFCB', [muEFCBFSSequenceGenCfg], is_probe_leg=is_probe_leg)

    #---------------------
    def getmuEFIso(self, flags, is_probe_leg=False):
        if any(x in self.dict['topo'] for x in ['b7invmAB9vtx20', 'b11invmAB60vtx20', 'b11invmAB24vtx20', 'b24invmAB60vtx20']):
            from TrigBphysHypo.TrigMultiTrkComboHypoConfig import DrellYanComboHypoCfg, TrigMultiTrkComboHypoToolFromDict
            return self.getStep(flags, 'muEFIsoDY', [muEFIsoSequenceGenCfg], comboHypoCfg=DrellYanComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags, 'muEFIso', [muEFIsoSequenceGenCfg], is_probe_leg=is_probe_leg)

    #---------------------
    def getmuEFMSIso(self, flags, is_probe_leg=False):
        return self.getStep(flags, 'muEFMSIso',[ muEFMSIsoSequenceGenCfg], is_probe_leg=is_probe_leg)

    #--------------------
    def getmuMSEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getEmptyStep('muMS_empty')

    #--------------------
    def getmuFastEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getEmptyStep('muFast_empty')

    #--------------------
    def getEFCBEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getEmptyStep('muefCB_Empty')

    #--------------------
    def getLateMuRoI(self, flags, is_probe_leg=False): # No T&P support, add if needed
        return self.getStep(flags, 'muEFLateRoI',[efLateMuRoISequenceGenCfg], is_probe_leg=is_probe_leg)

    #--------------------
    def getLateMu(self, flags, is_probe_leg=False): # No T&P support, add if needed
        return self.getStep(flags, 'muEFLate',[efLateMuSequenceGenCfg], is_probe_leg=is_probe_leg)

    #--------------------
    def getmuRoiClu(self, flags, is_probe_leg=False):
        return self.getStep(flags, 'muRoiClu',[muRoiClusterSequenceGenCfg])


def TrigMuonEFIdtpInvMassHypoToolCfg(flags, chainDict):
    tool = TrigMuonEFIdtpInvMassHypoToolFromDict(flags, chainDict)
    return tool


