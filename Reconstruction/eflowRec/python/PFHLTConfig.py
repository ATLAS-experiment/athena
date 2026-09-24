# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from AthenaCommon.CFElements import parOR
from eflowRec import PFOnlineMon
from eflowRec.PFCfg import getPFTrackClusterMatchingTool
from TrigEDMConfig.TriggerEDM import recordable


#---------------------------------------------------------------------------------#
# Tracking geometry & conditions
def TrackingGeoCfg(inputFlags):
    result = ComponentAccumulator()

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(inputFlags))

    return result

#---------------------------------------------------------------------------------#
# Calo geometry & conditions
def CaloGeoAndNoiseCfg(inputFlags):
    result = ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    from TileGeoModel.TileGMConfig import TileGMCfg

    result.merge(LArGMCfg(inputFlags))
    result.merge(TileGMCfg(inputFlags))

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    # Schedule total noise cond alg
    result.merge(CaloNoiseCondAlgCfg(inputFlags,"totalNoise"))

    return result

def PFTrackExtensionCfg(flags, tracktype, tracksin):
    """ Get the track-to-calo extension after a preselection

    Returns the component accumulator, the preselected track collection and the extension cache
    """
    result = ComponentAccumulator()
    pretracks_name = f"HLTPFPreselTracks_{tracktype}"
    cache_name = f"HLTPFTrackExtensionCache_{tracktype}"

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        PFTrackSelectionToolCfg)
    result.addEventAlgo(CompFactory.PFTrackPreselAlg(
        f"HLTPFTrackPresel_{tracktype}",
        InputTracks=tracksin,
        OutputTracks=pretracks_name,
        TrackSelTool=result.popToolsAndMerge(PFTrackSelectionToolCfg(flags))
    ))


    monTool_trackExtrap = PFOnlineMon.getMonTool_ParticleCaloExtensionTool(flags)
    monTool_trackExtrap.HistPath = 'TrackCaloExtrapolation_general'

    from TrackToCalo.TrackToCaloConfig import HLTPF_ParticleCaloExtensionToolCfg
    result.addEventAlgo(CompFactory.Trk.PreselCaloExtensionBuilderAlg(
        f"HLTPFTrackExtension_{tracktype}",
        ParticleCaloExtensionTool=result.popToolsAndMerge(
            HLTPF_ParticleCaloExtensionToolCfg(flags, MonTool=monTool_trackExtrap)),
        InputTracks=pretracks_name,
        OutputCache=cache_name,
    ))

    return result, pretracks_name, cache_name

def MuonCaloTagCfg(flags, tracktype, tracksin, extcache, cellsin):
    """ Create the muon calo tagging configuration
    
    Return the component accumulator and the tracks with muons removed
    """
    from TrkConfig.AtlasExtrapolatorConfig import TrigPFlowExtrapolatorCfg
    result = ComponentAccumulator()
    extrapolator = result.popToolsAndMerge(TrigPFlowExtrapolatorCfg(flags))
    output_tracks = f"PFMuonCaloTagTracks_{tracktype}"

    from TrackToCalo.TrackToCaloConfig import (
        HLTPF_ParticleCaloExtensionToolCfg,
        HLTPF_ParticleCaloCellAssociationToolCfg)

    caloext = result.popToolsAndMerge(HLTPF_ParticleCaloExtensionToolCfg(flags))
    calocellassoc = result.popToolsAndMerge(HLTPF_ParticleCaloCellAssociationToolCfg(
        flags,
        ParticleCaloExtensionTool=caloext,
        CaloCellContainer="",
    ))

    result.addEventAlgo(
        CompFactory.PFTrackMuonCaloTaggingAlg(
            f"PFTrackMuonCaloTaggingAlg_{tracktype}",
            InputTracks = tracksin,
            InputCaloExtension = extcache,
            InputCells = cellsin,
            OutputTracks = output_tracks,
            MinPt = flags.Trigger.FSHad.PFOMuonRemovalMinPt,
            MuonScoreTool = CompFactory.CaloMuonScoreTool(
                CaloMuonEtaCut=3,
                ParticleCaloCellAssociationTool = calocellassoc
            ),
            LooseTagTool=CompFactory.CaloMuonTag("LooseCaloMuonTag", TagMode="Loose"),
            TightTagTool=CompFactory.CaloMuonTag("TightCaloMuonTag", TagMode="Tight"),
            DepositInCaloTool=CompFactory.TrackDepositInCaloTool(
                ExtrapolatorHandle=extrapolator,
                ParticleCaloCellAssociationTool = calocellassoc,
                ParticleCaloExtensionTool = caloext
            )
        ),
        primary=True,
    )
    return result, output_tracks

def MuonIsoTagCfg(flags, tracktype, tracksin, verticesin, extcache, clustersin):
    """ Create the muon iso tagging configuration
    
    Return the component accumulator and the tracks with muons removed
    """
    result = ComponentAccumulator()
    output_tracks = f"PFMuonIsoTagTracks_{tracktype}"

    from TrackToCalo.TrackToCaloConfig import HLTPF_ParticleCaloExtensionToolCfg

    result.addEventAlgo(
        CompFactory.PFTrackMuonIsoTaggingAlg(
            f"PFTrackMuonIsoTaggingalg_{tracktype}",
            InputTracks = tracksin,
            InputClusters = clustersin,
            InputVertices = verticesin,
            OutputTracks = output_tracks,
            MinPt = flags.Trigger.FSHad.PFOMuonRemovalMinPt,
            TrackIsoTool = CompFactory.xAOD.TrackIsolationTool(
                TrackParticleLocation=tracksin,
                VertexLocation="",
            ),
            CaloIsoTool = CompFactory.xAOD.CaloIsolationTool(
                ParticleCaloExtensionTool=result.popToolsAndMerge(
                    HLTPF_ParticleCaloExtensionToolCfg(flags)),
                InputCaloExtension=extcache,
                ParticleCaloCellAssociationTool="",
                saveOnlyRequestedCorrections=True,
            )
        ),
        primary=True,
    )
    return result, output_tracks

#---------------------------------------------------------------------------------#
# PFlow track selection
def HLTPFTrackSelectorCfg(inputFlags,tracktype,tracksin,verticesin,clustersin,cellsin=None):
    result = ComponentAccumulator()

    muon_mode = inputFlags.Trigger.FSHad.PFOMuonRemoval
    if muon_mode == "None":
        tracks = tracksin
        extension_cache=""
    else:
        ext_acc, pretracks, extension_cache = PFTrackExtensionCfg(
            inputFlags, tracktype, tracksin
        )
        result.merge(ext_acc)
        if muon_mode == "Calo":
            if cellsin is None:
                raise ValueError("Cells must be provided for the 'Calo' muon mode!")
            tag_acc, tracks = MuonCaloTagCfg(
                inputFlags, tracktype, pretracks, extension_cache, cellsin
            )
        elif muon_mode == "Iso":
            tag_acc, tracks = MuonIsoTagCfg(
                inputFlags, tracktype, pretracks, verticesin, extension_cache, clustersin
            )
        else:
            raise ValueError(f"Invalid muon removal mode '{muon_mode}'")
        result.merge(tag_acc)

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        PFTrackSelectionToolCfg)
    from TrackToCalo.TrackToCaloConfig import HLTPF_ParticleCaloExtensionToolCfg


    from eflowRec import PFOnlineMon
    monTool_extrapolator = PFOnlineMon.getMonTool_eflowTrackCaloExtensionTool(inputFlags)
    monTool_extrapolator.HistPath = 'TrackExtrapolator'

    result.addEventAlgo(
        CompFactory.PFTrackSelector(
            f"PFTrackSelector_{tracktype}",
            trackExtrapolatorTool = CompFactory.eflowTrackCaloExtensionTool(
                "HLTPF_eflowTrkCaloExt",
                TrackCaloExtensionTool=result.popToolsAndMerge(
                    HLTPF_ParticleCaloExtensionToolCfg(inputFlags)),
                PFParticleCache = extension_cache,
                MonTool_TrackCaloExtension = monTool_extrapolator
            ),
            trackSelectionTool = result.popToolsAndMerge(PFTrackSelectionToolCfg(inputFlags)),
            electronsName="",
            muonsName="",
            tracksName=tracks,
            VertexContainer=verticesin,
            eflowRecTracksOutputName=f"eflowRecTracks_{tracktype}",
            MonTool = PFOnlineMon.getMonTool_PFTrackSelector(inputFlags),
        ),
        primary=True,
    )

    return result

#---------------------------------------------------------------------------------#
# Legacy (non-unified) particle flow tool configuration.
#
# Offline reconstruction runs the unified tool chain exclusively (see
# getOfflinePFAlgorithm in PFCfg.py). The HLT still runs the older combined
# chain, in which one PFSubtractionTool performs both track-cluster matching and
# charged shower subtraction, and cluster moments are calculated by
# PFMomentCalculatorTool. The configuration of those two tools therefore lives
# here, with the only client that still uses them, rather than in PFCfg.py.
# It is removed once the HLT is ported to the unified chain.
def getHLTPFCellLevelSubtractionTool(inputFlags,toolName):
    PFCellLevelSubtractionToolFactory = CompFactory.PFSubtractionTool
    PFCellLevelSubtractionTool = PFCellLevelSubtractionToolFactory(toolName,useNNEnergy = inputFlags.PF.useMLEOverP)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFCellLevelSubtractionTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss()
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFCellLevelSubtractionTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFCellLevelSubtractionTool.useLegacyEBinIndex=False

    if(inputFlags.PF.EOverPMode):
        PFCellLevelSubtractionTool.CalcEOverP = True
        PFCellLevelSubtractionTool.nClusterMatchesToUse = -1
    else:
        PFCellLevelSubtractionTool.nClusterMatchesToUse = 1

    if(inputFlags.PF.EOverPMode):
        PFCellLevelSubtractionTool.PFTrackClusterMatchingTool = getPFTrackClusterMatchingTool(inputFlags,0.2,"EtaPhiSquareDistance","PlainEtaPhi","CalObjBldMatchingTool")
    else:
        PFCellLevelSubtractionTool.PFTrackClusterMatchingTool = getPFTrackClusterMatchingTool(inputFlags,1.64,"EtaPhiSquareSignificance","GeomCenterEtaPhi","CalObjBldMatchingTool")

    PFCellLevelSubtractionTool.PFTrackClusterMatchingTool_02 = getPFTrackClusterMatchingTool(inputFlags,0.2,"EtaPhiSquareDistance","PlainEtaPhi","MatchingTool_Pull_02")

    if inputFlags.PF.useMLEOverP:
        PFEnergyPredictorTool = CompFactory.PFEnergyPredictorTool("PFCellLevelEnergyPredcictorTool",ModelPath = inputFlags.PF.EOverP_NN_Model)
        PFCellLevelSubtractionTool.NNEnergyPredictorTool = PFEnergyPredictorTool
    PFCellLevelSubtractionTool.addCPData = inputFlags.PF.addCPData

    if inputFlags.PF.useTruthCheating:
        if inputFlags.PF.useTrackClusterTruthMatching:
            PFCellLevelSubtractionTool.CaloClusterReadDecorHandleKey_NLeadingTruthParticles = "CaloTopoClusters." + inputFlags.Calo.TopoCluster.CalibrationHitDecorationName
            PFCellLevelSubtractionTool.useTrackClusterTruthMatching=True

        if inputFlags.PF.useTruthForChargedShowerSubtraction:
            PFCellLevelSubtractionTool.useTruthForChargedShowerSubtraction = True
            PFCellLevelSubtractionTool.PFSimulateTruthShowerTool = CompFactory.PFSimulateTruthShowerTool("PFSimulateTruthShowerTool")

    return PFCellLevelSubtractionTool


def getHLTPFRecoverSplitShowersTool(inputFlags,toolName):
    PFRecoverSplitShowersToolFactory = CompFactory.PFSubtractionTool
    PFRecoverSplitShowersTool = PFRecoverSplitShowersToolFactory(toolName,useNNEnergy = inputFlags.PF.useMLEOverP)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFRecoverSplitShowersTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss("eflowCellEOverPTool_Run2_mc20_JetETMiss_Recover")
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFRecoverSplitShowersTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool_Recover", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFRecoverSplitShowersTool.useLegacyEBinIndex=False

    PFRecoverSplitShowersTool.RecoverSplitShowers = True

    if inputFlags.PF.useMLEOverP:
        PFEnergyPredictorTool = CompFactory.PFEnergyPredictorTool("PFRecoverSplitShowersEnergyPredcictorTool",ModelPath = inputFlags.PF.EOverP_NN_Model)
        PFRecoverSplitShowersTool.NNEnergyPredictorTool = PFEnergyPredictorTool

    return PFRecoverSplitShowersTool


def getHLTPFMomentCalculatorTool(inputFlags):
    result = ComponentAccumulator()

    if inputFlags.PF.useClusterMoments:
        MomentsNames = [
            "FIRST_PHI" 
            ,"FIRST_ETA"
            ,"SECOND_R" 
            ,"SECOND_LAMBDA"
            ,"DELTA_PHI"
            ,"DELTA_THETA"
            ,"DELTA_ALPHA" 
            ,"CENTER_X"
            ,"CENTER_Y"
            ,"CENTER_Z"
            ,"CENTER_MAG"
            ,"CENTER_LAMBDA"
            ,"LATERAL"
            ,"LONGITUDINAL"
            ,"FIRST_ENG_DENS" 
            ,"ENG_FRAC_EM" 
            ,"ENG_FRAC_MAX" 
            ,"ENG_FRAC_CORE" 
            ,"FIRST_ENG_DENS" 
            ,"SECOND_ENG_DENS"
            ,"ISOLATION"
            ,"EM_PROBABILITY"
            ,"ENG_POS"
            ,"ENG_BAD_CELLS"
            ,"N_BAD_CELLS"
            ,"BADLARQ_FRAC"
            ,"AVG_LAR_Q"
            ,"AVG_TILE_Q"
            ,"SIGNIFICANCE"
        ]
    else:
        MomentsNames = ["CENTER_MAG"]

    PFMomentCalculatorTool = CompFactory.PFMomentCalculatorTool("PFMomentCalculatorTool")

    from CaloRec.CaloTopoClusterConfig import getTopoMoments
    PFClusterMomentsMaker = result.popToolsAndMerge(getTopoMoments(inputFlags))
    PFClusterMomentsMaker.MomentsNames = MomentsNames
    PFMomentCalculatorTool.CaloClusterMomentsMaker = PFClusterMomentsMaker

    PFClusterCollectionTool = CompFactory.PFClusterCollectionTool
    PFMomentCalculatorTool.PFClusterCollectionTool = PFClusterCollectionTool("PFClusterCollectionTool")

    if(inputFlags.PF.useCalibHitTruthClusterMoments):
        PFMomentCalculatorTool.UseCalibHitTruth=True
        from CaloRec.CaloTopoClusterConfig import getTopoCalibMoments
        PFMomentCalculatorTool.CaloCalibClusterMomentsMaker2 = getTopoCalibMoments(inputFlags)

    result.setPrivateTools(PFMomentCalculatorTool)
    return result

def PFCfg(inputFlags, tracktype="", clustersin=None, calclustersin=None, tracksin=None, verticesin=None, cellsin=None):

    result=ComponentAccumulator()
    seqname = f'HLTPFlow_{tracktype}'
    result.addSequence(parOR(seqname))

    # Set defaults for the inputs
    if clustersin is None:
        clustersin=inputFlags.eflowRec.RawClusterColl
    if calclustersin is None:
        calclustersin=inputFlags.eflowRec.CalClusterColl
    if tracksin is None:
        tracksin = inputFlags.eflowRec.TrackColl
    if verticesin is None:
        verticesin = inputFlags.eflowRec.VertexColl

    result.merge(TrackingGeoCfg(inputFlags))
    calogeocfg = CaloGeoAndNoiseCfg(inputFlags)
    result.merge(calogeocfg)

    selcfg = HLTPFTrackSelectorCfg(inputFlags, tracktype, tracksin, verticesin, clustersin, cellsin)
    PFTrackSelector = selcfg.getPrimary()

    # Add monitoring tool
    monTool = PFOnlineMon.getMonTool_PFTrackSelector(inputFlags)
    PFTrackSelector.MonTool = monTool

    result.merge( selcfg, seqname )

    #---------------------------------------------------------------------------------#
    # PFlowAlgorithm -- subtraction steps


    from eflowRec.PFCfg import getPFClusterSelectorTool

    PFTrackClusterMatchingTool_1 = CompFactory.PFTrackClusterMatchingTool("CalObjBldMatchingTool")
    monTool_matching = PFOnlineMon.getMonTool_PFTrackClusterMatching(inputFlags)
    monTool_matching.HistPath = 'PFTrackClusterMatchingTool_1'
    PFTrackClusterMatchingTool_1.MonTool_ClusterMatching = monTool_matching

    cellSubtractionTool = getHLTPFCellLevelSubtractionTool(
        inputFlags,
        "PFCellLevelSubtractionTool",
    )
    cellSubtractionTool.PFTrackClusterMatchingTool=PFTrackClusterMatchingTool_1

    recoverSplitShowersTool = getHLTPFRecoverSplitShowersTool(
        inputFlags,
        "PFRecoverSplitShowersTool",
    )
    recoverSplitShowersTool.PFTrackClusterMatchingTool = PFTrackClusterMatchingTool_1

    result.addEventAlgo(
        CompFactory.PFAlgorithm(
            f"PFAlgorithm_{tracktype}",
            PFClusterSelectorTool = getPFClusterSelectorTool(
                inputFlags,
                clustersin,
                calclustersin,
                "PFClusterSelectorTool",
            ),
            SubtractionToolList = [
                cellSubtractionTool,
                recoverSplitShowersTool,
            ],
            BaseToolList = [
                result.popToolsAndMerge(getHLTPFMomentCalculatorTool(inputFlags)),
            ],
            MonTool = PFOnlineMon.getMonTool_PFAlgorithm(inputFlags),
            eflowRecTracksInputName = PFTrackSelector.eflowRecTracksOutputName,
            eflowRecClustersOutputName = f"eflowRecClusters_{tracktype}",
            PFCaloClustersOutputName = f"PFCaloCluster_{tracktype}",
            eflowCaloObjectsOutputName = f"eflowCaloObjects_{tracktype}",
        ),
        seqname
    )

    #---------------------------------------------------------------------------------#
    # PFO creators here

    chargedPFOArgs = dict(
            inputFlags=inputFlags,
            nameSuffix=f"_{tracktype}",
            chargedFlowElementOutputName=recordable(f"HLT_{tracktype}ChargedParticleFlowObjects"),
            eflowCaloObjectContainerName=f"eflowCaloObjects_{tracktype}"
    )
    neutralPFOArgs = dict(
            inputFlags=inputFlags,
            nameSuffix=f"_{tracktype}",
            neutralFlowElementOutputName=recordable(f"HLT_{tracktype}NeutralParticleFlowObjects"),
            eflowCaloObjectContainerName=f"eflowCaloObjects_{tracktype}"
    )
    from eflowRec.PFCfg import getChargedFlowElementCreatorAlgorithm,getNeutralFlowElementCreatorAlgorithm
    result.addEventAlgo(getNeutralFlowElementCreatorAlgorithm(**neutralPFOArgs), seqname)
    result.addEventAlgo(getChargedFlowElementCreatorAlgorithm(**chargedPFOArgs), seqname)

    
    return result

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    cfgFlags = initConfigFlags()
    #cfgFlags.Input.Files=["myESD.pool.root"]
    cfgFlags.Input.Files=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc16_13TeV.361022.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ2W.recon.ESD.e3668_s3170_r10572_homeMade.pool.root"]
    #
    cfgFlags.addFlag("eflowRec.TrackColl","InDetTrackParticles")
    cfgFlags.addFlag("eflowRec.VertexColl","PrimaryVertices")
    cfgFlags.addFlag("eflowRec.RawClusterColl","CaloTopoClusters")
    cfgFlags.addFlag("eflowRec.CalClusterColl","CaloCalTopoClustersNew")

    #PF flags
    cfgFlags.PF.addClusterMoments = False
    cfgFlags.PF.useClusterMoments = False
    
    #
    # Try to get around TRT alignment folder problem in MC
    cfgFlags.GeoModel.Align.Dynamic = False
    #
    cfgFlags.lock()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    cfg=MainServicesCfg(cfgFlags) 

    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    tccfg = CaloTopoClusterCfg(cfgFlags)
    tcalg = tccfg.getPrimary()
    tcalg.ClustersOutputName = "CaloCalTopoClustersNew"
    cfg.merge(tccfg)
    cfg.addEventAlgo(tcalg,sequenceName="AthAlgSeq")

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))

    cfg.merge(PFCfg(cfgFlags))

    cfg.printConfig()# (summariseProps=True)

    outputlist = [
        "xAOD::CaloClusterContainer#CaloCalTopoClusters*",
        "xAOD::CaloClusterAuxContainer#*CaloCalTopoClusters*Aux.",
        "xAOD::PFOContainer#*ParticleFlowObjects",
        "xAOD::PFOAuxContainer#*ParticleFlowObjectsAux."
        ]
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
    cfg.merge(OutputStreamCfg(cfgFlags,"xAOD",ItemList=outputlist))
    from pprint import pprint
    pprint( cfg.getEventAlgo(outputStreamName("xAOD")).ItemList )

    histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='expert-monitoring.root', OPT='RECREATE'"])
    cfg.addService(histSvc)

    cfg.getService("StoreGateSvc").Dump = True

    cfg.run(10)
