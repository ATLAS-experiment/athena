# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, EmptyMenuSequenceCA
from TrigEDMConfig.TriggerEDM import recordable
import AthenaCommon.SystemOfUnits as Units

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase

def TrackCountHypoToolGen(chainDict):
    hypo = CompFactory.TrackCountHypoTool(chainDict["chainName"])
    hypo.minNtrks = 1
    return hypo

def CosmicsTrkSequenceCfg(flags):

    trkRecoSeq = InViewRecoCA("CosmicTrkRecoSeq", InViewRoIs = "CosmicRoIs")

    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flagsWithTrk = getFlagsForActiveConfig(flags, "cosmics", log)

    from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence
    seq = InDetTrigSequence(flagsWithTrk, flagsWithTrk.Tracking.ActiveConfig.input_name, 
                            rois ="CosmicRoIs", inView = "VDVCosmicsIDTracking")


    idTrackingAlgs = seq.sequence("Offline")
    trkRecoSeq.mergeReco(idTrackingAlgs)

    trackCountHypo = CompFactory.TrackCountHypoAlg("CosmicsTrackCountHypoAlg", 
        minPt = [100*Units.MeV],
        maxZ0 = [401*Units.mm],
        vertexZ = [803*Units.mm])
    trackCountHypo.tracksKey = recordable("HLT_IDTrack_Cosmic_IDTrig")
    trackCountHypo.trackCountKey = "HLT_CosmicsTrackCount" # irrelevant, not recorded

    #TODO move a complete configuration of the algs to TrigMinBias package
    from TrigMinBias.TrigMinBiasMonitoring import TrackCountMonitoring
    trackCountHypo.MonTool = TrackCountMonitoring(flags, trackCountHypo) # monitoring tool configures itself using config of the hypo alg

    
    trkSequence = SelectionCA("CosmicTrkSequence")
    trkSequence.mergeReco(trkRecoSeq)
    trkSequence.addHypoAlgo(trackCountHypo)
    log.debug("Prepared ID tracking sequence")
    log.debug(trkSequence)
    return MenuSequenceCA(flags,
                          trkSequence,
                          HypoToolGen = TrackCountHypoToolGen)


def EmptyMSBeforeCosmicID(flags):
    return EmptyMenuSequenceCA("EmptyBeforeCosmicID")

#----------------------------------------------------------------
class CosmicChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):       
                         
        steps = []
        log.debug("Assembling chain for %s", self.chainName)
        # --------------------
        # define here the names of the steps and obtain the chainStep configuration         
        # --------------------
        if 'cosmic_id' in self.chainName:
            steps += [  self.getStep(flags, 1, 'Empty', [EmptyMSBeforeCosmicID]),
                        self.getStep(flags, 2, 'CosmicTracking', [CosmicsTrkSequenceCfg]) ]

        return self.buildChain(steps)


