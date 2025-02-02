/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ComponentFactoryPreloader/ComponentFactoryPreloader.h>

#include <AsgTools/AsgComponentFactories.h>
#include <AsgTools/MessageCheckAsgTools.h>

#include <AsgAnalysisAlgorithms/AsgClassificationDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgCutBookkeeperAlg.h>
#include <AsgAnalysisAlgorithms/AsgEnergyDecoratorAlg.h>
#include <AsgAnalysisAlgorithms/AsgEventScaleFactorAlg.h>
#include <AsgAnalysisAlgorithms/AsgObjectScaleFactorAlg.h>
#include <AsgAnalysisAlgorithms/AsgFlagSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgLeptonTrackDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgLeptonTrackSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgMaskSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgMassSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgOriginalObjectLinkAlg.h>
#include <AsgAnalysisAlgorithms/AsgPriorityDecorationAlg.h>
#include <AsgAnalysisAlgorithms/AsgPtEtaSelectionTool.h>
#include <AsgAnalysisAlgorithms/AsgSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgShallowCopyAlg.h>
#include <AsgAnalysisAlgorithms/AsgUnionPreselectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgUnionSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgViewFromSelectionAlg.h>
#include <AsgAnalysisAlgorithms/AsgxAODMetNTupleMakerAlg.h>
#include <AsgAnalysisAlgorithms/AsgxAODNTupleMakerAlg.h>
#include <AsgAnalysisAlgorithms/BootstrapGeneratorAlg.h>
#include <AsgAnalysisAlgorithms/CopyNominalSelectionAlg.h>
#include <AsgAnalysisAlgorithms/EventCutFlowHistAlg.h>
#include <AsgAnalysisAlgorithms/EventDecoratorAlg.h>
#include <AsgAnalysisAlgorithms/EventFlagSelectionAlg.h>
#include <AsgAnalysisAlgorithms/EventSelectionByObjectFlagAlg.h>
#include <AsgAnalysisAlgorithms/EventStatusSelectionAlg.h>
#include <AsgAnalysisAlgorithms/FakeBkgCalculatorAlg.h>
#include <AsgAnalysisAlgorithms/IOStatsAlg.h>
#include <AsgAnalysisAlgorithms/KinematicHistAlg.h>
#include <AsgAnalysisAlgorithms/LeptonSFCalculatorAlg.h>
#include <AsgAnalysisAlgorithms/MCTCDecorationAlg.h>
#include <AsgAnalysisAlgorithms/ObjectCutFlowHistAlg.h>
#include <AsgAnalysisAlgorithms/OverlapRemovalAlg.h>
#include <AsgAnalysisAlgorithms/PMGTruthWeightAlg.h>
#include <AsgAnalysisAlgorithms/PileupReweightingAlg.h>
#include <AsgAnalysisAlgorithms/SysListDumperAlg.h>
#include <AsgAnalysisAlgorithms/SystObjectLinkerAlg.h>
#include <AsgAnalysisAlgorithms/TreeFillerAlg.h>
#include <AsgAnalysisAlgorithms/TreeMakerAlg.h>
#include <AsgAnalysisAlgorithms/VGammaORAlg.h>
#include <AssociationUtils/DeltaROverlapTool.h>
#include <AssociationUtils/EleJetOverlapTool.h>
#include <AssociationUtils/EleMuSharedTrkOverlapTool.h>
#include <AssociationUtils/MuJetOverlapTool.h>
#include <AssociationUtils/OverlapRemovalTool.h>
#include <DiTauMassTools/MissingMassTool.h>
#include <EgammaAnalysisAlgorithms/EgammaCalibrationAndSmearingAlg.h>
#include <EgammaAnalysisAlgorithms/EgammaCaloClusterEtaAlg.h>
#include <EgammaAnalysisAlgorithms/EgammaFSRForMuonsCollectorAlg.h>
#include <EgammaAnalysisAlgorithms/EgammaIsGoodOQSelectionTool.h>
#include <EgammaAnalysisAlgorithms/EgammaIsolationCorrectionAlg.h>
#include <EgammaAnalysisAlgorithms/EgammaIsolationSelectionAlg.h>
#include <EgammaAnalysisAlgorithms/ElectronEfficiencyCorrectionAlg.h>
#include <EgammaAnalysisAlgorithms/ElectronLRTMergingAlg.h>
#include <EgammaAnalysisAlgorithms/ElectronSiHitDecAlg.h>
#include <EgammaAnalysisAlgorithms/PhotonEfficiencyCorrectionAlg.h>
#include <EgammaAnalysisAlgorithms/PhotonExtraVariablesAlg.h>
#include <EgammaAnalysisAlgorithms/PhotonOriginCorrectionAlg.h>
#include <EgammaAnalysisAlgorithms/PhotonShowerShapeFudgeAlg.h>
#include <ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h>
#include <ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h>
#include <ElectronPhotonSelectorTools/AsgDeadHVCellRemovalTool.h>
#include <EventSelectionAlgorithms/ChargeSelectorAlg.h>
#include <EventSelectionAlgorithms/DileptonInvariantMassSelectorAlg.h>
#include <EventSelectionAlgorithms/DileptonInvariantMassWindowSelectorAlg.h>
#include <EventSelectionAlgorithms/DileptonOSSFInvariantMassWindowSelectorAlg.h>
#include <EventSelectionAlgorithms/JetNGhostSelectorAlg.h>
#include <EventSelectionAlgorithms/MissingETPlusTransverseMassSelectorAlg.h>
#include <EventSelectionAlgorithms/MissingETSelectorAlg.h>
#include <EventSelectionAlgorithms/NLargeRJetMassWindowSelectorAlg.h>
#include <EventSelectionAlgorithms/NObjectMassSelectorAlg.h>
#include <EventSelectionAlgorithms/NObjectPtSelectorAlg.h>
#include <EventSelectionAlgorithms/RunNumberSelectorAlg.h>
#include <EventSelectionAlgorithms/SaveFilterAlg.h>
#include <EventSelectionAlgorithms/SumNLeptonPtSelectorAlg.h>
#include <EventSelectionAlgorithms/TransverseMassSelectorAlg.h>
#include <FakeBkgTools/AsymptMatrixTool.h>
#include <FTagAnalysisAlgorithms/BTaggingEfficiencyAlg.h>
#include <FTagAnalysisAlgorithms/BTaggingInformationDecoratorAlg.h>
#include <FTagAnalysisAlgorithms/BTaggingScoresAlg.h>
#include <GammaORTools/VGammaORTool.h>
#include <GoodRunsLists/GRLSelectorAlg.h>
#include <GoodRunsLists/GoodRunsListSelectionTool.h>
#include <InDetSecVtxTruthMatchTool/InDetSecVtxTruthMatchTool.h>
#include <IsolationCorrections/IsolationCorrectionTool.h>
#include <IsolationSelection/IsolationSelectionTool.h>
#include <JetAnalysisAlgorithms/BJetCalibrationAlg.h>
#include <JetAnalysisAlgorithms/JetCalibrationAlg.h>
#include <JetAnalysisAlgorithms/JetDecoratorAlg.h>
#include <JetAnalysisAlgorithms/JetGhostMergingAlg.h>
#include <JetAnalysisAlgorithms/JetGhostMuonAssociationAlg.h>
#include <JetAnalysisAlgorithms/JetModifierAlg.h>
#include <JetAnalysisAlgorithms/JetReclusteringAlg.h>
#include <JetAnalysisAlgorithms/ReclusteredJetCalibrationAlg.h>
#include <JetAnalysisAlgorithms/JetSelectionAlg.h>
#include <JetAnalysisAlgorithms/JetTruthTagAlg.h>
#include <JetAnalysisAlgorithms/JetUncertaintiesAlg.h>
#include <JetAnalysisAlgorithms/JvtEfficiencyAlg.h>
#include <JetAnalysisAlgorithms/JvtUpdateAlg.h>
#include <JetCalibTools/JetCalibrationTool.h>
#include <JetCalibTools/BJetCorrectionTool.h>
#include <JetCalibTools/MuonInJetCorrectionTool.h>
#include <JetJvtEfficiency/JvtEfficiencyTool.h>
#include <JetJvtEfficiency/NNJvtSelectionTool.h>
#include <JetMomentTools/JetVertexNNTagger.h>
#include <JetUncertainties/JetUncertaintiesTool.h>
#include <LRTElectronAnalysisTools/ElectronLRTOverlapRemovalTool.h>
#include <LRTMuonAnalysisTools/MuonLRTOverlapRemovalTool.h>
#include <METUtilities/METMaker.h>
#include <METUtilities/METSignificance.h>
#include <METUtilities/METSystematicsTool.h>
#include <MetAnalysisAlgorithms/MetBuilderAlg.h>
#include <MetAnalysisAlgorithms/MetMakerAlg.h>
#include <MetAnalysisAlgorithms/MetSignificanceAlg.h>
#include <MuonAnalysisAlgorithms/MuonCalibrationAndSmearingAlg.h>
#include <MuonAnalysisAlgorithms/MuonEfficiencyScaleFactorAlg.h>
#include <MuonAnalysisAlgorithms/MuonIsolationAlg.h>
#include <MuonAnalysisAlgorithms/MuonLRTMergingAlg.h>
#include <MuonAnalysisAlgorithms/MuonSelectionAlg.h>
#include <MuonAnalysisAlgorithms/MuonTriggerEfficiencyScaleFactorAlg.h>
#include <MuonEfficiencyCorrections/MuonEfficiencyScaleFactors.h>
#include <MuonEfficiencyCorrections/MuonTriggerScaleFactors.h>
#include <MuonMomentumCorrections/MuonCalibIntSagittaTool.h>
#include <MuonMomentumCorrections/MuonCalibIntScaleSmearTool.h>
#include <MuonMomentumCorrections/MuonCalibTool.h>
#include <MuonSelectorTools/MuonSelectionTool.h>
#include <PileupReweighting/PileupReweightingTool.h>
#include <PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h>
#include <PMGTools/PMGTruthWeightTool.h>
#include <SelectionHelpers/SelectionNameSvc.h>
#include <StandaloneAnalysisAlgorithms/xAODWriterAlg.h>
#include <SystematicsHandles/SystematicsSvc.h>
#include <TauAnalysisAlgorithms/DiTauEfficiencyCorrectionsAlg.h>
#include <TauAnalysisAlgorithms/DiTauMassCalculatorAlg.h>
#include <TauAnalysisAlgorithms/DiTauSmearingAlg.h>
#include <TauAnalysisAlgorithms/DiTauTruthMatchingAlg.h>
#include <TauAnalysisAlgorithms/TauCombineMuonRMTausAlg.h>
#include <TauAnalysisAlgorithms/TauEfficiencyCorrectionsAlg.h>
#include <TauAnalysisAlgorithms/TauExtraVariablesAlg.h>
#include <TauAnalysisAlgorithms/TauSmearingAlg.h>
#include <TauAnalysisAlgorithms/TauTruthDecorationsAlg.h>
#include <TauAnalysisAlgorithms/TauTruthMatchingAlg.h>
#include <TauAnalysisTools/CommonSmearingTool.h>
#include <TauAnalysisTools/DiTauEfficiencyCorrectionsTool.h>
#include <TauAnalysisTools/DiTauSmearingTool.h>
#include <TauAnalysisTools/DiTauTruthMatchingTool.h>
#include <TauAnalysisTools/TauEfficiencyCorrectionsTool.h>
#include <TauAnalysisTools/TauSelectionTool.h>
#include <TauAnalysisTools/TauSmearingTool.h>
#include <TauAnalysisTools/TauTruthMatchingTool.h>
#include <TrackingAnalysisAlgorithms/SecVertexTruthMatchAlg.h>
#include <TrackingAnalysisAlgorithms/TrackParticleMergerAlg.h>
#include <TrackingAnalysisAlgorithms/VertexSelectionAlg.h>
#include <TrigConfxAOD/xAODConfigTool.h>
#include <TrigDecisionTool/TrigDecisionTool.h>
#include <TrigGlobalEfficiencyCorrection/TrigGlobalEfficiencyCorrectionTool.h>
#include <TriggerMatchingTool/MatchingTool.h>
#include <TriggerAnalysisAlgorithms/TrigEventSelectionAlg.h>
#include <TriggerAnalysisAlgorithms/TrigGlobalEfficiencyAlg.h>
#include <TriggerAnalysisAlgorithms/TrigPrescalesAlg.h>
#include <TriggerAnalysisAlgorithms/TrigMatchingAlg.h>
#include <TriggerMatchingTool/DRScoringTool.h>
#include <TriggerMatchingTool/MatchFromCompositeTool.h>
#include <TruthClassification/TruthClassificationTool.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelChargeDecoratorAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelIsolationAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelJetsAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelMissingETAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelOverlapRemovalAlg.h>
#include <TruthParticleLevelAnalysisAlgorithms/ParticleLevelPtEtaPhiDecoratorAlg.h>
#include <egammaMVACalib/egammaMVACalibTool.h>
#include <egammaMVACalib/egammaMVASvc.h>
#include <tauRecTools/TauCombinedTES.h>
#include <xAODBTaggingEfficiency/BTaggingEfficiencyTool.h>
#include <xAODBTaggingEfficiency/BTaggingSelectionTool.h>

//
// method implementations
//

namespace CP
{
  bool preloadComponentFactories ()
  {
    using namespace asg::msgComponentConfig;
    ANA_CHECK_SET_TYPE (bool);

    asg::msgComponentConfig::setMsgLevel (MSG::DEBUG);

    ANA_MSG_INFO ("preloading component factories");

    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgClassificationDecorationAlg>("CP::AsgClassificationDecorationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgCutBookkeeperAlg>("CP::AsgCutBookkeeperAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgEnergyDecoratorAlg>("CP::AsgEnergyDecoratorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgEventScaleFactorAlg>("CP::AsgEventScaleFactorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgObjectScaleFactorAlg>("CP::AsgObjectScaleFactorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgLeptonTrackDecorationAlg>("CP::AsgLeptonTrackDecorationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgLeptonTrackSelectionAlg>("CP::AsgLeptonTrackSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgOriginalObjectLinkAlg>("CP::AsgOriginalObjectLinkAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgPriorityDecorationAlg>("CP::AsgPriorityDecorationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgSelectionAlg>("CP::AsgSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgShallowCopyAlg>("CP::AsgShallowCopyAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgUnionPreselectionAlg>("CP::AsgUnionPreselectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgUnionSelectionAlg>("CP::AsgUnionSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgViewFromSelectionAlg>("CP::AsgViewFromSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgxAODMetNTupleMakerAlg>("CP::AsgxAODMetNTupleMakerAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::AsgxAODNTupleMakerAlg>("CP::AsgxAODNTupleMakerAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::BJetCalibrationAlg >("CP::BJetCalibrationAlg "));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::BTaggingEfficiencyAlg>("CP::BTaggingEfficiencyAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::BTaggingInformationDecoratorAlg>("CP::BTaggingInformationDecoratorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::BTaggingScoresAlg>("CP::BTaggingScoresAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::BootstrapGeneratorAlg>("CP::BootstrapGeneratorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ChargeSelectorAlg>("CP::ChargeSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::CopyNominalSelectionAlg>("CP::CopyNominalSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::DiTauEfficiencyCorrectionsAlg>("CP::DiTauEfficiencyCorrectionsAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::DiTauMassCalculatorAlg>("CP::DiTauMassCalculatorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::DiTauSmearingAlg>("CP::DiTauSmearingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::DiTauTruthMatchingAlg>("CP::DiTauTruthMatchingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::DileptonInvariantMassSelectorAlg>("CP::DileptonInvariantMassSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::DileptonInvariantMassWindowSelectorAlg>("CP::DileptonInvariantMassWindowSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::DileptonOSSFInvariantMassWindowSelectorAlg>("CP::DileptonOSSFInvariantMassWindowSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EgammaCalibrationAndSmearingAlg>("CP::EgammaCalibrationAndSmearingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EgammaCaloClusterEtaAlg>("CP::EgammaCaloClusterEtaAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EgammaFSRForMuonsCollectorAlg>("CP::EgammaFSRForMuonsCollectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EgammaIsolationCorrectionAlg>("CP::EgammaIsolationCorrectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EgammaIsolationSelectionAlg>("CP::EgammaIsolationSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ElectronEfficiencyCorrectionAlg>("CP::ElectronEfficiencyCorrectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ElectronLRTMergingAlg>("CP::ElectronLRTMergingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ElectronSiHitDecAlg>("CP::ElectronSiHitDecAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EventCutFlowHistAlg>("CP::EventCutFlowHistAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EventDecoratorAlg>("CP::EventDecoratorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EventFlagSelectionAlg>("CP::EventFlagSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EventSelectionByObjectFlagAlg>("CP::EventSelectionByObjectFlagAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::EventStatusSelectionAlg>("CP::EventStatusSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::FakeBkgCalculatorAlg>("CP::FakeBkgCalculatorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<GRLSelectorAlg>("GRLSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::IOStatsAlg>("CP::IOStatsAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetCalibrationAlg>("CP::JetCalibrationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetDecoratorAlg>("CP::JetDecoratorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetGhostMergingAlg>("CP::JetGhostMergingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetGhostMuonAssociationAlg>("CP::JetGhostMuonAssociationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetModifierAlg>("CP::JetModifierAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetNGhostSelectorAlg>("CP::JetNGhostSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetReclusteringAlg>("CP::JetReclusteringAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ReclusteredJetCalibrationAlg>("CP::ReclusteredJetCalibrationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetSelectionAlg>("CP::JetSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetTruthTagAlg>("CP::JetTruthTagAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JetUncertaintiesAlg>("CP::JetUncertaintiesAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JvtEfficiencyAlg>("CP::JvtEfficiencyAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::JvtUpdateAlg>("CP::JvtUpdateAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::KinematicHistAlg>("CP::KinematicHistAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::LeptonSFCalculatorAlg>("CP::LeptonSFCalculatorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MCTCDecorationAlg>("CP::MCTCDecorationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MetBuilderAlg>("CP::MetBuilderAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MetMakerAlg>("CP::MetMakerAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MetSignificanceAlg>("CP::MetSignificanceAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MissingETPlusTransverseMassSelectorAlg>("CP::MissingETPlusTransverseMassSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MissingETSelectorAlg>("CP::MissingETSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MuonCalibrationAndSmearingAlg>("CP::MuonCalibrationAndSmearingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MuonEfficiencyScaleFactorAlg>("CP::MuonEfficiencyScaleFactorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MuonIsolationAlg>("CP::MuonIsolationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MuonLRTMergingAlg>("CP::MuonLRTMergingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MuonSelectionAlgV2>("CP::MuonSelectionAlgV2"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::MuonTriggerEfficiencyScaleFactorAlg>("CP::MuonTriggerEfficiencyScaleFactorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::NLargeRJetMassWindowSelectorAlg>("CP::NLargeRJetMassWindowSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::NObjectMassSelectorAlg>("CP::NObjectMassSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::NObjectPtSelectorAlg>("CP::NObjectPtSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ObjectCutFlowHistAlg>("CP::ObjectCutFlowHistAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::OverlapRemovalAlg>("CP::OverlapRemovalAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::PhotonEfficiencyCorrectionAlg>("CP::PhotonEfficiencyCorrectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::PhotonExtraVariablesAlg>("CP::PhotonExtraVariablesAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::PhotonOriginCorrectionAlg>("CP::PhotonOriginCorrectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::PhotonShowerShapeFudgeAlg>("CP::PhotonShowerShapeFudgeAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::PileupReweightingAlg>("CP::PileupReweightingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::RunNumberSelectorAlg>("CP::RunNumberSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::SaveFilterAlg>("CP::SaveFilterAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::SecVertexTruthMatchAlg>("CP::SecVertexTruthMatchAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::SumNLeptonPtSelectorAlg>("CP::SumNLeptonPtSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::SystObjectLinkerAlg>("CP::SystObjectLinkerAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::SysListDumperAlg>("CP::SysListDumperAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TauCombineMuonRMTausAlg>("CP::TauCombineMuonRMTausAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TauEfficiencyCorrectionsAlg>("CP::TauEfficiencyCorrectionsAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TauExtraVariablesAlg>("CP::TauExtraVariablesAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TauSmearingAlg>("CP::TauSmearingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TauTruthDecorationsAlg>("CP::TauTruthDecorationsAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TauTruthMatchingAlg>("CP::TauTruthMatchingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TrackParticleMergerAlg>("CP::TrackParticleMergerAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TransverseMassSelectorAlg>("CP::TransverseMassSelectorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TreeFillerAlg>("CP::TreeFillerAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TreeMakerAlg>("CP::TreeMakerAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TrigEventSelectionAlg>("CP::TrigEventSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TrigGlobalEfficiencyAlg>("CP::TrigGlobalEfficiencyAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TrigPrescalesAlg>("CP::TrigPrescalesAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::TrigMatchingAlg>("CP::TrigMatchingAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ParticleLevelChargeDecoratorAlg>("CP::ParticleLevelChargeDecoratorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ParticleLevelIsolationAlg>("CP::ParticleLevelIsolationAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ParticleLevelJetsAlg>("CP::ParticleLevelJetsAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ParticleLevelMissingETAlg>("CP::ParticleLevelMissingETAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ParticleLevelOverlapRemovalAlg>("CP::ParticleLevelOverlapRemovalAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::ParticleLevelPtEtaPhiDecoratorAlg>("CP::ParticleLevelPtEtaPhiDecoratorAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::PMGTruthWeightAlg>("CP::PMGTruthWeightAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::VertexSelectionAlg>("CP::VertexSelectionAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::VGammaORAlg>("CP::VGammaORAlg"));
    ANA_CHECK (asg::registerAlgorithmFactory<CP::xAODWriterAlg>("CP::xAODWriterAlg"));

    ANA_CHECK (asg::registerToolFactory<AsgDeadHVCellRemovalTool> ("AsgDeadHVCellRemovalTool"));
    ANA_CHECK (asg::registerToolFactory<AsgElectronEfficiencyCorrectionTool> ("AsgElectronEfficiencyCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<AsgPhotonEfficiencyCorrectionTool> ("AsgPhotonEfficiencyCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<BTaggingEfficiencyTool> ("BTaggingEfficiencyTool"));
    ANA_CHECK (asg::registerToolFactory<BTaggingSelectionTool> ("BTaggingSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::AsgFlagSelectionTool> ("CP::AsgFlagSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::AsgMaskSelectionTool> ("CP::AsgMaskSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::AsgMassSelectionTool> ("CP::AsgMassSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::AsgPtEtaSelectionTool> ("CP::AsgPtEtaSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::AsymptMatrixTool> ("CP::AsymptMatrixTool"));
    ANA_CHECK (asg::registerToolFactory<CP::EgammaCalibrationAndSmearingTool> ("CP::EgammaCalibrationAndSmearingTool"));
    ANA_CHECK (asg::registerToolFactory<CP::EgammaIsGoodOQSelectionTool> ("CP::EgammaIsGoodOQSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::ElectronLRTOverlapRemovalTool> ("CP::ElectronLRTOverlapRemovalTool"));
    ANA_CHECK (asg::registerToolFactory<CP::IsolationCorrectionTool> ("CP::IsolationCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::IsolationSelectionTool> ("CP::IsolationSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::JvtEfficiencyTool> ("CP::JvtEfficiencyTool"));
    ANA_CHECK (asg::registerToolFactory<CP::MuonCalibIntSagittaTool> ("CP::MuonCalibIntSagittaTool"));
    ANA_CHECK (asg::registerToolFactory<CP::MuonCalibIntScaleSmearTool> ("CP::MuonCalibIntScaleSmearTool"));
    ANA_CHECK (asg::registerToolFactory<CP::MuonCalibTool> ("CP::MuonCalibTool"));
    ANA_CHECK (asg::registerToolFactory<CP::MuonLRTOverlapRemovalTool> ("CP::MuonLRTOverlapRemovalTool"));
    ANA_CHECK (asg::registerToolFactory<CP::MuonSelectionTool> ("CP::MuonSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::MuonEfficiencyScaleFactors> ("CP::MuonEfficiencyScaleFactors"));
    ANA_CHECK (asg::registerToolFactory<CP::MuonTriggerScaleFactors> ("CP::MuonTriggerScaleFactors"));
    ANA_CHECK (asg::registerToolFactory<CP::NNJvtSelectionTool> ("CP::NNJvtSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::PileupReweightingTool> ("CP::PileupReweightingTool"));
    ANA_CHECK (asg::registerToolFactory<DiTauMassTools::MissingMassTool> ("DiTauMassTools::MissingMassTool"));
    ANA_CHECK (asg::registerToolFactory<GoodRunsListSelectionTool> ("GoodRunsListSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<InDetSecVtxTruthMatchTool> ("InDetSecVtxTruthMatchTool"));
    ANA_CHECK (asg::registerToolFactory<JetCalibrationTool> ("JetCalibrationTool"));
    ANA_CHECK (asg::registerToolFactory<BJetCorrectionTool> ("BJetCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<MuonInJetCorrectionTool> ("MuonInJetCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<JetPileupTag::JetVertexNNTagger> ("JetPileupTag::JetVertexNNTagger"));
    ANA_CHECK (asg::registerToolFactory<JetUncertaintiesTool> ("JetUncertaintiesTool"));
    ANA_CHECK (asg::registerToolFactory<ORUtils::DeltaROverlapTool> ("ORUtils::DeltaROverlapTool"));
    ANA_CHECK (asg::registerToolFactory<ORUtils::EleJetOverlapTool> ("ORUtils::EleJetOverlapTool"));
    ANA_CHECK (asg::registerToolFactory<ORUtils::EleMuSharedTrkOverlapTool> ("ORUtils::EleMuSharedTrkOverlapTool"));
    ANA_CHECK (asg::registerToolFactory<ORUtils::MuJetOverlapTool> ("ORUtils::MuJetOverlapTool"));
    ANA_CHECK (asg::registerToolFactory<ORUtils::OverlapRemovalTool> ("ORUtils::OverlapRemovalTool"));
    ANA_CHECK (asg::registerToolFactory<PMGTools::PMGTruthWeightTool> ("PMGTools::PMGTruthWeightTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::CommonSmearingTool> ("TauAnalysisTools::CommonSmearingTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::DiTauEfficiencyCorrectionsTool> ("TauAnalysisTools::DiTauEfficiencyCorrectionsTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::DiTauSmearingTool> ("TauAnalysisTools::DiTauSmearingTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::DiTauTruthMatchingTool> ("TauAnalysisTools::DiTauTruthMatchingTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::TauEfficiencyCorrectionsTool> ("TauAnalysisTools::TauEfficiencyCorrectionsTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::TauSelectionTool> ("TauAnalysisTools::TauSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::TauSmearingTool> ("TauAnalysisTools::TauSmearingTool"));
    ANA_CHECK (asg::registerToolFactory<TauAnalysisTools::TauTruthMatchingTool> ("TauAnalysisTools::TauTruthMatchingTool"));
    ANA_CHECK (asg::registerToolFactory<TauCombinedTES> ("TauCombinedTES"));
    ANA_CHECK (asg::registerToolFactory<Trig::DRScoringTool> ("Trig::DRScoringTool"));
    ANA_CHECK (asg::registerToolFactory<Trig::MatchFromCompositeTool> ("Trig::MatchFromCompositeTool"));
    ANA_CHECK (asg::registerToolFactory<Trig::TrigDecisionTool> ("Trig::TrigDecisionTool"));
    ANA_CHECK (asg::registerToolFactory<Trig::MatchingTool> ("Trig::MatchingTool"));
    ANA_CHECK (asg::registerToolFactory<TrigConf::xAODConfigTool> ("TrigConf::xAODConfigTool"));
    ANA_CHECK (asg::registerToolFactory<TrigGlobalEfficiencyCorrectionTool> ("TrigGlobalEfficiencyCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<TruthClassificationTool> ("TruthClassificationTool"));
    ANA_CHECK (asg::registerToolFactory<VGammaORTool> ("VGammaORTool"));
    ANA_CHECK (asg::registerToolFactory<egammaMVACalibTool> ("egammaMVACalibTool"));
    ANA_CHECK (asg::registerToolFactory<met::METMaker> ("met::METMaker"));
    ANA_CHECK (asg::registerToolFactory<met::METSignificance> ("met::METSignificance"));
    ANA_CHECK (asg::registerToolFactory<met::METSystematicsTool> ("met::METSystematicsTool"));

    ANA_CHECK (asg::registerServiceFactory<egammaMVASvc> ("egammaMVASvc"));
    ANA_CHECK (asg::registerServiceFactory<CP::SystematicsSvc> ("CP::SystematicsSvc"));
    ANA_CHECK (asg::registerServiceFactory<CP::SelectionNameSvc> ("CP::SelectionNameSvc"));

    return true;
  }
}
