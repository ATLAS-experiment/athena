#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from enum import Enum
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence, SelectionCA, InEventRecoCA
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from ..CommonSequences.FullScanDefs import trkFSRoI, fs_towers
from ..Config.MenuComponents import parOR
from .JetRecoCommon import jetDefToString, jetCalibFromJetDef
from TrigEDMConfig.TriggerEDM import recordable

# Hypo tool generators
from TrigHLTJetHypo.TrigJetHypoToolConfig import trigJetHypoToolFromDict
from .JetPresel import caloPreselJetHypoToolFromDict, roiPreselJetHypoToolFromDict
from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg, jetmetTopoClusteringCfg_LC, HICaloTowerCfg
from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from TrigGenericAlgs.TrigGenericAlgsConfig import TrigEventInfoRecorderAlgCfg

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

###############################################################################################
### --- Input Makers ----

# For step 1, starting from the basic calo reco and topoclustering
# Used for calo-only chains and preselection for tracking
def getCaloInputMaker():
    from TrigT2CaloCommon.CaloDef import clusterFSInputMaker
    InputMakerAlg = clusterFSInputMaker()
    return InputMakerAlg

# For later steps, where calo reco should not be run
# The same instance of an algorithm cannot be run in different steps
# Used for chains that use tracking
def getTrackingInputMaker(flags : AthConfigFlags, trkopt : str):
    if trkopt=="ftf":
        log.debug( "jet FS tracking: useDynamicRoiZWidth: %s", flags.Trigger.InDetTracking.fullScan.useDynamicRoiZWidth )
        
        roiUpdater = None
        if flags.Trigger.InDetTracking.fullScan.useDynamicRoiZWidth:
            roiUpdater = CompFactory.RoiUpdaterTool( useBeamSpot=True )

            log.info( roiUpdater )

            InputMakerAlg = CompFactory.InputMakerForRoI(
                "IM_Jet_TrackingStep",
                mergeUsingFeature = False,
                RoITool = CompFactory.ViewCreatorFSROITool(
                    name="RoiTool_FS",
                    RoiUpdater=roiUpdater,
                    RoisWriteHandleKey=recordable( flags.Trigger.InDetTracking.fullScan.roi )
                ),
                RoIs = trkFSRoI,
            )
        else: 
            InputMakerAlg = CompFactory.InputMakerForRoI(
                "IM_Jet_TrackingStep",
                mergeUsingFeature = False,
                RoITool = CompFactory.ViewCreatorInitialROITool(),
                RoIs = trkFSRoI,
            )

    elif trkopt=="roiftf":
        InputMakerAlg = CompFactory.EventViewCreatorAlgorithm(
            "IMJetRoIFTF",
            mergeUsingFeature = False,
            RoITool = CompFactory.ViewCreatorJetSuperROITool(
                'ViewCreatorJetSuperRoI',
                RoisWriteHandleKey  = recordable( flags.Trigger.InDetTracking.jetSuper.roi ),
                RoIEtaWidth = flags.Trigger.InDetTracking.jetSuper.etaHalfWidth,
                RoIPhiWidth = flags.Trigger.InDetTracking.jetSuper.phiHalfWidth,
                RoIZWidth   = flags.Trigger.InDetTracking.jetSuper.zedHalfWidth,
            ),
            Views = "JetSuperRoIViews",
            InViewRoIs = "InViewRoIs",
            RequireParentView = False,
            ViewFallThrough = True,
        )
    else:
        raise RuntimeError(f"Unrecognised trkopt '{trkopt}' provided, choices are ['ftf','roiftf']")
    return InputMakerAlg

###############################################################################################
### --- Menu Sequence helpers ---

# Functions defining the MenuSequence that will be placed into ChainSteps
# Generate a menu sequence given a set of jet sequences to schedule.
# The hypo may be set up as a preselection hypo, in which case it will
# record a single DecisionObject, instead of one per jet.
# A hypo may alternatively be configured to passThrough, such that
# the hypo will not retrieve any jets and simply pass.
#
# In these functions, we:
#   - First generate the data dependencies for the full jet reco sequence.
#     These come in the form of a dict of JetDefinitions, which are
#     used to define inter-step data dependencies (cluster collections etc).
#     The JetDef is accompanied by a final jet collection name, which is
#     filtered into a vew container to accelerate the hypo
#   - Then pass the JetDefinitions into the configurator functions,
#     which return ComponentAccumulator.
#   - When ChainSteps are created by JetChainConfiguration.getStep,
#     the MenuSequence generators will be wrapped in a deferred call
#     so that when constructing the full HLT menu, we don't regenerate
#     identical sequence configurations repeatedly.

class JetHypoAlgType(Enum):
    STANDARD = 0
    CALOPRESEL = 1
    ROIPRESEL = 2
    PASSTHROUGH = 3

def jetSelectionCfg(flags, jetDefStr, jetsIn, hypoType=JetHypoAlgType.STANDARD):
    """constructs CA with hypo alg given arguments """
    if hypoType==JetHypoAlgType.PASSTHROUGH:
        hyponame = f"TrigStreamerHypoAlg_{jetDefStr}_passthrough"
        hypo = CompFactory.TrigStreamerHypoAlg(hyponame)
    else:
        assert jetsIn is not None
        if hypoType==JetHypoAlgType.CALOPRESEL:
            hyponame = f"TrigJetHypoAlg_{jetDefStr}_calopresel"
            hypo = CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn, DoPresel=True)
        elif hypoType==JetHypoAlgType.ROIPRESEL:
            hyponame = f"TrigJetHypoAlg_{jetDefStr}_roipresel"
            hypo = CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn, DoPresel=True)
        else:
            hyponame = f"TrigJetHypoAlg_{jetDefStr}"
            hypo = CompFactory.TrigJetHypoAlg(hyponame, Jets=jetsIn)
    ca = ComponentAccumulator()
    ca.addEventAlgo(hypo)
    return ca

def selName(recoSequenceName, hypoType=JetHypoAlgType.STANDARD):
    """Construct selection (the name passed to SelectionCA) given reco sequence and hypo type"""
    selname = recoSequenceName.replace('RecoSequence','MenuSequence')
    if hypoType==JetHypoAlgType.PASSTHROUGH:
        selname += "_passthrough"
    else:
        if hypoType==JetHypoAlgType.CALOPRESEL:
            selname += "_calopresel"
        elif hypoType==JetHypoAlgType.ROIPRESEL:
            selname += "_roipresel"
    return selname


def hypoToolGenerator(hypoType):
    """returns function (that in turn returns hypo tool) for menu sequence"""
    def trigStreamerHypoTool(flags, chainDict):
        return CompFactory.TrigStreamerHypoTool(chainDict["chainName"])
    return {
        JetHypoAlgType.STANDARD:    trigJetHypoToolFromDict,
        JetHypoAlgType.PASSTHROUGH: trigStreamerHypoTool,
        JetHypoAlgType.CALOPRESEL:  caloPreselJetHypoToolFromDict,
        JetHypoAlgType.ROIPRESEL:   roiPreselJetHypoToolFromDict,
    }[hypoType]
    


###############################################################################################
### --- Menu Sequence getters ---

# For the preselection step before running tracking (step 1)
# We set RoIs='' (recognised as seedless) instead of caloFSRoI (output of caloInputMater()) to
# cut data dependency to InputMaker and allow full scan CaloCell+Clustering to be
# shared with EGamma (ATR-24722)
@AccumulatorCache
def jetCaloPreselMenuSequenceGenCfg(flags, **jetDefDict):
    jetsOut, jetDef = jetDefDict['final']
    jetDefStr = jetDefToString(jetDef)
    reco = InEventRecoCA(f"jetSeqCaloPresel_{jetDefStr}_RecoSequence", inputMaker=getCaloInputMaker())

    if 'LC' in jetDef.inputdef.label:
        reco.mergeReco(jetmetTopoClusteringCfg_LC(flags, RoIs=''))
    else:
        reco.mergeReco(jetmetTopoClusteringCfg(flags, RoIs=''))
    
    from .JetRecoSequencesConfig import JetRecoCfg
    jetreco = JetRecoCfg(flags, **jetDefDict)
    reco.mergeReco(jetreco)

    log.debug("Generating jet preselection menu sequence for reco %s",jetDef.fullname())
    selAcc = SelectionCA(selName(reco.name, hypoType=JetHypoAlgType.CALOPRESEL))
    selAcc.mergeReco(reco)
    selAcc.mergeHypo(jetSelectionCfg(flags, jetDefStr=jetDefStr, jetsIn=jetsOut, hypoType=JetHypoAlgType.CALOPRESEL))

    return MenuSequence(flags, selAcc, HypoToolGen=hypoToolGenerator(hypoType=JetHypoAlgType.CALOPRESEL))

# A null preselection, which will only run the cluster making (step 1)
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
@AccumulatorCache
def jetCaloRecoMenuSequenceGenCfg(flags, clusterCalib):
    reco = InEventRecoCA(f"jetSeqCaloReco_{clusterCalib}_RecoSequence", inputMaker=getCaloInputMaker())

    if clusterCalib=='lcw':
        reco.mergeReco(jetmetTopoClusteringCfg_LC(flags, RoIs=''))
    else:
        reco.mergeReco(jetmetTopoClusteringCfg(flags, RoIs=''))

    selAcc = SelectionCA(selName(reco.name, hypoType=JetHypoAlgType.PASSTHROUGH))
    selAcc.mergeReco(reco)
    selAcc.mergeHypo(jetSelectionCfg(flags, jetDefStr="caloreco", jetsIn=None, hypoType=JetHypoAlgType.PASSTHROUGH))

    return MenuSequence(flags, selAcc, HypoToolGen=hypoToolGenerator(hypoType=JetHypoAlgType.PASSTHROUGH))


# A full hypo selecting only on calo jets (step 1)
# Passing isPerf = True disables the hypo
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
@AccumulatorCache
def jetCaloHypoMenuSequenceGenCfg(flags, isPerf, **jetDefDict):
    jetsOut, jetDef = jetDefDict['final']
    jetDefStr = jetDefToString(jetDef)
    reco = InEventRecoCA(f"jetSeqCaloHypo_{jetDefStr}{'_perf' if isPerf else ''}_RecoSequence", inputMaker=getCaloInputMaker())

    if 'LC' in jetDef.inputdef.label:
        reco.mergeReco(jetmetTopoClusteringCfg_LC(flags, RoIs=''))
    else:
        reco.mergeReco(jetmetTopoClusteringCfg(flags, RoIs=''))

    from .JetRecoSequencesConfig import JetRecoCfg
    jetreco = JetRecoCfg(flags, **jetDefDict)
    reco.mergeReco(jetreco)
    log.debug("Generating jet calo hypo menu sequence for reco %s",jetDef.fullname())

    hypoType = JetHypoAlgType.PASSTHROUGH if isPerf else JetHypoAlgType.STANDARD
    selAcc = SelectionCA(selName(reco.name, hypoType=hypoType))
    selAcc.mergeReco(reco)
    selAcc.mergeHypo(jetSelectionCfg(flags, jetDefStr=jetDefStr, jetsIn=jetsOut, hypoType=hypoType))

    return MenuSequence(flags, selAcc, HypoToolGen=hypoToolGenerator(hypoType))


# A full hypo selecting only on heavy ion calo jets (step 1)
# Passing isPerf = True disables the hypo
# We set RoIs='' for same reason as described for jetCaloPreselMenuSequence
@AccumulatorCache
def jetHICaloHypoMenuSequenceGenCfg(flags, isPerf, **jetRecoDict):
    reco = InEventRecoCA(f"jetSeqHICaloHypo_{jetRecoDict['jetDefStr']}{'_perf' if isPerf else ''}_RecoSequence", inputMaker=getCaloInputMaker())

    reco.mergeReco( HICaloTowerCfg(flags) )

    from .JetHIConfig import jetHIRecoSequenceCA
    jetreco, jetsOut, jetDef = jetHIRecoSequenceCA(flags, clustersKey="HLT_HICaloClustersFS",towerKey = fs_towers, **jetRecoDict)
    reco.mergeReco(jetreco)        
    log.debug("Generating jet HI calo hypo menu sequence for reco %s",jetDef.fullname())
    hypoType = JetHypoAlgType.PASSTHROUGH if isPerf else JetHypoAlgType.STANDARD
    selAcc = SelectionCA(selName(reco.name, hypoType=hypoType))
    selAcc.mergeReco(reco)
    selAcc.mergeHypo(jetSelectionCfg(flags, jetDefStr=jetRecoDict['jetDefStr'], jetsIn=jetsOut, hypoType=hypoType))

    return MenuSequence(flags, selAcc, HypoToolGen=hypoToolGenerator(hypoType))


# A full hypo selecting on jets with FS track reco (step 2)
# To combine either with a presel or a passthrough sequence
@AccumulatorCache
def jetFSTrackingHypoMenuSequenceGenCfg(flags, isPerf, **jetDefDict):
    jetsOut, jetDef = jetDefDict['final']
    jetDefStr = jetDefToString(jetDef)
    trkopt = jetDef.context
    reco = InEventRecoCA(f"jetFSTrackingHypo_{jetDefStr}{'_perf' if isPerf else ''}_RecoSequence", inputMaker=getTrackingInputMaker(flags,trkopt))

    assert trkopt != "notrk"
    from .JetTrackingConfig import JetFSTrackingCfg
    trk_acc = JetFSTrackingCfg(flags, trkopt, trkFSRoI)
    reco.mergeReco(trk_acc)

    from .JetRecoSequencesConfig import JetRecoCfg
    jetreco = JetRecoCfg(flags, **jetDefDict)
    reco.mergeReco(jetreco)
    log.debug("Generating jet tracking hypo menu sequence for reco %s",jetDef.fullname())

    if 'PFlow' in jetDef.basename and jetDef.basename.startswith('AntiKt4') and 'sub' in jetCalibFromJetDef(jetDef):
        pvKey = flags.Trigger.InDetTracking.fullScan.vertex_jet
        trig_evt_info_key = recordable("HLT_TCEventInfo_jet")

        # Can encapsulate in another CA if necessary but only this instance
        # currently needed
        reco.mergeReco(
            TrigEventInfoRecorderAlgCfg(
                flags,
                name="TrigEventInfoRecorderAlg_jet",
                decoratePFlowInfo=True,
                decorateEMTopoInfo=False,
                trigEventInfoKey=trig_evt_info_key, primaryVertexInputName=pvKey,
                RhoKey_EMTopo='HLT_Kt4EMTopoEventShape', RhoKey_PFlow='HLT_Kt4EMPFlowEventShape'
            )
        )

    hypoType = JetHypoAlgType.PASSTHROUGH if isPerf else JetHypoAlgType.STANDARD
    selAcc = SelectionCA(selName(reco.name, hypoType=hypoType))
    selAcc.mergeReco(reco)
    selAcc.mergeHypo(jetSelectionCfg(flags, jetDefStr=jetDefStr, jetsIn=jetsOut, hypoType=hypoType))

    return MenuSequence(flags, selAcc, HypoToolGen=hypoToolGenerator(hypoType))


# A full hypo selecting on jets with RoI track reco (step 2)
# Needs to be preceded by a presel sequence, and be provided
# with the input jets from which to define RoIs
# Presel jets to be reused, which makes ghost association impossible
# Substitute DR association decorator
@AccumulatorCache
def jetRoITrackJetTagSelCfg(flags, preselJetDef, isPresel=True):
    # Seems odd, but we have to combine event and view execution here
    # where InViewRecoCA will do all in view
    jetDefStr = jetDefToString(preselJetDef)+'_roiftf'
    trkopt = 'roiftf'
    reco = InEventRecoCA(
        f"jetRoITrackJetTagHypo_{jetDefStr}_RecoSequence",
        inputMaker=getTrackingInputMaker(flags,trkopt)
    )

    # Add to top-level serial sequence after IM
    from TrigGenericAlgs.TrigGenericAlgsConfig import ROBPrefetchingAlgCfg_Si
    reco.mergeReco(ROBPrefetchingAlgCfg_Si(flags, nameSuffix=reco.inputMaker().name))

    # Add to top-level serial sequence to ensure it is ready for in-view reco
    from .JetRecoSequencesConfig import (
        FastFtaggedJetCopyAlgCfg, JetRoITrackJetTagSequenceCfg, JetViewAlgCfg, formatFilteredJetsName, JET_DEFAULT_VIEW_PT_MIN_GEV
    )
    ftagjet_acc, ftaggedJetDef = FastFtaggedJetCopyAlgCfg(flags,preselJetDef)
    ftaggedJetName = recordable(ftaggedJetDef.fullname())
    reco.mergeReco(ftagjet_acc)

    track_acc = JetRoITrackJetTagSequenceCfg(
        flags,
        ftaggedJetName,
        trkopt,
        RoIs=reco.inputMaker().InViewRoIs)
    # Explicitly add the sequence here that is to run in the super-RoI view
    seqname = f"JetRoITrackJetTag_{trkopt}_RecoSequence"
    reco.addSequence(parOR(seqname),primary=True)
    reco.merge(track_acc,seqname)
    reco.inputMaker().ViewNodeName = seqname

    # Run the JetViewAlg sequence to filter out low pT jets
    # Have to run it outside of JetRoITrackJetTagSequence (which runs in EventView), so that hypo recognises the filtered jets.
    jetview_Acc = JetViewAlgCfg(flags,jetDef=ftaggedJetDef)
    filtered_jetsIn = formatFilteredJetsName(ftaggedJetName,JET_DEFAULT_VIEW_PT_MIN_GEV)
    reco.merge(jetview_Acc)

    # Needs track-to-jet association here, maybe with dR decorator
    hypoType = JetHypoAlgType.ROIPRESEL if isPresel else JetHypoAlgType.STANDARD
    selAcc = SelectionCA(selName(reco.name, hypoType=hypoType))
    selAcc.mergeReco(reco)
    selAcc.mergeHypo(jetSelectionCfg(flags, jetDefStr=jetDefStr, jetsIn=filtered_jetsIn, hypoType=hypoType))
    return selAcc, hypoType

def jetRoITrackJetTagHypoMenuSequenceGenCfg(flags, jetDef, isPresel=True):
    selAcc, hypoType = jetRoITrackJetTagSelCfg(flags, jetDef, isPresel)
    return MenuSequence(flags, selAcc, HypoToolGen=hypoToolGenerator(hypoType))

