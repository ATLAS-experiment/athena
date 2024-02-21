# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJComboHypoToolFromDict

from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getFullScanRecoOnlySequence
from TriggerMenuMT.HLT.UnconventionalTracking.DJTriggerConfiguration import DJPromptStep, DJDispStep
from TriggerMenuMT.HLT.UnconventionalTracking.DVTriggerConfiguration import DVRecoSequence, DVTriggerEDSequence
from TriggerMenuMT.HLT.UnconventionalTracking.DisTrkTriggerConfiguration import DisTrkTriggerHypoSequence
from TriggerMenuMT.HLT.UnconventionalTracking.FullScanLRTTrackingConfiguration import FullScanLRTMenuSequence
from TriggerMenuMT.HLT.UnconventionalTracking.HitDVConfiguration import HitDVHypoSequence, UTTJetRecoSequence
from TriggerMenuMT.HLT.UnconventionalTracking.IsoHighPtTrackTriggerConfiguration import IsoHPtTrackTriggerHypoSequence
from TriggerMenuMT.HLT.UnconventionalTracking.VrtSecInclusiveConfiguration import VrtSecInclusiveMenuSequence
from TriggerMenuMT.HLT.UnconventionalTracking.dEdxTriggerConfiguration import dEdxTriggerHypoSequence

#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class UnconventionalTrackingChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, inflags):
        log.debug("Assembling chain %s", self.chainName)
        
        from TrigInDetConfig.utils import getFlagsForActiveConfig
        flags = getFlagsForActiveConfig(inflags, "fullScan", log)
        
        chainSteps = []

        stepDictionary = self.getStepDictionary()

        key = self.chainPart['trigType']
        steps = stepDictionary[key]

        for step in steps:
            chainstep = getattr(self, step)(flags)
            chainSteps += [chainstep]

        myChain = self.buildChain(chainSteps)

        return myChain

    def getStepDictionary(self):

        stepDictionary = {
            "isotrk" : ['getIsoHPtTrackEmpty', 'getRoITrkEmpty', 'getFTFTrackReco', 'getIsoHPtTrackTrigger'],
            "fslrt" : ['getFSLRTEmpty', 'getRoITrkEmpty', 'getFSLRTTrigger'],
            "dedxtrk" : ['getdEdxEmpty', 'getRoITrkEmpty', 'getFTFTrackReco', 'getdEdxTrigger'],
            "hitdvjet" : ['getJetReco', 'getRoITrkEmpty', 'getFTFTrackReco', 'getHitDVTrigger'],
            "fsvsi" : ['getVSIEmpty', 'getRoITrkEmpty', 'getVSITrigger'],
            "distrk" : ['getDisTrkEmpty', 'getRoITrkEmpty', 'getFTFTrackReco', 'getDisTrkTrigger'],
            "dispvtx" : ['getJetReco', 'getRoITrkEmpty', 'getFTFTrackReco', 'getHitDVTrigger', 'getDVRecoStep', 'getDVEDStep'],
            "dispjet" : ['getJetReco', 'getRoITrkEmpty', 'getFTFTrackReco', 'getDJPromptStep', 'getDJDispStep']
        }

        return stepDictionary

    # --------------------
    # Step definitions in alignment order
    # Step 1
    def getJetReco(self, flags):
        return self.getStep(flags,1,'JetRecoOnlyCfg',[UTTJetRecoSequence])
    # Empty for alignment
    def getIsoHPtTrackEmpty(self, flags):
        return  self.getEmptyStep(1,'EmptyUncTrk')
    def getFSLRTEmpty(self, flags):
        return self.getEmptyStep(1, 'FSLRTEmptyStep')
    def getDisTrkEmpty(self, flags):
        return self.getEmptyStep(1, 'DisTrkEmptyStep')
    def getVSIEmpty(self, flags):
        return self.getEmptyStep(1, 'VSIEmptyStep')
    def getdEdxEmpty(self, flags):
        return self.getEmptyStep(1, 'dEdxEmptyStep')

    # Step 2
    def getFSLRTTrigger(self, flags):
        return self.getStep(flags,2,'FSLRT',[FullScanLRTMenuSequence])
    # Empty for alignment with jets
    def getRoITrkEmpty(self, flags):
        return self.getEmptyStep(2, 'RoITrkEmptyStep')

    # Step 3 -- all FTF tracking here
    def getFTFTrackReco(self, flags):
        return self.getStep(flags,3,'FTFRecoOnly',[getFullScanRecoOnlySequence])

    # Step 4+ -- everything post FTF tracking
    def getIsoHPtTrackTrigger(self, flags):
        return self.getStep(flags,4,'IsoHPtTrack',[IsoHPtTrackTriggerHypoSequence])
    def getdEdxTrigger(self, flags):
        return self.getStep(flags,4,'dEdx',[dEdxTriggerHypoSequence])
    def getHitDVTrigger(self, flags):
        return self.getStep(flags,4,'HitDV',[HitDVHypoSequence])
    def getDisTrkTrigger(self, flags):
        return self.getStep(flags,4,'DisTrk',[DisTrkTriggerHypoSequence])
    def getVSITrigger(self, flags):
        return self.getStep(flags,4,'LRTVSI',[VrtSecInclusiveMenuSequence])
    def getDJPromptStep(self, flags):
        return self.getStep(flags,3,'DJPromptStep',[DJPromptStep], comboTools = [TrigDJComboHypoToolFromDict])
    def getDJDispStep(self, flags):
        return self.getStep(flags,4,'DJDispStep',[DJDispStep])
    def getDVRecoStep(self, flags):
        return self.getStep(flags,5,'DVRecoStep',[DVRecoSequence])
    def getDVEDStep(self, flags):
        return self.getStep(flags,6,'DVEDStep',[DVTriggerEDSequence])
