# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigGenericAlgs.TrigGenericAlgsConfig import TimeBurnerCfg, TimeBurnerHypoToolGen, L1CorrelationAlgCfg
from L1TopoOnlineMonitoring import L1TopoOnlineMonitoringConfig as TopoMonConfig
from L1TopoSimulation import L1TopoSimulationConfig as TopoSimConfig
from TrigHypoCommonTools.TrigHypoCommonTools import TrigGenericHypoToolFromDict
from TrigEDMConfig.TriggerEDM import recordable
from AthenaCommon.CFElements import seqAND

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

    msca = MenuSequence(flags, selAcc,
                          HypoToolGen=TimeBurnerHypoToolGen)
    return msca


#----------------------------------------------------------------
def LArSuperCellMonitoringGenCfg(flags):
   from LArMonitoring.LArSuperCellMonAlg import LArSuperCellMonConfigHLT
   # Input maker - required by the framework, but inputs don't matter for LArSuperCell
   inputMaker = CompFactory.InputMakerForRoI("IM_LArSuperCellMon",
                                             RoITool=CompFactory.ViewCreatorInitialROITool(),
                                             RoIs="LArSuperCellMonRoIs",
   )
   reco = InEventRecoCA('LArSuperCellMonitoring',inputMaker=inputMaker)
   reco.merge( LArSuperCellMonConfigHLT(flags) )
   # TimeBurner alg works as a reject-all hypo
   selAcc = SelectionCA('LArSuperCellMonitoringSequence')
   selAcc.mergeReco(reco)
   selAcc.addHypoAlgo(
       TimeBurnerCfg(flags,
                     name="LArSuperCellMonHypoConfig",
                     SleepTimeMillisec=0
       )
   )

   # TimeBurnerHypo is never even called
   msca = MenuSequence(flags, selAcc,
                         HypoToolGen=TimeBurnerHypoToolGen)
   return msca

def L1TopoOnlineMonitorSequenceCfg(flags):

        # Input maker for FS initial RoI
        inputMaker = CompFactory.InputMakerForRoI("IM_L1TopoOnlineMonitor")
        inputMaker.RoITool = CompFactory.ViewCreatorInitialROITool()
        inputMaker.RoIs="L1TopoOnlineMonitorInputRoIs"

        reco = InEventRecoCA('L1TopoPhase1OnlineMonitor_reco',inputMaker=inputMaker)

        readMuCTPI_local=True
        if flags.Trigger.doLVL1:
            readMuCTPI_local=False
        
        reco.addSequence(seqAND('L1TopoSimSeq'))
        reco.merge(TopoSimConfig.L1TopoSimulationCfg(flags,doMonitoring=True,readMuCTPI=readMuCTPI_local,name="L1OnlineTopoSimulation"), sequenceName='L1TopoSimSeq')
    
        selAcc =  SelectionCA("L1TopoOnlineMonitorSequence")
        selAcc.mergeReco(reco)

        hypoAlg = TopoMonConfig.getL1TopoOnlineMonitorHypo(flags)
        selAcc.addHypoAlgo(hypoAlg)

        return MenuSequence(flags, selAcc,
                              HypoToolGen = TopoMonConfig.L1TopoOnlineMonitorHypoToolGen)


def MistimeMonSequenceCfg(flags):
        inputMaker = CompFactory.InputMakerForRoI("IM_MistimeMon",
                                                  RoITool = CompFactory.ViewCreatorInitialROITool(),
                                                  RoIs="MistimeMonInputRoIs",
        )

        outputName = recordable("HLT_TrigCompositeMistimeJ400")
        reco = InEventRecoCA('Mistime_reco',inputMaker=inputMaker)
        recoAlg = L1CorrelationAlgCfg(flags, "MistimeMonj400", ItemList=['L1_J400','L1_gJ400p0ETA25'],
                                      TrigCompositeWriteHandleKey=outputName, trigCompPassKey=outputName+".pass",
                                      l1AKey=outputName+".l1a_type", otherTypeKey=outputName+".other_type",
                                      beforeAfterKey=outputName+".beforeafterflag")
        reco.addRecoAlgo(recoAlg)
        selAcc =  SelectionCA("MistimeMonSequence")
        selAcc.mergeReco(reco)

        # Hypo to select on trig composite pass flag
        hypoAlg = CompFactory.TrigGenericHypoAlg("MistimeMonJ400HypoAlg", TrigCompositeContainer=outputName)
        selAcc.addHypoAlgo(hypoAlg)

        return MenuSequence(flags, selAcc,
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
        elif monType == 'larsupercellmon':
            chainSteps.append(self.getLArSuperCellMonitoringGenCfg(flags))
        elif monType == 'l1topoPh1debug':
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
        return self.getStep(flags, 'TimeBurner',[timeBurnerCfg])

    # --------------------
    # LArSuperCellMon configuration
    # --------------------
    def getLArSuperCellMonitoringGenCfg(self, flags):
        return self.getStep(flags, 'larsupercellmon',[LArSuperCellMonitoringGenCfg])

    # --------------------
    # L1TopoOnlineMonitor configuration
    # --------------------
    def getL1TopoOnlineMonitorStep(self, flags):

        sequenceCfg = L1TopoOnlineMonitorSequenceCfg
        return self.getStep(flags, 'L1TopoOnlineMonitor',[sequenceCfg])

    # --------------------
    # MistTimeMon configuration
    # --------------------
    def getMistimeMonStep(self, flags):
        return self.getStep(flags, 'MistimeMon',[MistimeMonSequenceCfg])
