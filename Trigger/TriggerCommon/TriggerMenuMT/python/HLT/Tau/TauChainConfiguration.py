# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

########################################################################
#
# SliceDef file for muon chains/signatures
#
#########################################################################
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase

from .TauMenuSequences import (
    tauCaloMVAMenuSequenceGenCfg, tauFTFTauCoreSequenceGenCfg, 
    tauFTFTauIsoSequenceGenCfg, tauFTFTauLRTSequenceGenCfg, 
    tauPrecTrackIsoSequenceGenCfg, tauPrecTrackLRTSequenceGenCfg, 
    tauTrackTwoMVASequenceGenCfg, tauTrackTwoLLPSequenceGenCfg, 
    tauTrackLRTSequenceGenCfg )


############################################# 
###  Class/function to configure muon chains 
#############################################

class TauChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):                            
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        stepDictionary = {
            "ptonly"        :['getCaloMVASeq', 'getFTFEmpty', 'getTrkEmpty' , 'getPTEmpty'      , 'getIDEmpty'     ],
            "tracktwoMVA"   :['getCaloMVASeq', 'getFTFCore' , 'getFTFIso'   , 'getPrecTrackIso' , 'getTrackTwoMVA' ],
            "tracktwoLLP"   :['getCaloMVASeq', 'getFTFCore' , 'getFTFIso'   , 'getPrecTrackIso' , 'getTrackTwoLLP' ],
            "trackLRT"      :['getCaloMVASeq', 'getFTFLRT'  , 'getTrkEmpty' , 'getPrecTrackLRT' , 'getTrackLRT'    ],
        }

        # this should be extended by the signature expert to make full use of the dictionary!
        key = self.chainPart['preselection']
        steps=stepDictionary[key]
        for step in steps:
            is_probe_leg = self.chainPart['tnpInfo']=='probe'
            if 'Empty' in step:
                chainstep = getattr(self, step)(flags)
            else:
                chainstep = getattr(self, step)(flags, is_probe_leg=is_probe_leg)
            chainSteps+=[chainstep]
    
        myChain = self.buildChain(chainSteps)
        return myChain


    # --------------------
    def getCaloMVASeq(self, flags, is_probe_leg=False):
        stepName = 'MVA_tau'
        return self.getStep(flags,1,stepName, [tauCaloMVAMenuSequenceGenCfg], is_probe_leg=is_probe_leg)
        
    # --------------------
    def getFTFCore(self, flags, is_probe_leg=False):
        stepName = 'FTFCore_tau'
        return self.getStep(flags,2,stepName, [tauFTFTauCoreSequenceGenCfg], is_probe_leg=is_probe_leg)

    # --------------------
    def getFTFLRT(self, flags, is_probe_leg=False):
        stepName = 'FTFLRT_tau'
        return self.getStep(flags,2,stepName, [tauFTFTauLRTSequenceGenCfg], is_probe_leg=is_probe_leg)

    # --------------------

    def getFTFEmpty(self, flags):
        stepName = 'FTFEmpty_tau'
        return self.getEmptyStep(2,stepName)

    # --------------------

    def getFTFIso(self, flags, is_probe_leg=False):
        stepName = 'FTFIso_tau'
        return self.getStep(flags,3,stepName, [tauFTFTauIsoSequenceGenCfg], is_probe_leg=is_probe_leg)

    # --------------------

    def getTrkEmpty(self, flags):
        stepName = 'TrkEmpty_tau'
        return self.getEmptyStep(3,stepName)

    # --------------------

    def getPrecTrackIso(self, flags, is_probe_leg=False):
        stepName = 'PrecTrkIso_tau'
        return self.getStep(flags,4,stepName,[tauPrecTrackIsoSequenceGenCfg],is_probe_leg=is_probe_leg)

    # --------------------
    def getPrecTrackLRT(self, flags, is_probe_leg=False):
        stepName = 'PrecTrkLRT_tau'
        return self.getStep(flags,4,stepName,[tauPrecTrackLRTSequenceGenCfg],is_probe_leg=is_probe_leg)

    # --------------------

    def getPTEmpty(self, flags):
        stepName = 'PTEmpty_tau'
        return self.getEmptyStep(4,stepName)

    # --------------------

    def getTrackTwoMVA(self, flags, is_probe_leg=False):
        stepName = "TrkTwoMVA_tau"
        return self.getStep(flags,5,stepName,[tauTrackTwoMVASequenceGenCfg],is_probe_leg=is_probe_leg)

    # --------------------

    def getTrackTwoLLP(self, flags, is_probe_leg=False):
        stepName = "TrkTwoLLP_tau"
        return self.getStep(flags,5,stepName,[tauTrackTwoLLPSequenceGenCfg],is_probe_leg=is_probe_leg)

    # --------------------
    def getTrackLRT(self, flags, is_probe_leg=False):
        stepName = "TrkLRT_tau"
        return self.getStep(flags,5,stepName,[tauTrackLRTSequenceGenCfg],is_probe_leg=is_probe_leg)

    # --------------------

    def getIDEmpty(self, flags):
        stepName = 'IDEmpty_tau'
        return self.getEmptyStep(5,stepName)
