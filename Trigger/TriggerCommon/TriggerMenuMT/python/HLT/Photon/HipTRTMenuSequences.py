#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

# menu components
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InViewRecoCA, menuSequenceCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from TrigTRTHighTHitCounter.TrigTRTHTHCounterConfig import TrigTRTHTHCounterFex
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator


def TRTHitGeneratorSequenceCfg(flags, is_probe_leg = False):

    recAcc = ComponentAccumulator()

    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys()

    """ hipTRT step ....."""
    inViewRoIs = "TRTHitGenerator"
    # EVCreator:
    roiTool = CompFactory.ViewCreatorCentredOnClusterROITool()
    roiTool.AllowMultipleClusters = False 
    roiTool.RoisWriteHandleKey = "HLT_Roi_TRTHit"
    roiTool.RoIEtaWidth = 0.10
    roiTool.RoIPhiWidth = 0.10
    viewName = "TRTHitGenerator"

    hipTRTReco = InViewRecoCA(viewName,
                              RoITool=roiTool, # view maker args
                              RequireParentView=True,
                              InViewRoIs=inViewRoIs,
                              isProbe=is_probe_leg)
   
    # view data verifier
    ViewVerify = CompFactory.AthViews.ViewDataVerifier("TRTHitGeneratorViewDataVerifier")
    ViewVerify.DataObjects = {('TrigRoiDescriptorCollection' , 'StoreGateSvc+TRTHitGenerator'),
                             }
    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    flagsWithTrk = getFlagsForActiveConfig(flags, 'photon', log)

    from TrigInDetConfig.InDetTrigSequence import InDetTrigSequence

    seq = InDetTrigSequence(flagsWithTrk, flagsWithTrk.Tracking.ActiveConfig.input_name, 
                            rois=inViewRoIs, inView=ViewVerify.getName())

    recAcc.merge(seq.dataPreparationTRT())
    
    recAcc.addEventAlgo(ViewVerify)
    trtHTHFex = TrigTRTHTHCounterFex(flags, name="TrigTRTH_fex",
                                     RoIs = inViewRoIs,
                                     containerName = "TRT_TrigDriftCircles",
                                     RNNOutputName = TrigEgammaKeys.TrigTRTHTCountsContainer)

    recAcc.addEventAlgo(trtHTHFex)

    hipTRTReco.mergeReco(recAcc)

    selAcc=SelectionCA("trtHitCounter", isProbe=is_probe_leg)   
    selAcc.mergeReco(hipTRTReco)
    trtHTHhypo = CompFactory.TrigTRTHTHhypoAlg(name="TrigTRTHTHhypo", RNNOutputName=TrigEgammaKeys.TrigTRTHTCountsContainer)
    selAcc.addHypoAlgo(trtHTHhypo)
    from TrigTRTHighTHitCounter.TrigTRTHTHhypoTool import TrigTRTHTHhypoToolFromDict
    return MenuSequenceCA(flags,selAcc, HypoToolGen=TrigTRTHTHhypoToolFromDict, isProbe=is_probe_leg)

def hipTRTMenuSequence(flags, is_probe_leg=False):

    if isComponentAccumulatorCfg():
        return TRTHitGeneratorSequenceCfg(flags, is_probe_leg=is_probe_leg)
    else: 
        return menuSequenceCAToGlobalWrapper(TRTHitGeneratorSequenceCfg, flags, is_probe_leg=is_probe_leg)


