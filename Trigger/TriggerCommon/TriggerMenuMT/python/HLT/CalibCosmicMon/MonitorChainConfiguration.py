# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigGenericAlgs.TrigGenericAlgsConfig import TimeBurnerCfg, TimeBurnerHypoToolGen, L1CorrelationAlgCfg
from L1TopoOnlineMonitoring import L1TopoOnlineMonitoringConfig as TopoMonConfig
from L1TopoSimulation import L1TopoSimulationConfig as TopoSimConfig
from TrigHypoCommonTools.TrigHypoCommonTools import TrigGenericHypoToolFromDict
from TrigEDMConfig.TriggerEDM import recordable
from AthenaConfiguration.Enums import Format

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

def L1TopoOnlineMonitorSequenceCfg(flags):

        # Input maker for FS initial RoI
        inputMaker = CompFactory.InputMakerForRoI("IM_L1TopoOnlineMonitor")
        inputMaker.RoITool = CompFactory.ViewCreatorInitialROITool()
        inputMaker.RoIs="L1TopoOnlineMonitorInputRoIs"

        reco = InEventRecoCA('L1TopoPhase1OnlineMonitor_reco',inputMaker=inputMaker)

        # if running on data without L1Sim, need to add L1TopoSim
        if flags.Input.Format is Format.BS and not flags.Trigger.doLVL1:
            topoSimCA = TopoSimConfig.L1TopoSimulationCfg(flags,doMonitoring=True,readMuCTPI=True)
            reco.mergeReco(topoSimCA)
        # in other hand, we only need to add the L1TopoPhase1OnlineMonitor
        else:
            recoAlg= TopoMonConfig.getL1TopoPhase1OnlineMonitor(flags,'L1/L1TopoSimDecisions')
            reco.addEventAlgo(recoAlg)
    
        selAcc =  SelectionCA("L1TopoOnlineMonitorSequence")
        selAcc.mergeReco(reco)

        hypoAlg = TopoMonConfig.getL1TopoOnlineMonitorHypo(flags)
        selAcc.addHypoAlgo(hypoAlg)

        return MenuSequenceCA(flags, selAcc,
                              HypoToolGen = TopoMonConfig.L1TopoOnlineMonitorHypoToolGen)


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
        elif monType == 'l1topoPh1debug':
            #Deactivated by default at the moment
            if False and not flags.Trigger.doLVL1 and flags.Input.Format is Format.BS and flags.Trigger.L1.doMuonTopoInputs and flags.Trigger.L1.doMuon and flags.enableL1MuonPhase1 and flags.enableL1TopoBWSimulation and flags.enableL1CaloPhase1:
                chainSteps.append(self.getL1TopoOnlineMonitorStep(flags))
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
    # L1TopoOnlineMonitor configuration
    # --------------------
    def getL1TopoOnlineMonitorStep(self, flags):

        sequenceCfg = L1TopoOnlineMonitorSequenceCfg
        return self.getStep(flags,1,'L1TopoOnlineMonitor',[sequenceCfg])

    # --------------------
    # MistTimeMon configuration
    # --------------------
    def getMistimeMonStep(self, flags):
        return self.getStep(flags,1,'MistimeMon',[MistimeMonSequenceCfg])
