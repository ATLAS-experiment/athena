#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

class CaloMenuDefs(object):
      """Static Class to collect all string manipulations in Calo sequences """
      from TrigEDMConfig.TriggerEDM import recordable
      L2CaloClusters= recordable("HLT_FastCaloEMClusters")


#
# central or forward fast calo sequence 
#

@AccumulatorCache
def fastCaloSequenceGenCfg(flags, name, doRinger=True, is_probe_leg=False):
    """ Creates Egamma Fast Calo  MENU sequence
    The Hypo name changes depending on name, so for different implementations (Electron, Gamma,....)
    """

    from TrigT2CaloCommon.CaloDef import fastCaloVDVCfg
    from TrigT2CaloCommon.CaloDef import fastCaloRecoSequenceCfg
    nameselAcc = "fastCaloSequence"+name
    output = "HLT_FastCaloEMClusters"
    selAcc = SelectionCA(nameselAcc,isProbe=is_probe_leg)
    InViewRoIs="EMCaloRoIs"
    reco = InViewRecoCA("EMCalo",InViewRoIs=InViewRoIs,isProbe=is_probe_leg)
    reco.mergeReco(fastCaloVDVCfg(flags,InViewRoIs=InViewRoIs))
    reco.mergeReco(fastCaloRecoSequenceCfg(flags, inputEDM=InViewRoIs,ClustersName=output,))
    selAcc.mergeReco(reco)

    # hypo # The Alg will ALWAYS configure photons and electrons for ringer
    # The tool is what will use that or not
    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import createTrigEgammaFastCaloHypoAlg

    theFastCaloHypo = createTrigEgammaFastCaloHypoAlg(flags, name+"FastCaloHypo", sequenceOut=output)
    selAcc.addHypoAlgo(theFastCaloHypo)

    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import TrigEgammaFastCaloHypoToolFromDict
    return MenuSequence(flags,selAcc,HypoToolGen=TrigEgammaFastCaloHypoToolFromDict)



