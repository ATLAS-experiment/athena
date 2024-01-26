# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import parOR
from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
from TriggerMenuMT.HLT.CommonSequences.FullScanDefs import caloFSRoI
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg, CaloDataAccessSvcDependencies

########################
## ALGORITHMS
# defined as private within this module, so that they can be configured only in functions in this module
########################

def _algoHLTCaloCell(flags, name="HLTCaloCellMaker", inputEDM='', outputEDM='CellsClusters', RoIMode=True) :
   if not inputEDM:
      from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
      inputEDM = mapThresholdToL1RoICollection("FSNOSEED")

   from AthenaCommon.AppMgr import ServiceMgr as svcMgr
   from TrigCaloRec.TrigCaloRecConfig import HLTCaloCellMaker
   algo=HLTCaloCellMaker(flags, name, roisKey=inputEDM)
   #"HLTCaloCellMaker"
   algo.RoIs=inputEDM
   algo.TrigDataAccessMT=svcMgr.TrigCaloDataAccessSvc
   algo.CellsName=outputEDM
   algo.ExtraInputs |= { ( 'LArBadChannelCont', 'ConditionStore+LArBadChannel'), ( 'LArMCSym', 'ConditionStore+LArMCSym'), ('LArOnOffIdMapping' , 'ConditionStore+LArOnOffIdMap' ), ('LArFebRodMapping'  , 'ConditionStore+LArFebRodMap' ), ('CaloDetDescrManager', 'ConditionStore+CaloDetDescrManager') }
   return algo

def _algoHLTHIEventShape(flags,name='HLTEventShapeMaker', inputEDM='CellsClusters', outputEDM='HIEventShape'):
    algo = CompFactory.HIEventShapeMaker(name = name,
                                         InputCellKey = inputEDM,
                                         InputTowerKey="",
                                         NaviTowerKey="",
                                         OutputContainerKey = outputEDM,
                                         HIEventShapeFillerTool = CompFactory.HIEventShapeFillerTool())
    return algo

def _algoHLTCaloCellCorrector(name='HLTCaloCellCorrector', inputEDM='CellsClusters', outputEDM='CorrectedCellsClusters', eventShape='HIEventShape'):
  from TrigCaloRec.TrigCaloRecConf import HLTCaloCellCorrector

  algo = HLTCaloCellCorrector(name)
  algo.EventShapeCollection = eventShape
  algo.InputCellKey = inputEDM
  algo.OutputCellKey = outputEDM

  return algo

def _algoHLTTopoCluster(flags, inputEDM="CellsClusters", algSuffix="") :
   from TrigCaloRec.TrigCaloRecConfig import hltTopoClusterMakerCfg
   algo = algorithmCAToGlobalWrapper(hltTopoClusterMakerCfg, flags,
                                     name="TrigCaloClusterMaker_topo"+algSuffix,
                                     doLC=False,
                                     clustersKey="HLT_TopoCaloClusters"+algSuffix,
                                     cellsKey=inputEDM)[0]
   return algo

def _algoHLTTopoClusterLC(flags, inputEDM="CellsClusters", algSuffix="") :
   from TrigCaloRec.TrigCaloRecConfig import hltTopoClusterMakerCfg
   algo = algorithmCAToGlobalWrapper(hltTopoClusterMakerCfg, flags,
                                     name="TrigCaloClusterMaker_topo"+algSuffix,
                                     doLC=True,
                                     clustersKey="HLT_TopoCaloClusters"+algSuffix,
                                     cellsKey=inputEDM)[0]
   return algo


#
# fast calo algorithm (central or forward regions)
#
@AccumulatorCache
def fastCaloRecoSequenceCfg(flags, inputEDM="", ClustersName="HLT_FastCaloEMClusters", RingerKey="HLT_FastCaloRinger", doForward=False, doAllEm=False, doAll=False):

    acc = ComponentAccumulator()
    acc.merge(trigCaloDataAccessSvcCfg(flags))
    if not inputEDM:
        from HLTSeeding.HLTSeedingConfig import mapThresholdToL1RoICollection
        # using jet seeds for testing. we should use EM as soon as we have EM seeds into the L1
        inputEDM = mapThresholdToL1RoICollection("EM")

    from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import t2CaloEgamma_ReFastAlgoCfg
    if (not doForward) and (not doAll) and (not doAllEm ) :
       acc.merge(t2CaloEgamma_ReFastAlgoCfg(flags, "FastCaloL2EgammaAlg", doRinger=True, RingerKey=RingerKey,RoIs=inputEDM,ExtraInputs=CaloDataAccessSvcDependencies, ClustersName = ClustersName))
    if doForward:
        from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import t2CaloEgamma_ReFastFWDAlgoCfg
        acc.merge(t2CaloEgamma_ReFastFWDAlgoCfg(flags, "FastCaloL2EgammaAlg_FWD", doRinger=True, RingerKey=RingerKey,RoIs=inputEDM,ExtraInputs=CaloDataAccessSvcDependencies, ClustersName = ClustersName))
    else:
        if ( doAllEm or doAll ) :
            if ( doAllEm ):
                from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import t2CaloEgamma_AllEmCfg
                acc.merge(t2CaloEgamma_AllEmCfg(flags, "L2CaloLayersEmFex",RoIs=inputEDM,ExtraInputs= CaloDataAccessSvcDependencies, ClustersName = ClustersName))
            else : # can only be doAll
                from TrigT2CaloEgamma.TrigT2CaloEgammaConfig import t2CaloEgamma_AllCfg
                acc.merge(t2CaloEgamma_AllCfg(flags, "L2CaloLayersFex",RoIs=inputEDM,ExtraInputs=CaloDataAccessSvcDependencies, ClustersName = ClustersName))
    return acc

def fastCaloVDVCfg(name="fastCaloVDV",InViewRoIs="EMCaloRoIs") :
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    reco = ComponentAccumulator()
    fastCaloVDV = CompFactory.AthViews.ViewDataVerifier(name)
    fastCaloVDV.DataObjects = {( 'CaloBCIDAverage' , 'StoreGateSvc+CaloBCIDAverage' ),
                               ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s'%InViewRoIs )}
    reco.addEventAlgo(fastCaloVDV)
    return reco

####################################
##### SEQUENCES
####################################

##################################
# cluster maker functions
###################################

def clusterFSInputMaker( ):
  """Creates the inputMaker for FS in menu"""
  RoIs = caloFSRoI
  from AthenaConfiguration.ComponentFactory import CompFactory
  InputMakerAlg = CompFactory.InputMakerForRoI("IMclusterFS", RoIsLink="initialRoI")
  InputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()
  InputMakerAlg.RoIs=RoIs
  return InputMakerAlg


def HLTCellMaker(flags,RoIs=caloFSRoI, outputName="CaloCells", algSuffix=""):
    cellMakerAlgo = _algoHLTCaloCell(flags, name="HLTCaloCellMaker"+algSuffix, inputEDM=RoIs, outputEDM=outputName, RoIMode=True)
    return cellMakerAlgo


def HLTFSCellMakerRecoSequence(flags,RoIs=caloFSRoI):
    cellMaker = HLTCellMaker(flags, RoIs, outputName="CaloCellsFS", algSuffix="FS")
    RecoSequence = parOR("ClusterRecoSequenceFS", [cellMaker])
    return (RecoSequence, cellMaker.CellsName)


def HLTFSTopoRecoSequence(flags,RoIs):
    cellMaker = HLTCellMaker(flags, RoIs, outputName="CaloCellsFS", algSuffix="FS")
    topoClusterMaker = _algoHLTTopoCluster(flags, inputEDM = cellMaker.CellsName, algSuffix="FS")
    RecoSequence = parOR("TopoClusterRecoSequenceFS", [cellMaker, topoClusterMaker])
    return (RecoSequence, topoClusterMaker.CaloClusters)


def HLTRoITopoRecoSequence(flags, RoIs, algSuffix=''):
    import AthenaCommon.CfgMgr as CfgMgr
    HLTRoITopoRecoSequenceVDV = CfgMgr.AthViews__ViewDataVerifier("HLTRoITopoRecoSequenceVDV%s"%algSuffix)
    HLTRoITopoRecoSequenceVDV.DataObjects = {( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+PrecisionCaloRoIs%s'%algSuffix ),
                                             ( 'CaloBCIDAverage' , 'StoreGateSvc+CaloBCIDAverage' ),
                                             ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' )}

    cellMaker = HLTCellMaker(flags, RoIs, algSuffix="RoI%s"%algSuffix)
    topoClusterMaker = _algoHLTTopoCluster(flags, inputEDM = cellMaker.CellsName, algSuffix="RoI%s"%algSuffix)
    RecoSequence = parOR("RoITopoClusterRecoSequence%s"%algSuffix, [HLTRoITopoRecoSequenceVDV, cellMaker, topoClusterMaker])
    return (RecoSequence, topoClusterMaker.CaloClusters)


def HLTHIRoITopoRecoSequence(flags, RoIs, algSuffix=''):

    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import  getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys()
    eventShape = TrigEgammaKeys.egEventShape

    import AthenaCommon.CfgMgr as CfgMgr
    HLTRoITopoRecoSequenceVDV = CfgMgr.AthViews__ViewDataVerifier("HLTHIRoITopoRecoSequenceVDV")
    HLTRoITopoRecoSequenceVDV.DataObjects = {( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+PrecisionCaloRoIs' ),
                                             ( 'xAOD::HIEventShapeContainer' , 'StoreGateSvc+' + eventShape ),
                                             ( 'CaloBCIDAverage' , 'StoreGateSvc+CaloBCIDAverage' ),
                                             ( 'SG::AuxElement' , 'StoreGateSvc+EventInfo.averageInteractionsPerCrossing' )}

    cellMaker = HLTCellMaker(flags, RoIs, algSuffix="HIRoI")
    cellCorrector = _algoHLTCaloCellCorrector(
        name='HLTRoICaloCellCorrector',
        inputEDM=cellMaker.CellsName,
        outputEDM='CorrectedRoICaloCells',
        eventShape=eventShape)

    topoClusterMaker = _algoHLTTopoCluster(flags, inputEDM = cellCorrector.OutputCellKey, algSuffix="HIRoI")
    RecoSequence = parOR("HIRoITopoClusterRecoSequence", [HLTRoITopoRecoSequenceVDV, cellMaker, cellCorrector, topoClusterMaker])
    return (RecoSequence, topoClusterMaker.CaloClusters)


def HLTLCTopoRecoSequence(flags, RoIs='InViewRoIs'):
    cellMaker = HLTCellMaker(flags, RoIs, outputName="CaloCellsLC", algSuffix="LC")
    cellMaker.TileCellsInROI = True
    topoClusterMaker = _algoHLTTopoClusterLC(flags, inputEDM = cellMaker.CellsName, algSuffix="LC")
    RecoSequence = parOR("TopoClusterRecoSequenceLC",[cellMaker,topoClusterMaker])
    return (RecoSequence, topoClusterMaker.CaloClusters)

