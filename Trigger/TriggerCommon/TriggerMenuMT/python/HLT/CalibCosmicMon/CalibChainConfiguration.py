# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigT2CaloCommon.CaloDef import fastCaloRecoSequenceCfg
from TrigGenericAlgs.TrigGenericAlgsConfig import TimeBurnerCfg, TimeBurnerHypoToolGen
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from TrigTrackingHypo.IDCalibHypoConfig import IDCalibHypoToolFromDict, createIDCalibHypoAlg
from ..CommonSequences.FullScanInDetConfig import commonInDetFullScanCfg
from TriggerMenuMT.HLT.Jet.JetMenuSequencesConfig import getTrackingInputMaker

from TrigCaloRec.TrigCaloRecConfig import hltCaloCellMakerCfg
from TrigCaloHypo.TrigCaloHypoConfig import TrigLArNoiseBurstRecoAlgCfg
from TrigCaloHypo.TrigCaloHypoConfig import TrigLArNoiseBurstHypoToolGen
from TrigT2CaloCommon.CaloDef import clusterFSInputMaker


def getLArNoiseBurstRecoCfg(flags):
    acc = InEventRecoCA("LArNoiseBurstRecoSequence", inputMaker=clusterFSInputMaker())
    cells_name = 'CaloCellsFS' 
    acc.mergeReco(hltCaloCellMakerCfg(flags=flags, name="HLTCaloCellMakerFS", roisKey=''))
    acc.mergeReco(TrigLArNoiseBurstRecoAlgCfg(flags, cells_name))
    return acc


# --------------------
# LArNoiseBurst configuration
# --------------------
@AccumulatorCache
def getLArNoiseBurstSequenceGenCfg(flags):

    hypoAlg = CompFactory.TrigLArNoiseBurstAlg("NoiseBurstAlg")
    InEventReco = InEventRecoCA("LArNoiseBurstRecoSequence", inputMaker=clusterFSInputMaker())

    noiseBurstRecoSeq = getLArNoiseBurstRecoCfg(flags)

    InEventReco.mergeReco(noiseBurstRecoSeq)
    selAcc = SelectionCA("LArNoiseBurstMenuSequence")
    selAcc.mergeReco(InEventReco)
    selAcc.addHypoAlgo(hypoAlg)
    
    return MenuSequence(flags,selAcc,HypoToolGen=TrigLArNoiseBurstHypoToolGen)
    

# --------------------
# LArPS Noise Detection EM configuration
# --------------------
@AccumulatorCache
def getCaloAllEMLayersPSSequenceGenCfg(flags,doAllorAllEM=False):

    from TrigT2CaloCommon.CaloDef import fastCaloVDVCfg
    nameselAcc = "LArPSSequence_All"
    namerecoAcc = "fastCaloInViewSequenceAllEM"
    hypoAlgName = "TrigL2CaloLayersAlg_AllEM"
    output = "HLT_LArPS_AllCaloEMClusters"
    if doAllorAllEM :
       nameselAcc = "LArPSSequence_AllEM"
       namerecoAcc = "fastCaloInViewSequenceAll"
       hypoAlgName = "TrigL2CaloLayersAlg_All"
       output = "HLT_LArPS_AllCaloClusters"
    selAcc = SelectionCA(nameselAcc)
    InViewRoIs="EMCaloRoIs"
    reco = InViewRecoCA(namerecoAcc,InViewRoIs=InViewRoIs)
    reco.mergeReco(fastCaloVDVCfg(flags,InViewRoIs=InViewRoIs))
    reco.mergeReco(fastCaloRecoSequenceCfg(flags, inputEDM=InViewRoIs,ClustersName=output,doAllEm=not doAllorAllEM,doAll=doAllorAllEM))

    selAcc.mergeReco(reco)
    
    from TrigCaloHypo.TrigCaloHypoConfig import TrigL2CaloLayersHypoToolGen
    TrigL2CaloLayersAlg = CompFactory.TrigL2CaloLayersAlg(hypoAlgName)
    TrigL2CaloLayersAlg.TrigClusterContainerKey = output
    selAcc.addHypoAlgo(TrigL2CaloLayersAlg)
    return MenuSequence(flags,selAcc,HypoToolGen=TrigL2CaloLayersHypoToolGen)


#----------------------------------------------------------------

class CalibChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):       
                         
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        stepDictionary = self.getStepDictionary()
                
        if 'acceptedevts' in self.chainPart['purpose']:
            steps=stepDictionary['AcceptedEvents']
        elif self.chainPart['purpose'][0] == 'larnoiseburst':
            steps=stepDictionary['LArNoiseBurst']
        elif self.chainPart['purpose'][0] == 'larpsallem':
            steps=stepDictionary['LArPSAllEM']
        elif self.chainPart['purpose'][0] == 'larpsall':
            steps=stepDictionary['LArPSAll']
        elif self.chainPart['purpose'][0] == 'idcalib':
            steps=stepDictionary['IDCalib']
        for i, step in enumerate(steps): 
            chainstep = getattr(self, step)(flags, i)
            chainSteps+=[chainstep]

        myChain = self.buildChain(chainSteps)
        return myChain


    def getStepDictionary(self):
        # --------------------
        # define here the names of the steps and obtain the chainStep configuration 
        # --------------------
        stepDictionary = {
            "AcceptedEvents": ['getAcceptedEventsStep'],
            "LArNoiseBurst": ['getAllTEStep'],
            "LArPSAllEM" : ['getCaloAllEMStep'],
            "LArPSAll" : ['getCaloAllStep'],
            "IDCalib": ['getIDCalibEmpty', 'getIDCalibEmpty', 'getIDCalibFTFReco', 'getIDCalibTrigger']
        }
        return stepDictionary


    def getAcceptedEventsStep(self, flags, i):
        return self.getStep(flags, 'AcceptedEvents', [acceptedEventsSequenceGenCfg])

    def getAllTEStep(self, flags, i):
        return self.getStep(flags, 'LArNoiseBurst', [getLArNoiseBurstSequenceGenCfg])

    def getCaloAllEMStep(self, flags, i):
        return self.getStep(flags, 'LArPSALLEM', [getCaloAllEMLayersPSSequenceGenCfg], doAllorAllEM=False)

    def getCaloAllStep(self, flags, i):
        return self.getStep(flags, 'LArPSALL', [getCaloAllEMLayersPSSequenceGenCfg], doAllorAllEM=True)

    def getIDCalibEmpty(self, flags, i):
        return self.getEmptyStep('IDCalibEmptyStep')

    def getIDCalibFTFReco(self, flags, i):
        return self.getStep(flags, 'IDCalibFTFCfg',[IDCalibFTFSequenceGenCfg])

    def getIDCalibTrigger(self, flags, i):
        return self.getStep(flags, 'IDCalibTriggerCfg',[IDCalibTriggerSequenceGenCfg])

#----------------------------------------------------------------

# --------------------
# IDCalib trigger configurations
# --------------------

@AccumulatorCache
def IDCalibTriggerSequenceGenCfg(flags):
    DummyInputMakerAlg = CompFactory.InputMakerForRoI( "IM_IDCalib_HypoOnlyStep" )
    DummyInputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()

    reco = InEventRecoCA('IDCalibEmptySeq_reco',inputMaker=DummyInputMakerAlg)

    theHypoAlg = createIDCalibHypoAlg(flags, "IDCalibHypo")
    theHypoAlg.tracksKey = flags.Trigger.InDetTracking.fullScan.tracks_FTF

    selAcc = SelectionCA('IDCalibEmptySeq_sel')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(theHypoAlg)

    msca = MenuSequence(
        flags, selAcc, 
        HypoToolGen=IDCalibHypoToolFromDict,
    )
    return msca
    

# --------------------

@AccumulatorCache
def IDCalibFTFSequenceGenCfg(flags):
    reco = InEventRecoCA('IDCalibTrkrecoSeq_reco',inputMaker=getTrackingInputMaker(flags, "ftf"))
    reco.mergeReco(commonInDetFullScanCfg(flags))

    selAcc = SelectionCA('IDCalibTrkrecoSeq')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(CompFactory.TrigStreamerHypoAlg("IDCalibTrkDummyStream"))

    msca = MenuSequence(
        flags, selAcc,
        HypoToolGen = lambda chainDict: CompFactory.TrigStreamerHypoTool(chainDict['chainName'])
    )
    return msca


#----------------------------------------------------------------

# --------------------
# HLT step for the AcceptedEvents chains
# --------------------
@AccumulatorCache
def acceptedEventsSequenceGenCfg(flags):
    '''
    Return MenuSequence for an HLT step used by the AcceptedEvents chains. This step is a trivial
    always-reject hypo with no reco. The step itself should be noop as only the HLTSeeding and the
    end-of-event sequence parts of AcceptedEvents chains are actually used.
    '''
    # Implementation identical to the timeburner chain but with zero sleep time

    inputMaker = CompFactory.InputMakerForRoI(
        "IM_AcceptedEvents",
        RoITool = CompFactory.ViewCreatorInitialROITool(),
        RoIs="AcceptedEventsRoIs",
    )
    reco = InEventRecoCA('AcceptedEvents_reco',inputMaker=inputMaker)
    # TimeBurner alg works as a reject-all hypo
    selAcc = SelectionCA('AcceptedEventsSequence')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(
        TimeBurnerCfg(
            flags,
            name="AcceptedEventsHypo",
            SleepTimeMillisec = 0,
        )
    )

    msca = MenuSequence(
        flags, selAcc,
        HypoToolGen=TimeBurnerHypoToolGen
    )
    return msca

