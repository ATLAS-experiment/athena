#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA
from TriggerMenuMT.HLT.CommonSequences.RejectSequences import RejectSequence
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

class CaloMenuDefs(object):
      """Static Class to collect all string manipulations in Calo sequences """
      from TrigEDMConfig.TriggerEDM import recordable
      L2CaloClusters= recordable("HLT_FastCaloEMClusters")


#
# central or forward fast calo sequence 
#

@AccumulatorCache
def fastCaloSequenceGenCfg(flags, name, doRinger=True, is_probe_leg=False, doRingerCalib=False):
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
    InViewRecoName="EMCalo"
    reco = InViewRecoCA(InViewRecoName,InViewRoIs=InViewRoIs,isProbe=is_probe_leg)
    reco.mergeReco(fastCaloVDVCfg(flags,InViewRoIs=InViewRoIs))
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix=InViewRoIs+'_probe' if is_probe_leg else InViewRoIs)
    reco.mergeReco(fastCaloRecoSequenceCfg(flags, inputEDM=InViewRoIs,ClustersName=output, doRingerCalib=doRingerCalib))
    selAcc.mergeReco(reco, robPrefetchCA=robPrefetchAlg)

    # hypo # The Alg will ALWAYS configure photons and electrons for ringer
    # The tool is what will use that or not
    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import createTrigEgammaFastCaloHypoAlg

    theFastCaloHypo = createTrigEgammaFastCaloHypoAlg(flags, name+"FastCaloHypo", sequenceOut=output)
    selAcc.addHypoAlgo(theFastCaloHypo)
    from TrigEgammaHypo.TrigEgammaFastCaloHypoTool import TrigEgammaFastCaloHypoToolFromDict
    return MenuSequence(flags,selAcc,HypoToolGen=TrigEgammaFastCaloHypoToolFromDict)

def fastCaloCalibSequenceGenCfg(flags, name, doRinger=True, is_probe_leg=False):
    return fastCaloSequenceGenCfg(flags,name,doRinger=doRinger,is_probe_leg=is_probe_leg, doRingerCalib=True)

@AccumulatorCache
def fastCaloPhotonPointSequenceGenCfg(flags, name, doRinger=True, is_probe_leg=False):
    """ Creates Egamma Fast Calo  MENU sequence
    The Hypo name changes depending on name, so for different implementations (Electron, Gamma,....)
    """

    from TrigT2CaloCommon.CaloDef import fastCaloVDVCfg
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Calo
    from TrigT2CaloCommon.CaloDef import fastCaloPhotonPointRecoSequenceCfg
    nameselAcc = "fastCaloSequence"+name
    output = "HLT_FastCaloEMClusters"
    selAcc = SelectionCA(nameselAcc,isProbe=is_probe_leg)
    InViewRoIs="EMCaloRoIsPhotonPoint"
    InViewRecoName="EMCaloPhotonPoint"
    reco = InViewRecoCA(InViewRecoName,InViewRoIs=InViewRoIs,isProbe=is_probe_leg)
    reco.mergeReco(fastCaloVDVCfg(flags,InViewRoIs=InViewRoIs))
    robPrefetchAlg = ROBPrefetchingAlgCfg_Calo( flags, nameSuffix=InViewRoIs+'_probe' if is_probe_leg else InViewRoIs)
    reco.mergeReco(fastCaloPhotonPointRecoSequenceCfg(flags,inputEDM=InViewRoIs,ClustersName=output))
    selAcc.mergeReco(reco, robPrefetchCA=robPrefetchAlg)

    # hypo # The Alg will ALWAYS configure photons and electrons for ringer
    # The tool is what will use that or not
    HypoName = "PhotonPoint"
    msca = RejectSequence(flags, HypoName, selAcc)
    return msca
