## Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Constants import INFO

def SCTVALIDKernelCfg(flags, name='SCTVALIDKernel', **kwargs):
    """Configure the derivation framework driving algorithm (kernel) for SCTVALID"""
    acc = ComponentAccumulator()

    #################
    ### Setup skimming tools
    #################
    skimmingTools = []

    SCTVALIDSequenceName='SCTVALIDSequence'
    acc.addSequence(seqAND(SCTVALIDSequenceName))

    # Applying prescales 
    # https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework#Applying_prescales
    
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import PrescaleToolCfg

    prescaleTool = acc.getPrimaryAndMerge(PrescaleToolCfg(
            flags, name="SCTxAOD_PrescaleTool", Prescale=flags.InDet.SCTxAODPrescale))

    skimmingTools += [prescaleTool]

    SCTVALIDKernelPresel = CompFactory.DerivationFramework.DerivationKernel("SCTVALIDKernelPresel",
                                                                            SkimmingTools=skimmingTools)
    acc.addEventAlgo(SCTVALIDKernelPresel, sequenceName="SCTVALIDSequence")

    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetSCT_PrepDataToxAODCfg
    acc.merge(InDetSCT_PrepDataToxAODCfg(flags,name="xAOD_SCT_PrepDataToxAOD",
                                         OutputLevel=INFO,
                                         WriteRDOinformation=True, # TO CHECK
                                         WriteSiHits=False,
                                         WriteSDOs=False,
                                         UseTruthInfo=flags.Input.isMC), 
              sequenceName=SCTVALIDSequenceName)


    from InDetConfig.InDetPrepRawDataToxAODConfig import InDetSCT_RawDataToxAODCfg
    acc.merge(InDetSCT_RawDataToxAODCfg(flags, name = "xAOD_SCT_RawDataToxAOD"),
              sequenceName=SCTVALIDSequenceName)    
    
    # Add the TSOS augmentation tool to the derivation framework
    augmentationTools=[]

    if flags.Reco.EnableTracking is True:
        from DerivationFrameworkInDet.InDetToolsConfig import TrackStateOnSurfaceDecoratorCfg
        DFTSOS = acc.getPrimaryAndMerge(TrackStateOnSurfaceDecoratorCfg(flags, name="SCTVALID_DFTrackStateOnSurfaceDecorator",
                                                                        DecorationPrefix = "SCTVALID_",
                                                                        StoreTRT=True,
                                                                        StoreSCT=True,
                                                                        StorePixel=True,
                                                                        PRDtoTrackMap="")
                                        )
        augmentationTools.append(DFTSOS)

    from DerivationFrameworkInDet.InDetToolsConfig import EventInfoBSErrDecoratorCfg
    DFEI = acc.getPrimaryAndMerge(EventInfoBSErrDecoratorCfg(flags, name = "SCTxAOD_DFEventInfoBSErrDecorator"))
    augmentationTools.append(DFEI)

    from DerivationFrameworkInDet.InDetToolsConfig import UnassociatedHitsGetterToolCfg
    unassociatedHitsGetterTool = acc.popToolsAndMerge(UnassociatedHitsGetterToolCfg(flags, name="SCTVALID_UnassociatedHitsGetterTool",
                                                                                      TrackCollection = "CombinedInDetTracks",
                                                                                      PixelClusters = "PixelClusters",
                                                                                      SCTClusterContainer = "SCT_Clusters",
                                                                                      TRTDriftCircleContainer = "TRT_DriftCircles"))

    from DerivationFrameworkJetEtMiss.JetToolConfig import DistanceInTrainToolCfg
    distanceInTrainTool = acc.getPrimaryAndMerge(DistanceInTrainToolCfg(flags))
    augmentationTools.append(distanceInTrainTool)
    
    from DerivationFrameworkInDet.InDetToolsConfig import UnassociatedHitsDecoratorCfg
    unassociatedHitsDecorator = acc.getPrimaryAndMerge(UnassociatedHitsDecoratorCfg(flags, 
                                                                                    name='SCTxAOD_unassociatedHitsDecorator',
                                                                                    UnassociatedHitsGetter = unassociatedHitsGetterTool))
    augmentationTools.append(unassociatedHitsDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=augmentationTools,
        SkimmingTools=skimmingTools,
        ThinningTools=[],
        OutputLevel=INFO), sequenceName=SCTVALIDSequenceName)
    return acc

# Main config
def SCTVALIDCfg(flags):
    """Main config fragment for SCTVALID"""
    acc = ComponentAccumulator()

    # Main algorithm (kernel)
    acc.merge(SCTVALIDKernelCfg(flags, 
                                name = "SCTVALIDKernel",
                                StreamName = 'StreamDAOD_SCTVALID') )

    # =============================
    # Define contents of the format
    # =============================
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    SCTVALIDSlimmingHelper = SlimmingHelper(
        "SCTVALIDSlimmingHelper",
        NamesAndTypes = flags.Input.TypedCollections,
        flags         = flags
        )

    AllVariables = []
    StaticContent = []
    SmartCollections = []
    ExtraVariables = []

    SCTVALIDSlimmingHelper.AppendToDictionary.update({
        "EventInfo": "xAOD::EventInfo", "EventInfoAux": "xAOD::EventAuxInfo",
        "Muons": "xAOD::MuonContainer", "MuonsAux": "xAOD::MuonAuxContainer",
        "Electrons": "xAOD::ElectronContainer", "ElectronsAux": "xAOD::ElectronAuxContainer",
        "GSFTrackParticles": "xAOD::TrackParticleContainer",
        "GSFTrackParticlesAux": "xAOD::TrackParticleAuxContainer", 

    })  

    AllVariables += ["EventInfo", "InDetTrackParticles",] 
    SmartCollections += ["Muons", "Electrons"]

    excludedPrimaryVerticesAuxData = "-VTAV"
    excludedInDetTrackParticlesAuxData = ".-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition"
    excludedGSFTrackParticlesAuxData = ".-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition"
    
    StaticContent = []
    StaticContent += ["xAOD::VertexContainer#PrimaryVertices"]
    StaticContent += ["xAOD::VertexAuxContainer#PrimaryVerticesAux" + excludedPrimaryVerticesAuxData]
    StaticContent += ["xAOD::TrackParticleContainer#InDetTrackParticles"]
    StaticContent += ["xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux" + excludedInDetTrackParticlesAuxData]
    StaticContent += ["xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux"]

    StaticContent += ["xAOD::TrackParticleContainer#GSFTrackParticles"]
    StaticContent += ["xAOD::TrackParticleAuxContainer#GSFTrackParticlesAux" + excludedGSFTrackParticlesAuxData]


    StaticContent += ["TileCellContainer#MBTSContainer"]
    StaticContent += ["BCM_RDOs#BCM_RDO_Container"]

    SCTVALIDSlimmingHelper.AppendToDictionary.update(
            {
             "SCT_MSOSs": "xAOD::TrackStateValidationContainer",
             "SCT_MSOSsAux": "xAOD::TrackStateValidationAuxContainer",
             "SCT_Clusters": "xAOD::TrackMeasurementValidationContainer",
             "SCT_ClustersAux": "xAOD::TrackMeasurementValidationAuxContainer",
             "SCT_RawHits": "xAOD::SCTRawHitValidationContainer",
             "SCT_RawHitsAux": "xAOD::SCTRawHitValidationAuxContainer"})

    AllVariables += [
                    "SCT_MSOSs", 
                    "SCT_Clusters",
                    "SCT_RawHits", 
                    "SCT_RawHitsAux"]

    if flags.Input.isMC:

        SCTVALIDSlimmingHelper.AppendToDictionary.update({
            "TruthEvents": "xAOD::TruthEventContainer",
            "TruthEventsAux": "xAOD::TruthEventAuxContainer",
            "TruthParticles": "xAOD::TruthParticleContainer",
            "TruthParticlesAux": "xAOD::TruthParticleAuxContainer",
            "TruthVertices": "xAOD::TruthVertexContainer",
            "TruthVerticesAux": "xAOD::TruthVertexAuxContainer"})
        
        AllVariables += ["TruthEvents", "TruthParticles", "TruthVertices"]

    # Trigger info is actually stored only when running on data...
    SCTVALIDSlimmingHelper.IncludeTriggerNavigation = True
    SCTVALIDSlimmingHelper.IncludeAdditionalTriggerContent = True

    SCTVALIDSlimmingHelper.AllVariables = AllVariables
    SCTVALIDSlimmingHelper.StaticContent = StaticContent
    SCTVALIDSlimmingHelper.SmartCollections = SmartCollections
    SCTVALIDSlimmingHelper.ExtraVariables = ExtraVariables

    # Output stream
    SCTVALIDItemList = SCTVALIDSlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_SCTVALID",
              ItemList=SCTVALIDItemList, AcceptAlgs=["SCTVALIDKernel"]))
    acc.merge(SetupMetaDataForStreamCfg(
        flags, "DAOD_SCTVALID",
        AcceptAlgs=["SCTVALIDKernel"],
        createMetadata=[MetadataCategory.CutFlowMetaData, 
                        MetadataCategory.TriggerMenuMetaData]))

    return acc
