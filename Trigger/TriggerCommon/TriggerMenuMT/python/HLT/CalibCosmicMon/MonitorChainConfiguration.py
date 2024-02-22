# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigGenericAlgs.TrigGenericAlgsConfig import TimeBurnerCfg, TimeBurnerHypoToolGen, L1CorrelationAlgCfg
from TrigHypoCommonTools.TrigHypoCommonTools import TrigGenericHypoToolFromDict
from TrigEDMConfig.TriggerEDM import recordable

#----------------------------------------------------------------
# fragments generating configuration will be functions in New JO, 
# so let's make them functions already now
#----------------------------------------------------------------
def timeBurnerCfg(flags):
    # Input maker - required by the framework, but inputs don't matter for TimeBurner
    inputMaker = CompFactory.InputMakerForRoI("IM_TimeBurner",
                                              RoITool=CompFactory.ViewCreatorInitialROITool(),
                                              RoIs="TimeBurnerInputRoIs",
    )
    reco = InEventRecoCA('TimeBurner_reco',inputMaker=inputMaker)
    # TimeBurner alg works as a reject-all hypo
    selAcc = SelectionCA('TimeBurnerSequence')
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(
        TimeBurnerCfg(flags,
                      name="TimeBurnerHypo",
                      SleepTimeMillisec=200
        )
    )

    msca = MenuSequenceCA(flags, selAcc,
                          HypoToolGen=TimeBurnerHypoToolGen)
    return msca


def MistimeMonSequenceCfg(flags):
        inputMaker = CompFactory.InputMakerForRoI("IM_MistimeMon",
                                                  RoITool = CompFactory.ViewCreatorInitialROITool(),
                                                  RoIs="MistimeMonInputRoIs",
        )

        outputName = recordable("HLT_TrigCompositeMistimeJ400")
        reco = InEventRecoCA('Mistime_reco',inputMaker=inputMaker)
        recoAlg = L1CorrelationAlgCfg(flags, "MistimeMonj400", ItemList=['L1_J400','L1_jJ500'],
                                      TrigCompositeWriteHandleKey=outputName, trigCompPassKey=outputName+".pass",
                                      l1AKey=outputName+".l1a_type", otherTypeKey=outputName+".other_type",
                                      beforeAfterKey=outputName+".beforeafterflag")
        reco.addRecoAlgo(recoAlg)
        selAcc =  SelectionCA("MistimeMonSequence")
        selAcc.mergeReco(reco)

        # Hypo to select on trig composite pass flag
        hypoAlg = CompFactory.TrigGenericHypoAlg("MistimeMonJ400HypoAlg", TrigCompositeContainer=outputName)
        selAcc.addHypoAlgo(hypoAlg)

        return MenuSequenceCA(flags, selAcc,
                HypoToolGen = TrigGenericHypoToolFromDict)


#----------------------------------------------------------------
# Class to configure chain
#----------------------------------------------------------------
class MonitorChainConfiguration(ChainConfigurationBase):

    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self,chainDict)
        
    # ----------------------
    # Assemble the chain depending on information from chainName
    # ----------------------
    def assembleChainImpl(self, flags):                            
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        monTypeList = self.chainPart.get('monType')
        if not monTypeList:
            raise RuntimeError('No monType defined in chain ' + self.chainName)
        if len(monTypeList) > 1:
            raise RuntimeError('Multiple monType values defined in chain ' + self.chainName)
        monType = monTypeList[0]

        if monType == 'timeburner':
            chainSteps.append(self.getTimeBurnerStep(flags))
        elif monType == 'mistimemonj400':
            chainSteps.append(self.getMistimeMonStep(flags))
        else:
            raise RuntimeError('Unexpected monType '+monType+' in MonitorChainConfiguration')

        return self.buildChain(chainSteps)

    # --------------------
    # TimeBurner configuration
    # --------------------
    def getTimeBurnerStep(self, flags):
        return self.getStep(flags,1,'TimeBurner',[timeBurnerCfg])

    # --------------------
    # MistTimeMon configuration
    # --------------------
    def getMistimeMonStep(self, flags):
        return self.getStep(flags,1,'MistimeMon',[MistimeMonSequenceCfg])
