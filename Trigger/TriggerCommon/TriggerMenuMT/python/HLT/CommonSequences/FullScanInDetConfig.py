# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg, trigInDetLRTCfg
from InDetConfig.InDetPriVxFinderConfig import InDetTrigPriVxFinderCfg

from AthenaCommon.Logging import logging
from AthenaCommon.CFElements import parOR

logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

from .FullScanDefs import trkFSRoI

@AccumulatorCache
def commonInDetFullScanCfg(flags: AthConfigFlags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    seqname='TrigInDetFullScan'
    acc.addSequence(parOR(seqname),primary=True)

    flagsWithTrk = getFlagsForActiveConfig(flags, 'fullScan', log)
    acc.merge(
        trigInDetFastTrackingCfg(
            flagsWithTrk,
            trkFSRoI,
            signatureName='fullScan',
            in_view=False
        ),
        seqname
    )

    vxkwargs = {
        "inputTracks": flagsWithTrk.Tracking.ActiveConfig.tracks_FTF,
        "outputVtx": flagsWithTrk.Tracking.ActiveConfig.vertex_jet,
    }
    if flags.Trigger.FSTrk.doJetRestrictedVertexSort:
        from TrkConfig.TrkVertexToolsConfig import JetRestrictedSumPt2VertexCollectionSortingToolCfg
        from TrkConfig.TrkVertexWeightCalculatorsConfig import JetRestrictedSumPtVertexWeightCalculatorCfg
        jetcalccfg = JetRestrictedSumPtVertexWeightCalculatorCfg(
            flags,
            JetContainer='HLT_AntiKt4EMTopoJets_subjesIS',
            TrackParticleLocation=flagsWithTrk.Tracking.ActiveConfig.tracks_FTF,
        )
        jetcalc = jetcalccfg.popPrivateTools()
        acc.merge(jetcalccfg)
        vxsortcfg = JetRestrictedSumPt2VertexCollectionSortingToolCfg(
            flags,
            VertexWeightCalculator=jetcalc
        )
        vxsort = vxsortcfg.popPrivateTools()
        acc.merge(vxsortcfg)
        vxkwargs["VertexCollectionSortingTool"] = vxsort

    acc.merge(
        InDetTrigPriVxFinderCfg(
            flagsWithTrk,
            **vxkwargs,
        ),
        seqname
    )

    return acc


def commonInDetLRTCfg(flags    : AthConfigFlags, 
                      flagsLRT : AthConfigFlags, 
                      rois     : str = trkFSRoI) -> ComponentAccumulator:
    
    acc = ComponentAccumulator()
    seqname = 'TrigInDetLRT_'+flagsLRT.Tracking.ActiveConfig.name
    acc.addSequence(parOR(seqname),primary=True)

    acc.merge(
        trigInDetLRTCfg(
            flagsLRT,
            flags.Tracking.ActiveConfig.trkTracks_FTF,
            rois,
            in_view=False
        ),
        seqname
    )

    return acc
