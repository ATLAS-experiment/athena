# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TrigLongLivedParticlesHypo.TrigDJHypoConfig import TrigDJComboHypoToolFromDict

from TriggerMenuMT.HLT.UnconventionalTracking.CommonConfiguration import getFullScanRecoOnlySequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.DJTriggerConfiguration import DJPromptStepSequenceGenCfg, DJDispStepSequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.DVTriggerConfiguration import DVRecoSequenceGenCfg, DVTriggerEDSequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.DisTrkTriggerConfiguration import DisTrkTriggerHypoSequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.FullScanLRTTrackingConfiguration import FullScanLRTMenuSequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.HitDVConfiguration import HitDVHypoSequenceGenCfg, UTTJetRecoSequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.IsoHighPtTrackTriggerConfiguration import IsoHPtTrackTriggerHypoSequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.VrtSecInclusiveConfiguration import VrtSecInclusiveMenuSequenceGenCfg
from TriggerMenuMT.HLT.UnconventionalTracking.dEdxTriggerConfiguration import dEdxTriggerHypoSequenceGenCfg

#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class UnconventionalTrackingChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)

    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):
        log.debug("Assembling chain %s", self.chainName)
        
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
        return self.getStep(flags, 'JetRecoOnlyCfg',[UTTJetRecoSequenceGenCfg])
    # Empty for alignment
    def getIsoHPtTrackEmpty(self, flags):
        return  self.getEmptyStep('EmptyUncTrk')
    def getFSLRTEmpty(self, flags):
        return self.getEmptyStep('FSLRTEmptyStep')
    def getDisTrkEmpty(self, flags):
        return self.getEmptyStep('DisTrkEmptyStep')
    def getVSIEmpty(self, flags):
        return self.getEmptyStep('VSIEmptyStep')
    def getdEdxEmpty(self, flags):
        return self.getEmptyStep('dEdxEmptyStep')

    # Step 2
    def getFSLRTTrigger(self, flags):
        return self.getStep(flags, 'FSLRT',[FullScanLRTMenuSequenceGenCfg])
    # Empty for alignment with jets
    def getRoITrkEmpty(self, flags):
        return self.getEmptyStep('RoITrkEmptyStep')

    # Step 3 -- all FTF tracking here
    def getFTFTrackReco(self, flags):
        return self.getStep(flags, 'FTFRecoOnly',[getFullScanRecoOnlySequenceGenCfg])

    # Step 4+ -- everything post FTF tracking
    def getIsoHPtTrackTrigger(self, flags):
        return self.getStep(flags, 'IsoHPtTrack',[IsoHPtTrackTriggerHypoSequenceGenCfg])
    def getdEdxTrigger(self, flags):
        return self.getStep(flags, 'dEdx',[dEdxTriggerHypoSequenceGenCfg])
    def getHitDVTrigger(self, flags):
        return self.getStep(flags, 'HitDV',[HitDVHypoSequenceGenCfg])
    def getDisTrkTrigger(self, flags):
        return self.getStep(flags, 'DisTrk',[DisTrkTriggerHypoSequenceGenCfg])
    def getVSITrigger(self, flags):
        return self.getStep(flags, 'LRTVSI',[VrtSecInclusiveMenuSequenceGenCfg])
    def getDJPromptStep(self, flags):
        return self.getStep(flags, 'DJPromptStep',[DJPromptStepSequenceGenCfg], comboTools = [TrigDJComboHypoToolFromDict])
    def getDJDispStep(self, flags):
        return self.getStep(flags, 'DJDispStep',[DJDispStepSequenceGenCfg])
    def getDVRecoStep(self, flags):
        return self.getStep(flags, 'DVRecoStep',[DVRecoSequenceGenCfg])
    def getDVEDStep(self, flags):
        return self.getStep(flags, 'DVEDStep',[DVTriggerEDSequenceGenCfg])
