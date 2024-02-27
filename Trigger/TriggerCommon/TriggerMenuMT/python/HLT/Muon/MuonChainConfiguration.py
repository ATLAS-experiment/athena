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

from .MuonMenuSequences import muFastSequence, muFastCalibSequence, mul2mtSAOvlpRmSequence, muCombSequence, muCombOvlpRmSequence, mul2mtCBOvlpRmSequence, mul2IOOvlpRmSequence, muCombLRTSequence, muEFSASequence, muEFSAFSSequence, efLateMuSequence, muEFCBSequence, muEFCBIDperfSequence, muEFCBLRTSequence, muEFCBLRTIDperfSequence, muEFCBFSSequence, muEFIDtpSequence, muEFIsoSequence, muEFMSIsoSequence

from .MuonMenuSequences import efLateMuRoISequence, muRoiClusterSequence
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
           return self.getStep(flags,1,"mufastcalib", [muFastCalibSequence], is_probe_leg=is_probe_leg )
        elif 'l2mt' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags,1,"mufastl2mt", [mul2mtSAOvlpRmSequence], is_probe_leg=is_probe_leg )
        else:
           return self.getStep(flags,1,"mufast", [muFastSequence], is_probe_leg=is_probe_leg )


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
            return self.getStep(flags,2,"muCombl2mt", [mul2mtCBOvlpRmSequence], is_probe_leg=is_probe_leg )
        elif 'l2io' in self.chainPart['l2AlgInfo']:
            return self.getStep(flags,2, 'muCombIO', [mul2IOOvlpRmSequence], is_probe_leg=is_probe_leg )
        elif doOvlpRm:
           return self.getStep(flags,2, 'muComb', [muCombOvlpRmSequence], is_probe_leg=is_probe_leg )
        elif "LRT" in self.chainPart['addInfo']:
           return self.getStep(flags,2, 'muCombLRT', [muCombLRTSequence], is_probe_leg=is_probe_leg )
        else:
           return self.getStep(flags,2, 'muComb', [muCombSequence], is_probe_leg=is_probe_leg )

    # --------------------
    def getmuCombIO(self, flags, is_probe_leg=False):
        return self.getStep(flags,2, 'muCombIO', [mul2IOOvlpRmSequence], is_probe_leg=is_probe_leg )

    # --------------------
    def getmuEFSA(self, flags, is_probe_leg=False):
        return self.getStep(flags,3,'muEFSA',[ muEFSASequence], is_probe_leg=is_probe_leg)

    # --------------------
    def getmuEFCB(self, flags, is_probe_leg=False):

        if 'invm' in self.chainPart['invMassInfo']: # No T&P support, add if needed
            return self.getStep(flags,4,'EFCB', [muEFCBSequence], comboTools=[TrigMuonEFInvMassHypoToolFromDict], is_probe_leg=is_probe_leg)
        elif "LRT" in self.chainPart['addInfo']:
            if "idperf" in self.chainPart['addInfo']:
                return self.getStep(flags,4,'EFCBLRTIDPERF', [muEFCBLRTIDperfSequence], is_probe_leg=is_probe_leg)
            else:
                return self.getStep(flags,4,'EFCBLRT', [muEFCBLRTSequence], is_probe_leg=is_probe_leg)
        elif "idperf" in self.chainPart['addInfo']:
            return self.getStep(flags,4,'EFCBIDPERF', [muEFCBIDperfSequence], is_probe_leg=is_probe_leg)
        elif "idtp" in self.chainPart['addInfo']:
            return self.getStep(flags,4,'EFIDTP', [muEFIDtpSequence], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags,4,'EFCB', [muEFCBSequence], is_probe_leg=is_probe_leg)

    # --------------------
    def getFSmuEFSA(self, flags, is_probe_leg=False):
        return self.getStep(flags,5,'FSmuEFSA', [muEFSAFSSequence], is_probe_leg=is_probe_leg)

    # --------------------
    def getFSmuEFCB(self, flags, is_probe_leg=False):
        if 'invm' in self.chainPart['invMassInfo']:
            return self.getStep(flags,6,'FSmuEFCB', [muEFCBFSSequence],comboTools=[TrigMuonEFInvMassHypoToolFromDict], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags,6,'FSmuEFCB', [muEFCBFSSequence], is_probe_leg=is_probe_leg)

    #---------------------
    def getmuEFIso(self, flags, is_probe_leg=False):
        if any(x in self.dict['topo'] for x in ['b7invmAB9vtx20', 'b11invmAB60vtx20', 'b11invmAB24vtx20', 'b24invmAB60vtx20']):
            from TrigBphysHypo.TrigMultiTrkComboHypoConfig import DrellYanComboHypoCfg, TrigMultiTrkComboHypoToolFromDict
            return self.getStep(flags,5,'muEFIsoDY', [muEFIsoSequence], comboHypoCfg=DrellYanComboHypoCfg, comboTools=[TrigMultiTrkComboHypoToolFromDict], is_probe_leg=is_probe_leg)
        else:
            return self.getStep(flags,5,'muEFIso', [muEFIsoSequence], is_probe_leg=is_probe_leg)

    #---------------------
    def getmuEFMSIso(self, flags, is_probe_leg=False):
        return self.getStep(flags,5,'muEFMSIso',[ muEFMSIsoSequence], is_probe_leg=is_probe_leg)

    #--------------------
    def getmuMSEmptyAll(self, flags, stepID): # No T&P info needed for empty step?
        return self.getEmptyStep(stepID,'muMS_empty')

    #--------------------
    def getmuMSEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getmuMSEmptyAll(flags, 2)

    #--------------------
    def getmuFastEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getEmptyStep(1,'muFast_empty')

    #--------------------
    def getEFCBEmpty(self, flags, is_probe_leg=False): # No T&P info needed for empty step?
        return self.getEmptyStep(4,'muefCB_Empty')

    #--------------------
    def getLateMuRoI(self, flags, is_probe_leg=False): # No T&P support, add if needed
        return self.getStep(flags,1,'muEFLateRoI',[efLateMuRoISequence], is_probe_leg=is_probe_leg)

    #--------------------
    def getLateMu(self, flags, is_probe_leg=False): # No T&P support, add if needed
        return self.getStep(flags,2,'muEFLate',[efLateMuSequence], is_probe_leg=is_probe_leg)

    #--------------------
    def getmuRoiClu(self, flags, is_probe_leg=False):
        return self.getStep(flags,1,'muRoiClu',[muRoiClusterSequence])


def TrigMuonEFIdtpInvMassHypoToolCfg(flags, chainDict):
    tool = TrigMuonEFIdtpInvMassHypoToolFromDict(flags, chainDict)
    return tool


