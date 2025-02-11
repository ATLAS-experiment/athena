## Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory
from AthenaCommon.CFElements import seqAND
from AthenaCommon.Constants import INFO

def SCTVALIDTriggerSkimmingToolCfg(flags, name, **kwargs):
    
    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType

    # This is not all periods! Run 2 and current triggers only
    allperiods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future2e34
    TriggerAPI.setConfigFlags(flags)
    triggers  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)

    #remove duplicates
    triggers = sorted(list(set(triggers)))

    acc = ComponentAccumulator()
    TriggerSkimmingTool = CompFactory.DerivationFramework.TriggerSkimmingTool
    acc.addPublicTool(TriggerSkimmingTool(name, 
                                          TriggerListAND = [],
                                          TriggerListOR  = triggers,
                                          **kwargs),
                                          primary = True)
    return acc

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
        MSOSThinningSelectionString = ""
        if flags.InDet.SCTxAODZmumuSkimming:
            MSOSThinningSelectionString = "InDetTrackParticles.pt>10*GeV"
        from DerivationFrameworkInDet.InDetToolsConfig import TrackStateOnSurfaceDecoratorCfg
        DFTSOS = acc.getPrimaryAndMerge(TrackStateOnSurfaceDecoratorCfg(flags, name="SCTVALID_DFTrackStateOnSurfaceDecorator",
                                                                        DecorationPrefix = "SCTVALID_",
                                                                        StoreTRT=True,
                                                                        StoreSCT=True,
                                                                        StorePixel=True,
                                                                        PRDtoTrackMap="",
                                                                        SelectionString=MSOSThinningSelectionString)
                                        )
        augmentationTools.append(DFTSOS)

    from DerivationFrameworkInDet.InDetToolsConfig import EventInfoBSErrDecoratorCfg
    DFEI = acc.getPrimaryAndMerge(EventInfoBSErrDecoratorCfg(flags, name = "SCTxAOD_DFEventInfoBSErrDecorator"))
    augmentationTools.append(DFEI)

    # track isolation
    from IsolationAlgs.IsoToolsConfig import TrackIsolationToolCfg, CaloIsolationToolCfg
    TrackIsoToolStd = acc.popToolsAndMerge(TrackIsolationToolCfg(flags))

    from CaloIdentifier import SUBCALO
    CaloIsoTool = acc.popToolsAndMerge(CaloIsolationToolCfg(flags,
                                                            name = "CaloIsoTool",
                                                            EMCaloNums = [SUBCALO.LAREM],
                                                            HadCaloNums = [SUBCALO.LARHEC, SUBCALO.TILE],
                                                            UseEMScale  = True,
                                                            UseCaloExtensionCaching = False,
                                                            saveOnlyRequestedCorrections = True))

    import ROOT
    isoPar = ROOT.xAOD.Iso.IsolationType
    from DerivationFrameworkInDet.InDetToolsConfig import IsolationTrackDecoratorCfg
    IsolationTrackDecoratorTool = acc.getPrimaryAndMerge(IsolationTrackDecoratorCfg(flags,
                                                                                    name               = "SCTVALID_IsolationTrackDecorator",
                                                                                    TrackIsolationTool = TrackIsoToolStd,
                                                                                    CaloIsolationTool  = CaloIsoTool,
                                                                                    TargetContainer    = "InDetTrackParticles",
                                                                                    SelectionString    = "InDetTrackParticles.pt>10*GeV",
                                                                                    iso                = [isoPar.ptcone40, isoPar.ptcone30, isoPar.ptcone20],
                                                                                    isoSuffix          = ["ptcone40", "ptcone30", "ptcone20"],
                                                                                    Prefix             = "SCTVALID_"))
    augmentationTools.append(IsolationTrackDecoratorTool)

    from DerivationFrameworkInDet.InDetToolsConfig import UnassociatedHitsGetterToolCfg
    unassociatedHitsGetterTool = acc.popToolsAndMerge(UnassociatedHitsGetterToolCfg(flags, name="SCTVALID_UnassociatedHitsGetterTool",
                                                                                      TrackCollection = "CombinedInDetTracks",
                                                                                      PixelClusters = "PixelClusters",
                                                                                      SCTClusterContainer = "SCT_Clusters",
                                                                                      TRTDriftCircleContainer = "TRT_DriftCircles"))

    from DerivationFrameworkInDet.InDetToolsConfig import UnassociatedHitsDecoratorCfg
    unassociatedHitsDecorator = acc.getPrimaryAndMerge(UnassociatedHitsDecoratorCfg(flags, 
                                                                                    name='SCTxAOD_unassociatedHitsDecorator',
                                                                                    UnassociatedHitsGetter = unassociatedHitsGetterTool))
    augmentationTools.append(unassociatedHitsDecorator)

    from DerivationFrameworkJetEtMiss.JetToolConfig import DistanceInTrainToolCfg
    distanceInTrainTool = acc.getPrimaryAndMerge(DistanceInTrainToolCfg(flags))
    augmentationTools.append(distanceInTrainTool)
    
    # thinning tools
    thinningTools=[]

    if flags.InDet.SCTxAODZmumuSkimming:
        from DerivationFrameworkInDet.InDetToolsConfig import TrackParticleThinningCfg, MuonTrackParticleThinningCfg
        
        # Tag and probe track thinning
        thinning_expression = " && ".join(
            [
                "(InDetTrackParticles.pt > 10*GeV)",
                "(abs(InDetTrackParticles.eta) < 2.5)",
            ]
        )
        SCTVALID_TagAndProbeTrackParticleThinningTool = CompFactory.DerivationFramework.TagAndProbeTrackParticleThinning(name            = "SCTVALID_TagAndProbeTrackParticleThinningTool",
                                                                                                                         SelectionString = thinning_expression,
                                                                                                                         d0SignifCut     = 5.0,
                                                                                                                         z0Cut           = 10.0,
                                                                                                                         StreamName      = kwargs['StreamName'])
        acc.addPublicTool(SCTVALID_TagAndProbeTrackParticleThinningTool)
        thinningTools.append(SCTVALID_TagAndProbeTrackParticleThinningTool)

        # Pixel tracklets need to have greater than 5 GeV of pT
        SCTVALID_DTTrackParticleThinningTool = acc.getPrimaryAndMerge(TrackParticleThinningCfg(flags,
                                                                                               name                    = "SCTVALID_DTTrackParticleThinningTool",
                                                                                               StreamName              = kwargs['StreamName'],
                                                                                               SelectionString         = "InDetDisappearingTrackParticles.pt>5*GeV",
                                                                                               InDetTrackParticlesKey  = "InDetDisappearingTrackParticles"))
        thinningTools.append(SCTVALID_DTTrackParticleThinningTool)
    
        # Include inner detector tracks associated with muons
        SCTVALID_MuonTPThinningTool = acc.getPrimaryAndMerge(MuonTrackParticleThinningCfg(flags,
                                                                                          name       = "SCTVALID_MuonTPThinningTool",
                                                                                          StreamName = kwargs['StreamName'],
                                                                                          MuonKey    = "Muons"))
        thinningTools.append(SCTVALID_MuonTPThinningTool)

        # Trigger skimming
        SCTVALIDTriggerSkimmingTool = acc.getPrimaryAndMerge(SCTVALIDTriggerSkimmingToolCfg(flags,
                                                                                            name = "SCTVALIDTriggerSkimmingTool"))

        skimmingTools.append(SCTVALIDTriggerSkimmingTool)

        # at least one good quality muon
        muonRequirements = '( Muons.pt > 20*GeV ) && ( Muons.ptvarcone30 / Muons.pt < 0.15 ) && ( Muons.topoetcone20 / Muons.pt < 0.3 )'
        muonExpression   = '( count('+muonRequirements+') >= 1 )'
        from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
        SCTVALIDSkimmingTool = acc.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                                                name       = "SCTVALIDSkimmingTool",
                                                                                expression = muonExpression))
        skimmingTools.append(SCTVALIDSkimmingTool)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=augmentationTools,
        SkimmingTools=skimmingTools,
        ThinningTools=thinningTools,
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
        "PrimaryVertices": "xAOD::VertexContainer", "PrimaryVerticesAux": "xAOD::VertexAuxContainer",
        "GSFTrackParticles": "xAOD::TrackParticleContainer",
        "GSFTrackParticlesAux": "xAOD::TrackParticleAuxContainer", 
        "InDetDisappearingTrackParticles": "xAOD::TrackParticleContainer",
        "InDetDisappearingTrackParticlesAux": "xAOD::TrackParticleAuxContainer", 
        "MuonSpectrometerTrackParticles": "xAOD::TrackParticleContainer",
        "MuonSpectrometerTrackParticlesAux": "xAOD::TrackParticleAuxContainer",
        "AntiKt4EMPFlowJets": "xAOD::JetContainer", "AntiKt4EMPFlowJetsAux": "xAOD::JetAuxContainer",
    })  

    AllVariables += ["EventInfo",
                     "InDetTrackParticles",
                     "InDetDisappearingTrackParticles",
                     "MuonSpectrometerTrackParticles"]
    SmartCollections += ["Muons", "Electrons", "AntiKt4EMPFlowJets"]

    ExtraVariables += ["PrimaryVertices.sumPt2.x.y.z.vertexType.nTrackParticles"]

    excludedInDetTrackParticlesAuxData = ".-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition"
    excludedGSFTrackParticlesAuxData = ".-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition"
    excludedMuonSpectrometerTrackParticlesAuxData = ".-clusterAssociation.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition.-btagIp_d0.-btagIp_d0Uncertainty.-btagIp_z0SinTheta.-btagIp_z0SinThetaUncertainty.-eProbabilityNN.-btagIp_trackMomentum.-btagIp_trackDisplacement.-TRTdEdxUsedHits.-TRTdEdx.-TTVA_AMVFVertices_forReco.-TTVA_AMVFWeights_forReco.-JetFilter_TrackCompatibility_antikt4emtopo.-btagIp_invalidIp.-JetFilter_TrackCompatibility_antikt4empflow.-nBC_meas.-AssoClustersUFO"
    
    StaticContent = []
    StaticContent += ["xAOD::TrackParticleContainer#InDetTrackParticles"]
    StaticContent += ["xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux" + excludedInDetTrackParticlesAuxData]
    StaticContent += ["xAOD::TrackParticleContainer#InDetDisappearingTrackParticles"]
    StaticContent += ["xAOD::TrackParticleAuxContainer#InDetDisappearingTrackParticlesAux" + excludedInDetTrackParticlesAuxData]
    StaticContent += ["xAOD::TrackParticleContainer#MuonSpectrometerTrackParticles"]
    StaticContent += ["xAOD::TrackParticleAuxContainer#MuonSpectrometerTrackParticlesAux" + excludedMuonSpectrometerTrackParticlesAuxData]
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

    if flags.InDet.SCTxAODSaveOnlyAssociatedMSOS is True:
        AllVariables += ["SCT_MSOSs"]
    else:
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
