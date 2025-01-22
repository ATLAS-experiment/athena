#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory

class CaloMenuDefs_FWD(object):
      """Static Class to collect all string manipulations in Calo sequences """
      from TrigEDMConfig.TriggerEDM import recordable
      L2CaloClusters= recordable("HLT_FastCaloEMClusters_FWD")


#
# central or forward fast calo sequence 
#

@AccumulatorCache
def fastCalo_FWDSequenceGenCfg(flags,name,doRinger=True, is_probe_leg=False):
   """ Creates Egamma Fast Calo FWD MENU sequence (Reco and Hypo)
   The Hypo name changes depending on name, so for different implementations (Electron, Gamma,....)
   """

   from TrigT2CaloCommon.CaloDef import fastCaloVDVCfg
   from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
   from TrigT2CaloCommon.CaloDef import fastCaloRecoSequenceCfg
   nameselAcc = "fastCaloFWDSequence"
   output = "HLT_FastCaloEMClusters_FWD"
   CaloMenuDefs_FWD.L2CaloClusters = output
   selAcc = SelectionCA(nameselAcc,isProbe=is_probe_leg)
   InViewRoIs="FSJETMETCaloRoI"
   reco = InViewRecoCA("EMCaloFWD",InViewRoIs=InViewRoIs,isProbe=is_probe_leg)
   reco.mergeReco(fastCaloVDVCfg(flags,InViewRoIs=InViewRoIs))
   robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix=InViewRoIs+'_probe' if is_probe_leg else InViewRoIs)
   reco.mergeReco(fastCaloRecoSequenceCfg(flags, inputEDM=InViewRoIs,doForward=True,ClustersName=output))
   selAcc.mergeReco(reco, robPrefetchCA=robPrefetchAlg)

   # hypo
   from TrigEgammaForwardHypo.TrigEgammaForwardFastCaloHypoTool import TrigEgammaForwardFastCaloHypoToolFromDict
   theFastCaloHypo = CompFactory.TrigEgammaForwardFastCaloHypoAlgMT(name+"EgammaFastCaloFWDHypo")
   selAcc.addHypoAlgo(theFastCaloHypo)

   return MenuSequence(flags,selAcc,HypoToolGen=TrigEgammaForwardFastCaloHypoToolFromDict)

