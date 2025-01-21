# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.CommonSequences.FullScanDefs import caloFSRoI
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from TrigT2CaloCommon.TrigCaloDataAccessConfig import trigCaloDataAccessSvcCfg, CaloDataAccessSvcDependencies


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

def fastCaloVDVCfg(flags,name="fastCaloVDV",InViewRoIs="EMCaloRoIs") :
    reco = ComponentAccumulator()
    fastCaloVDV = CompFactory.AthViews.ViewDataVerifier(name)
    fastCaloVDV.DataObjects = {( 'CaloBCIDAverage' , 'StoreGateSvc+CaloBCIDAverage' ),
                               ( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s'%InViewRoIs )}
    if ( not flags.Input.isMC ):
       fastCaloVDV.DataObjects.add(('LArDeadOTXFromSC' , 'StoreGateSvc+DeadOTXFromSC' ))
    reco.addEventAlgo(fastCaloVDV)
    return reco

##################################
# cluster maker functions
###################################

def clusterFSInputMaker( ):
  """Creates the inputMaker for FS in menu"""
  RoIs = caloFSRoI
  InputMakerAlg = CompFactory.InputMakerForRoI("IMclusterFS", RoIsLink="initialRoI")
  InputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()
  InputMakerAlg.RoIs=RoIs
  return InputMakerAlg
