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
#include <mutex>

#include <AssociationUtils/DeltaROverlapTool.h>
#include <AssociationUtils/EleJetOverlapTool.h>
#include <AssociationUtils/EleMuSharedTrkOverlapTool.h>
#include <AssociationUtils/MuJetOverlapTool.h>
#include <AssociationUtils/OverlapRemovalTool.h>
#include <DiTauMassTools/MissingMassTool.h>
#include <ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h>
#include <ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h>
#include <ElectronPhotonSelectorTools/AsgDeadHVCellRemovalTool.h>
#include <FakeBkgTools/AsymptMatrixTool.h>
#include <GammaORTools/VGammaORTool.h>
#include <GoodRunsLists/GRLSelectorAlg.h>
#include <GoodRunsLists/GoodRunsListSelectionTool.h>
#include <InDetSecVtxTruthMatchTool/InDetSecVtxTruthMatchTool.h>
#include <IsolationCorrections/IsolationCorrectionTool.h>
#include <IsolationSelection/IsolationSelectionTool.h>
#include <JetCalibTools/JetCalibrationTool.h>
#include <JetCalibTools/BJetCorrectionTool.h>
#include <JetCalibTools/MuonInJetCorrectionTool.h>
#include <JetJvtEfficiency/JvtEfficiencyTool.h>
#include <JetJvtEfficiency/NNJvtSelectionTool.h>
#include <JetMomentTools/JetVertexNNTagger.h>
#include <JetUncertainties/JetUncertaintiesTool.h>
#include <JetUncertainties/FFJetSmearingTool.h>
#include <LRTElectronAnalysisTools/ElectronLRTOverlapRemovalTool.h>
#include <LRTMuonAnalysisTools/MuonLRTOverlapRemovalTool.h>
#include <METUtilities/METMaker.h>
#include <METUtilities/METSignificance.h>
#include <METUtilities/METSystematicsTool.h>
#include <MuonEfficiencyCorrections/MuonEfficiencyScaleFactors.h>
#include <MuonEfficiencyCorrections/MuonTriggerScaleFactors.h>
#include <MuonMomentumCorrections/MuonCalibIntSagittaTool.h>
#include <MuonMomentumCorrections/MuonCalibIntScaleSmearTool.h>
#include <MuonMomentumCorrections/MuonCalibTool.h>
#include <MuonSelectorTools/MuonSelectionTool.h>
#include <PileupReweighting/PileupReweightingTool.h>
#include <PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h>
#include <PMGTools/PMGTruthWeightTool.h>
#include <TauAnalysisTools/CommonSmearingTool.h>
#include <TauAnalysisTools/DiTauEfficiencyCorrectionsTool.h>
#include <TauAnalysisTools/DiTauSmearingTool.h>
#include <TauAnalysisTools/DiTauTruthMatchingTool.h>
#include <TauAnalysisTools/TauEfficiencyCorrectionsTool.h>
#include <TauAnalysisTools/TauSelectionTool.h>
#include <TauAnalysisTools/TauSmearingTool.h>
#include <TauAnalysisTools/TauTruthMatchingTool.h>
#include <TrigConfxAOD/xAODConfigTool.h>
#include <TrigDecisionTool/TrigDecisionTool.h>
#include <TrigGlobalEfficiencyCorrection/TrigGlobalEfficiencyCorrectionTool.h>
#include <TriggerMatchingTool/MatchingTool.h>
#include <TriggerMatchingTool/DRScoringTool.h>
#include <TriggerMatchingTool/MatchFromCompositeTool.h>
#include <TruthClassification/TruthClassificationTool.h>
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
  // this function gets called once by the function below.  calling it
  // once avoids any errors if setup happens multiple times, e.g. in
  // test fixtures
  static bool doPreloadComponentFactories ()
  {
    using namespace asg::msgComponentConfig;
    ANA_CHECK_SET_TYPE (bool);

    // uncomment this if you want to see detailed messages about
    // component factories and configuration
    // asg::msgComponentConfig::setMsgLevel (MSG::DEBUG);

    ANA_MSG_INFO ("preloading component factories");

    ANA_CHECK (asg::registerAlgorithmFactory<GRLSelectorAlg>("GRLSelectorAlg"));

    ANA_CHECK (asg::registerToolFactory<AsgDeadHVCellRemovalTool> ("AsgDeadHVCellRemovalTool"));
    ANA_CHECK (asg::registerToolFactory<AsgElectronEfficiencyCorrectionTool> ("AsgElectronEfficiencyCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<AsgPhotonEfficiencyCorrectionTool> ("AsgPhotonEfficiencyCorrectionTool"));
    ANA_CHECK (asg::registerToolFactory<BTaggingEfficiencyTool> ("BTaggingEfficiencyTool"));
    ANA_CHECK (asg::registerToolFactory<BTaggingSelectionTool> ("BTaggingSelectionTool"));
    ANA_CHECK (asg::registerToolFactory<CP::AsymptMatrixTool> ("CP::AsymptMatrixTool"));
    ANA_CHECK (asg::registerToolFactory<CP::EgammaCalibrationAndSmearingTool> ("CP::EgammaCalibrationAndSmearingTool"));
    ANA_CHECK (asg::registerToolFactory<CP::ElectronLRTOverlapRemovalTool> ("CP::ElectronLRTOverlapRemovalTool"));
    ANA_CHECK (asg::registerToolFactory<CP::FFJetSmearingTool> ("CP::FFJetSmearingTool"));
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

    return true;
  }

  bool preloadComponentFactories ()
  {
    static bool result = false;
    static std::once_flag flag;
    std::call_once (flag, [&] () { result = doPreloadComponentFactories (); });
    return result;
  }
}
