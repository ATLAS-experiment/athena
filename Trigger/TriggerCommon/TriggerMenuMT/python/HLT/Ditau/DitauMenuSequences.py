#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InViewRecoCA
from DiTauRec.DiTauToolsConfig import (
    SeedJetBuilderCfg, SubjetBuilderCfg, 
    VertexFinderCfg, 
    DiTauTrackFinderCfg, 
    DiTauConstituentFinderCfg, 
    IDVarCalculatorCfg, 
    TVAToolCfg, 
    DiTauIDVarDecoratorCfg,
    DiTauOnnxScoreCalculatorCfg,
)


from TrigEDMConfig.TriggerEDM import recordable

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


def ditauTrackingCfg(flags, inputRoI: str, inputVertex: str, inputJets: str) -> ComponentAccumulator:
    #safety measure to ensure we get the right instance of flags
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    trkflags = getFlagsForActiveConfig(flags, "diTau", log)

    from TrigInDetConfig.InnerTrackingTrigSequence import InnerTrackingTrigSequence
    seq = InnerTrackingTrigSequence.create(trkflags, 
                                           trkflags.Tracking.ActiveConfig.input_name, 
                                           rois   = inputRoI,
                                           inView = "VDVInDetFTF")
    acc = seq.sequence("FastTrackFinder")
    acc.merge(seq.sequenceAfterPattern())

    verifier = CompFactory.AthViews.ViewDataVerifier(name = 'VDVsecondStageDitauTracking',
                                                     DataObjects = {('xAOD::VertexContainer', f'StoreGateSvc+{inputVertex}'),
                                                                    ('xAOD::JetContainer', f'StoreGateSvc+{inputJets}')} )
    acc.addEventAlgo(verifier)

    return acc

def JetTVAAlgCfg(flags, inputTracks, inputVertex, name="HLT_DiTauRec_JetAlgorithm"): # Name changed wrt legacy config DiTauRec_TVATool
    """Configure the JetAlgorithm"""
    acc = ComponentAccumulator()

    tools = [acc.popToolsAndMerge(
        TVAToolCfg(
            flags,
            TrackParticleContainer = inputTracks,
            VertexContainer = inputVertex,
            TrackVertexAssociation = "HLT_JetTrackVtxAssoc_forDiTaus"
        )
    )]
    tools = tools

    acc.addEventAlgo(CompFactory.JetAlgorithm(name, Tools = tools))
    return acc

def ditauRecoCfg(flags, inputJets: str, inputVertex: str, inputFSTracks: str,  inputTracks: str, inputCells: str, inputClusters: str) -> ComponentAccumulator:
    from .DitauConfigFlagsHLT import createDiTauConfigFlags
    flags_ditau = createDiTauConfigFlags()

    acc = ComponentAccumulator()
    acc.merge(JetTVAAlgCfg(
        flags,
        inputTracks=inputFSTracks,
        inputVertex=inputVertex,
    )) 
    tools = [
        acc.popToolsAndMerge(SeedJetBuilderCfg(flags, JetCollection=inputJets)),
        acc.popToolsAndMerge(SubjetBuilderCfg(flags)),
        acc.popToolsAndMerge(VertexFinderCfg(
            flags,
            TrackVertexAssociation = "HLT_JetTrackVtxAssoc_forDiTaus",
            PrimVtxContainerName = inputVertex,
            AssociatedTracks = "GhostTrack_ftf"
        )),
        acc.popToolsAndMerge(DiTauTrackFinderCfg(
            flags, 
            TrackParticleContainer=inputTracks
        )),
        acc.popToolsAndMerge(DiTauConstituentFinderCfg(
            flags,
        )),
        # acc.popToolsAndMerge(CellFinderCfg(
        #     flags,
        #     ClusterContainer = inputClusters,
        #     CellContainer = inputCells                   
        # )),
        acc.popToolsAndMerge(IDVarCalculatorCfg(flags, useCells=False)),
        acc.popToolsAndMerge(DiTauIDVarDecoratorCfg(flags)),
        acc.popToolsAndMerge(DiTauOnnxScoreCalculatorCfg(
            flags, 
            onnxModelPath=f"{flags_ditau.DiTau.CalibFolder}{flags_ditau.DiTau.DiTauIDModel}",
        )),
    ]

    acc.addEventAlgo(CompFactory.DiTauBuilder(
        "HLT_DiTauBuilder",
        DiTauContainer  = recordable("HLT_DiTauJets"),
        Tools           = tools,
        SeedJetName     = inputJets,
        minPt           = flags_ditau.DiTau.JetSeedPt,
        maxEta          = flags_ditau.DiTau.MaxEta,
        Rjet            = flags_ditau.DiTau.Rjet,
        Rsubjet         = flags_ditau.DiTau.Rsubjet,
        Rcore           = flags_ditau.DiTau.Rcore,
    ))
    return acc

def ditauSequenceGenCfg(flags, seq_name, jet_name):
    # input maker and ROI tool
    prmVtxKey = flags.Trigger.InDetTracking.fullScan.vertex
    roiTool = CompFactory.ViewCreatorCentredOnJetWithPVConstraintROITool(
        RoisWriteHandleKey  = recordable( 'HLT_Roi_DiTau' ),
        VertexReadHandleKey = prmVtxKey,
        PrmVtxLink  = prmVtxKey.replace( "HLT_","" ),
        RoIEtaWidth = 1.0,
        RoIPhiWidth = 1.0,
        RoIZWidth   = 7.0,
    )
    ditauAcc = InViewRecoCA("Ditau", RoITool = roiTool,
                                    InViewRoIs = "InViewRoIs",
                                    mergeUsingFeature = True,
                                    RequireParentView = False,
                                    ViewFallThrough = True,
                                    InViewJets = f'{jet_name}_DiTau_jets',
                                    PlaceJetInView = True)
    InputMakerAlg = ditauAcc.inputMaker()
    # Tracking
    trackingAcc = ditauTrackingCfg(
        flags,
        inputRoI=InputMakerAlg.InViewRoIs,
        inputVertex=prmVtxKey,
        inputJets=InputMakerAlg.InViewJets
    )
    ditauAcc.mergeReco(trackingAcc)

    # ditau reconstruction
    inputJets = InputMakerAlg.InViewJets
    inputVertex = prmVtxKey
    inputFSTracks = flags.Trigger.InDetTracking.fullScan.tracks_FTF
    inputIDTracks = flags.Trigger.InDetTracking.diTau.tracks_IDTrig
    inputCells = "CaloCellsFS"
    inputClusters = "HLT_TopoCaloClustersFS"

    recoAcc = ditauRecoCfg(
        flags,
        inputJets=inputJets,
        inputVertex=inputVertex,
        inputFSTracks=inputFSTracks,
        inputTracks=inputIDTracks,
        inputCells=inputCells,
        inputClusters=inputClusters
    )

    ditauAcc.mergeReco(recoAcc)

    ditauAcc.addRecoAlgo(CompFactory.AthViews.ViewDataVerifier(
        name=f'{ditauAcc.name}RecoVDV',
        DataObjects={
            ('CaloCellContainer',                   f'StoreGateSvc+{inputCells}'),
            ('xAOD::CaloClusterContainer',          f'StoreGateSvc+{inputClusters}'),
            ('CaloClusterCellLinkContainer',        f'StoreGateSvc+{inputClusters}_links'),
            ('xAOD::TrackParticleContainer' ,       f'StoreGateSvc+{inputFSTracks}'),
        }
    ))

    selAcc = SelectionCA(f'Trig_DitauReco_{seq_name}')
    selAcc.mergeReco(ditauAcc)
    selAcc.addHypoAlgo(CompFactory.TrigDiTauHypoAlg(
        f'DiTauHypoAlg_{seq_name}',
        DiTauJets_key='HLT_DiTauJets',
    ))

    from TrigDitauHypo.TrigDiTauHypoTool import TrigDiTauHypoToolFromDict
    menuSeq = MenuSequence(flags, selAcc, HypoToolGen=TrigDiTauHypoToolFromDict)

    return menuSeq