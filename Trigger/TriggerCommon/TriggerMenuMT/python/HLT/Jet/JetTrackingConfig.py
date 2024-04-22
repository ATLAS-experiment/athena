
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from JetRecTools import JetRecToolsConfig
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from TrigInDetConfig.utils import getFlagsForActiveConfig
from TrigInDetConfig.TrigInDetConfig import trigInDetFastTrackingCfg, trigInDetPrecisionTrackingCfg
from InDetConfig.InDetPriVxFinderConfig import InDetTrigPriVxFinderCfg
from InDetUsedInVertexFitTrackDecorator.UsedInVertexFitTrackDecoratorCfg import getUsedInVertexFitTrackDecoratorAlg

from AthenaConfiguration.AccumulatorCache import AccumulatorCache

from ..Config.MenuComponents import parOR
from ..CommonSequences.FullScanInDetConfig import commonInDetFullScanCfg

@AccumulatorCache
def JetFSTrackingCfg(flags, trkopt, RoIs):
    """ Create the tracking CA and return it as well as the output name dictionary """
    seqname = f"JetFSTracking_{trkopt}_RecoSequence"
    acc = ComponentAccumulator()
    acc.addSequence(parOR(seqname),primary=True)

    acc.merge(commonInDetFullScanCfg(flags),seqname)

    # get the jetContext for trkopt 
    jetContext = flags.Jet.Context[trkopt]

    acc.addEventAlgo(
        getUsedInVertexFitTrackDecoratorAlg(
            trackCont = jetContext["Tracks"],
            vtxCont   = jetContext["Vertices"]
        ),
        seqname
    )

    # Create the TTVA
    acc.addEventAlgo(
        JetRecToolsConfig.getJetTrackVtxAlg(
            jetContext, algname="jetalg_TrackPrep"+trkopt,
            # # parameters for the CP::TrackVertexAssociationTool (or the TrackVertexAssociationTool.getTTVAToolForReco function) :
            #WorkingPoint = "Nonprompt_All_MaxWeight", # this is the new default in offline (see also CHS configuration in StandardJetConstits.py)
            WorkingPoint = "Custom",
            d0_cut       = 2.0, 
            dzSinTheta_cut = 2.0, 
            doPVPriority = flags.Trigger.InDetTracking.fullScan.adaptiveVertex,
        ),
        seqname
    )
    
    # Add the pseudo-jet creator
    acc.addEventAlgo(
        CompFactory.PseudoJetAlgorithm(
            f"pjgalg_{jetContext['GhostTracksLabel']}",
            InputContainer=jetContext["Tracks"],
            OutputContainer=jetContext["GhostTracks"],
            Label=jetContext["GhostTracksLabel"],
            SkipNegativeEnergy=True,
        ),
        seqname
    )

    return acc

@AccumulatorCache
def JetRoITrackingCfg(flags, jetsIn, trkopt, RoIs):
    """ Create the tracking CA and return it as well as the output name dictionary """

    acc = ComponentAccumulator()

    acc.addEventAlgo(CompFactory.AthViews.ViewDataVerifier(
        name = "VDVInDetFTF_jetSuper",
        DataObjects = {
            ('xAOD::JetContainer' , 'StoreGateSvc+HLT_AntiKt4EMTopoJets_subjesIS_fastftag'),
        }
    ))

    assert trkopt == "roiftf"

    from AthenaCommon.Logging import logging
    log = logging.getLogger(__name__)
    flagsWithTrk = getFlagsForActiveConfig(flags, 'jetSuper', log)

    acc.merge(
        trigInDetFastTrackingCfg(
            flagsWithTrk,
            RoIs,
            signatureName="jetSuper",
            in_view=True
        )
    )

    if flagsWithTrk.Trigger.Jet.doJetSuperPrecisionTracking:
        acc.merge(
            trigInDetPrecisionTrackingCfg(
                flagsWithTrk,
                RoIs,
                signatureName="jetSuper",
                in_view=False
            )
        )

        vertexInputTracks = flagsWithTrk.Tracking.ActiveConfig.tracks_IDTrig

    else:
        vertexInputTracks = flagsWithTrk.Tracking.ActiveConfig.tracks_FTF

    acc.merge(
        InDetTrigPriVxFinderCfg(
            flagsWithTrk,
            inputTracks = vertexInputTracks,
            outputVtx =   flagsWithTrk.Tracking.ActiveConfig.vertex,
        )
    )

    # make sure we output only the key,value related to tracks (otherwise, alg duplication issues)
    jetContext = flags.Jet.Context[trkopt]    
    outmap = { k:jetContext[k] for k in flags.Jet.Context.CommonTrackKeys }
    if flags.Trigger.Jet.doJetSuperPrecisionTracking:
        outmap["Tracks"] = vertexInputTracks

    return acc, outmap

