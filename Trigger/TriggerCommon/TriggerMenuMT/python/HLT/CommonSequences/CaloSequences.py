#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

class CaloMenuDefs(object):
      """Static Class to collect all string manipulations in Calo sequences """
      from TrigEDMConfig.TriggerEDM import recordable
      L2CaloClusters= recordable("HLT_FastCaloEMClusters")


#
# central or forward fast calo sequence 
#

@AccumulatorCache
def fastCaloMenuSequenceCfg(flags, name, doRinger=True, is_probe_leg=False):
    """ Creates Egamma Fast Calo  MENU sequence
    The Hypo name changes depending on name, so for different implementations (Electron, Gamma,....)
    """

    from TrigT2CaloCommon.CaloDef import fastCaloVDVCfg
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    from TrigT2CaloCommon.CaloDef import fastCaloRecoSequenceCfg
    nameselAcc = "fastCaloSequence"+name
    output = "HLT_FastCaloEMClusters"
    selAcc = SelectionCA(nameselAcc,isProbe=is_probe_leg)
    InViewRoIs="EMCaloRoIs"
    reco = InViewRecoCA("EMCalo",InViewRoIs=InViewRoIs,isProbe=is_probe_leg)
    reco.mergeReco(fastCaloVDVCfg(InViewRoIs=InViewRoIs))
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix=InViewRoIs+'_probe' if is_probe_leg else InViewRoIs)
    reco.mergeReco(fastCaloRecoSequenceCfg(flags, inputEDM=InViewRoIs,ClustersName=output))
    selAcc.mergeReco(reco, robPrefetchCA=robPrefetchAlg)

    # hypo # The Alg will ALWAYS configure photons and electrons for ringer
    # The tool is what will use that or not
    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import createTrigEgammaFastCaloHypoAlg

    theFastCaloHypo = createTrigEgammaFastCaloHypoAlg(flags, name+"FastCaloHypo", sequenceOut=output)
    selAcc.addHypoAlgo(theFastCaloHypo)

    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import TrigEgammaFastCaloHypoToolFromDict
    return MenuSequenceCA(flags,selAcc,HypoToolGen=TrigEgammaFastCaloHypoToolFromDict,isProbe=is_probe_leg)


def fastCaloMenuSequence(flags, name, doRinger=True, is_probe_leg=False):
     if isComponentAccumulatorCfg():
        return fastCaloMenuSequenceCfg(flags,name=name,doRinger=doRinger,is_probe_leg=is_probe_leg)
     else: 
        return menuSequenceCAToGlobalWrapper(fastCaloMenuSequenceCfg,flags,name=name,doRinger=doRinger,is_probe_leg=is_probe_leg)
