# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# MuonsCommonConfig.py
# Configures all tools needed for muon object selection and kernels
# used to write results into SG.
# ComponentAccumulator version
#********************************************************************

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonsCommonCfg(flags, suff=""):
    """Main method configuring common muon augmentations"""

    Container = "Muons"+suff

    acc = ComponentAccumulator()
    #====================================================================
    # MCP GROUP TOOLS
    #====================================================================

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    DFCommonMuonToolWrapperTools = []

    ### IDHits
    # turn of the momentum correction which is not needed for IDHits cut and Preselection
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg

    DFCommonMuonsSelector = acc.popToolsAndMerge(MuonSelectionToolCfg(flags,
                                                                      name            = "DFCommonMuonsSelector",
                                                                      MaxEta          = 3.,
                                                                      MuQuality       = 3,
                                                                      TurnOffMomCorr  = True))
    acc.addPublicTool(DFCommonMuonsSelector)
    DFCommonMuonToolWrapperIDCuts = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(
        flags,
        name               = "DFCommonMuonToolWrapperIDCuts"+suff,
        AsgSelectionTool   = DFCommonMuonsSelector,
        CutType            = "IDHits",
        StoreGateEntryName = "DFCommonMuonPassIDCuts",
        ContainerName      = Container))
    DFCommonMuonToolWrapperTools.append(DFCommonMuonToolWrapperIDCuts)

    DFCommonMuonToolWrapperPreselection = acc.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(
        flags,
        name               = "DFCommonMuonToolWrapperPreselection"+suff,
        AsgSelectionTool   = DFCommonMuonsSelector,
        CutType            = "Preselection",
        StoreGateEntryName = "DFCommonMuonPassPreselection",
        ContainerName      = Container))
    DFCommonMuonToolWrapperTools.append(DFCommonMuonToolWrapperPreselection)

    #############
    #  Add tools
    #############
    CommonAugmentation = CompFactory.DerivationFramework.CommonAugmentation
    acc.addEventAlgo(CommonAugmentation("DFCommonMuonsKernel"+suff,
                                        AugmentationTools = DFCommonMuonToolWrapperTools))

    from IsolationAlgs.DerivationTrackIsoConfig import DerivationTrackIsoCfg
    # A selection of WP is probably needed, as only a few variables are in CP content !
    #   maybe MUON derivations can add some other ones for studies
    #listofTTVAWP = [ 'Loose', 'Nominal', 'Tight',
    #                 'Prompt_D0Sig', 'Prompt_MaxWeight',
    #                 'Nonprompt_Hard_D0Sig',
    #                 'Nonprompt_Medium_D0Sig',
    #                 'Nonprompt_All_D0Sig',
    #                 'Nonprompt_Hard_MaxWeight',
    #                 'Nonprompt_Medium_MaxWeight',
    #                 'Nonprompt_All_MaxWeight' ]
    for WP in [ 'Nonprompt_All_MaxWeight' ]:
        acc.merge(DerivationTrackIsoCfg(flags, WP = WP, object_types = ('Electrons', 'Muons'), postfix=suff))

    if "LRT" in Container and not hasattr(acc, 'LRTMuonCaloIsolationBuilder'):
        from IsolationAlgs.IsolationSteeringDerivConfig import LRTMuonIsolationSteeringDerivCfg
        acc.merge(LRTMuonIsolationSteeringDerivCfg(flags))

        from IsolationAlgs.IsolationBuilderConfig import muIsolationCfg
        acc.merge(muIsolationCfg(flags,
                                 name="muonIsolationLRT",
                                 # Avoid overlap with the previously-configured IsolationBuilder.
                                 noCalo=True,
                                 MuonCollectionContainerName = Container
                                 ))

    return acc

def MuonVariablesCfg(flags):
    extraVariablesMuons = [
        "pt","eta", "phi","truthType","truthOrigin","truthClassification","author","muonType","charge","allAuthors", "CaloMuonIDTag", "CaloMuonScore",
        ## Link to the different track particles & segments
        "inDetTrackParticleLink","extrapolatedMuonSpectrometerTrackParticleLink", "muonSpectrometerTrackParticleLink","combinedTrackParticleLink", "TruthLink","truthParticleLink", "msOnlyExtrapolatedMuonSpectrometerTrackParticleLink", "clusterLink", "muonSegmentLinks",
        "InnerDetectorPt","MuonSpectrometerPt","DFCommonGoodMuon", "momentumBalanceSignificance","scatteringCurvatureSignificance","scatteringNeighbourSignificance",
        "neflowisol20","topoetcone20", "topoetcone20_CloseByCorr", "neflowisol20_CloseByCorr",
        "ptcone20_Nonprompt_All_MaxWeightTTVA_pt1000", "ptcone20_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000", "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500",
        "ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt500_CloseByCorr","ptvarcone30_Nonprompt_All_MaxWeightTTVA_pt1000_CloseByCorr",
        ##readding these for the time being, we should asap validate if we can safely remove them since these are outdated
        "ptcone20","ptcone30","ptcone40","ptvarcone20","ptvarcone30","ptvarcone40","topoetcone30","topoetcone40",
        ## Hit summary
        "numberOfPrecisionLayers","combinedTrackOutBoundsPrecisionHits","numberOfPrecisionLayers","numberOfPrecisionHoleLayers","numberOfGoodPrecisionLayers",
        "innerSmallHits","innerLargeHits","middleSmallHits","middleLargeHits","outerSmallHits","outerLargeHits",
        "extendedSmallHits","extendedLargeHits",
        "extendedSmallHoles","isSmallGoodSectors",
        ## to recheck asap if these are used
        "extendedClosePrecisionHits","extendedLargeHoles","extendedOutBoundsPrecisionHits","innerClosePrecisionHits","innerLargeHoles","innerOutBoundsPrecisionHits","innerSmallHoles","isEndcapGoodLayers","middleClosePrecisionHits","middleLargeHoles","middleOutBoundsPrecisionHits","middleSmallHoles","outerClosePrecisionHits","outerLargeHoles","outerOutBoundsPrecisionHits","outerSmallHoles","segmentDeltaEta","spectrometerFieldIntegral",
        ### Eloss
        "EnergyLoss","energyLossType",
        #### Added by the MCP derivation framework
       "DFCommonJetDr","DFCommonMuonPassIDCuts","DFCommonMuonPassPreselection","DFCommonGoodMuon",
        ##we should validate asap if we can remove this
       "CaloLRLikelihood","quality",
    ]
    ### Depending on the run period add other decorations
    from AthenaConfiguration.Enums import LHCPeriod
    if flags.GeoModel.Run >= LHCPeriod.Run3: extraVariablesMuons += [
            "etaLayer1STGCHits", "etaLayer2STGCHits","phiLayer1STGCHits","phiLayer2STGCHits","MMHits"
    ]
    else: extraVariablesMuons +=  [
            "cscUnspoiledEtaHits","cscEtaHits",
    ]
    return extraVariablesMuons


def CombinedTrackVarsCfg(flags):
    return ["phi","theta","qOverP","d0","z0","vz","truthType","truthOrigin","truthClassification",
            "definingParametersCovMatrixDiag", "definingParametersCovMatrixOffDiag",
            "chiSquared","numberDoF",
            "vertexLink",
            "truthParticleLink",
            #ID hit summary
            "numberOfPixelHits","numberOfPixelHoles","numberOfPixelDeadSensors", "numberOfInnermostPixelLayerHits",
            "numberOfSCTHits","numberOfSCTHoles","numberOfSCTDeadSensors",
            "numberOfTRTHits","numberOfTRTOutliers",
            # MS hit summary
            "numberOfPrecisionLayers","numberOfPrecisionHoleLayers",
            "numberOfPhiLayers","numberOfPhiHoleLayers",
            ## AEOTS
            "alignEffectChId","alignEffectDeltaTrans","alignEffectSigmaDeltaTrans",
            "alignEffectDeltaAngle","alignEffectSigmaDeltaAngle",
            ]

def MuonCPMETrkVarsCfg(flags):
    return [
        #Perigee
     "phi", "theta","qOverP", "d0", "z0", "vz", "truthType", "truthOrigin", "truthClassification",
        "definingParametersCovMatrixDiag", "definingParametersCovMatrixOffDiag",
        "chiSquared", "numberDoF",

     "vertexLink", "truthParticleLink",
        #ID hit summary (these shouldn't be here, adding them to fully replicate the previous version of DAODs since I got a crash somehow in tests; we should remove them asap)
     "numberOfPixelHits","numberOfPixelHoles","numberOfPixelDeadSensors",
        "numberOfSCTHits","numberOfSCTHoles","numberOfSCTDeadSensors",
        "numberOfTRTHits","numberOfTRTOutliers",
        # MS hit summary
     "numberOfPhiLayers", "numberOfPhiHoleLayers",
        "numberOfPrecisionHoleLayers", "numberOfPrecisionLayers",
        # AEOT
     "alignEffectChId", "alignEffectDeltaTrans", "alignEffectSigmaDeltaTrans",  "alignEffectDeltaAngle", "alignEffectSigmaDeltaAngle"
    ]

def MSTrkVarsCfg(flags):
    return [
        "theta","qOverP","phi","d0","z0","vz",
        "definingParametersCovMatrixDiag","definingParametersCovMatrixOffDiag",
        "chiSquared", "numberDoF",
        "vertexLink","truthParticleLink"
    ]

def MuonCPInDetSiAssocVarsCfg(flags):
    return  [
        "theta","phi","qOverP", "d0", "z0", "vz",
        "chiSquared", "numberDoF",
        "definingParametersCovMatrixDiag","definingParametersCovMatrixOffDiag",
        ## Track summary
        "numberOfPixelHits","numberOfPixelDeadSensors", "numberOfPixelHoles",
        "numberOfSCTHits","numberOfSCTDeadSensors", "numberOfSCTHoles",
        "numberOfTRTHits","numberOfTRTOutliers",
        ### Hmm why's this not in the other variables
        "truthType","truthOrigin","truthClassification",
    ]

def MuonCPInDetVarsCfg(flags):
  return ["phi","theta","qOverP","numberOfPixelHits","numberOfPixelHoles","numberOfPixelDeadSensors","numberOfSCTHits","numberOfSCTHoles","numberOfSCTDeadSensors","numberOfTRTHits","numberOfTRTOutliers","numberOfPrecisionLayers","d0","z0","vz","definingParametersCovMatrixDiag","definingParametersCovMatrixOffDiag","vertexLink","truthParticleLink","chiSquared","numberDoF","numberOfPhiLayers","numberOfPhiHoleLayers","numberOfPrecisionHoleLayers","truthType","truthOrigin","truthClassification"]

def MuonCPInDetFwdVarsCfg(flags):
  return ["theta","phi","qOverP","numberOfPrecisionLayers","numberOfPrecisionHoleLayers","numberOfPixelHits","numberOfPixelDeadSensors","numberOfSCTHits","numberOfSCTDeadSensors","d0","z0","vz","definingParametersCovMatrixDiag","definingParametersCovMatrixOffDiag","numberOfPixelHoles","numberOfSCTHoles","numberOfTRTHits","numberOfTRTOutliers","truthType","truthOrigin","truthClassification"]

def MuonCPElectronsVarsCfg(flags):
    ##we should validate asap if we can remove them
  return ["trackParticleLinks","pt","eta","phi","m","f1","topoetcone40","truthParticleLink","caloClusterLinks"]

def MuonCPPhotonsVarsCfg(flags):
    ##we should validate asap if we can remove them
  return ["pt","eta","phi","m","caloClusterLinks","author","f1","topoetcone40","Tight","truthParticleLink","vertexLink"]

def MuonSegmentVarsCfg(flags):
    return ["chamberIndex"]

def MuonCPContentCfg(flags):
    return [
        "InDetTrackParticles",
        "InDetTrackParticlesAux.{id_variables}".format(id_variables = ".".join(MuonCPInDetVarsCfg(flags))),
        "InDetForwardTrackParticles",
        "InDetForwardTrackParticlesAux.{fwdid_variables}".format(fwdid_variables = ".".join(MuonCPInDetFwdVarsCfg(flags))),
        "CombinedMuonTrackParticles",
        "CombinedMuonTrackParticlesAux.{cmb_variables}".format(cmb_variables = ".".join(CombinedTrackVarsCfg(flags))),
        "ExtrapolatedMuonTrackParticles",
        "ExtrapolatedMuonTrackParticlesAux.{me_variables}".format(me_variables = ".".join(MuonCPMETrkVarsCfg(flags))),
        "MuonSpectrometerTrackParticles",
        "MuonSpectrometerTrackParticlesAux.{ms_variables}".format(ms_variables = ".".join(MSTrkVarsCfg(flags))),
        "Muons",
        "MuonsAux.{muon_variables}".format(muon_variables = ".".join(MuonVariablesCfg(flags))),
        "InDetForwardTrackParticles",
        "InDetForwardTrackParticlesAux.{sifwd_variables}".format(sifwd_variables = ".".join(MuonCPInDetSiAssocVarsCfg(flags))),
        "MuonSegments",
        "MuonSegmentsAux.{seg_variables}".format(seg_variables = ".".join(MuonSegmentVarsCfg(flags))),
        "Electrons",
        "ElectronsAux.{el_variables}".format(el_variables = ".".join(MuonCPElectronsVarsCfg(flags))),
        "Photons",
        "PhotonsAux.{ph_variables}".format(ph_variables = ".".join(MuonCPPhotonsVarsCfg(flags))),
    ]

def MuonCPContentLRTCfg(flags):
    return [
        "InDetLargeD0TrackParticles",
        "InDetLargeD0TrackParticlesAux.{id_variables}".format(id_variables = ".".join(MuonCPInDetVarsCfg(flags))),
        "InDetForwardTrackParticles",
        "InDetForwardTrackParticlesAux.{fwdid_variables}".format(fwdid_variables = ".".join(MuonCPInDetFwdVarsCfg(flags))),
        "CombinedMuonsLRTTrackParticles",
        "CombinedMuonsLRTTrackParticlesAux.{cmb_variables}".format(cmb_variables = ".".join(CombinedTrackVarsCfg(flags))),
        "MuonsLRT",
        "MuonsLRTAux.{muon_variables}".format(muon_variables = ".".join(MuonVariablesCfg(flags))),
        "MuonSpectrometerTrackParticles",
        "MuonSpectrometerTrackParticlesAux.{ms_variables}".format(ms_variables = ".".join(MSTrkVarsCfg(flags))),
        "ExtraPolatedMuonsLRTTrackParticles",
        "ExtraPolatedMuonsLRTTrackParticlesAux.{me_variables}".format(me_variables = ".".join(MuonCPMETrkVarsCfg(flags))),
        "MuonSegments",
        "MuonSegmentsAux.{seg_variables}".format(seg_variables = ".".join(MuonSegmentVarsCfg(flags))),
        "LRTElectrons",
        "LRTElectronsAux.{el_variables}".format(el_variables = ".".join(MuonCPElectronsVarsCfg(flags))),
        "Photons",
        "PhotonsAux.{ph_variables}".format(ph_variables = ".".join(MuonCPPhotonsVarsCfg(flags))),
        "LRTegammaClustersAux.calEta.calPhi.calE.calM",
        "LRTegammaTopoSeededClusters",
        "LRTegammaTopoSeededClustersAux.calEta.calPhi",
    ]
