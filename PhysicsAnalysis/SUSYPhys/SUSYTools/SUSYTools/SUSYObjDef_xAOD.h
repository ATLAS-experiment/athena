/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// This file's extension implies that it's C, but it's really -*- C++ -*-.
// $Id$
/**
 * @file SUSYTools/SUSYTools/SUSYObjDef.h
 * @author Renaud Bruneliere <Renaud.Bruneliere@cern.ch>
 * @date Feb, 2011
 * @brief Baseline SUSY object definitions.
 */

#ifndef SUSYTOOLS_SUSYOBJDEF_XAOD_H
#define SUSYTOOLS_SUSYOBJDEF_XAOD_H

// Framework include(s) - base class
#include "AsgTools/AsgMetadataTool.h"

// Interface class
#include "SUSYTools/ISUSYObjDef_xAODTool.h"

// Includes for systematics
#include "AsgMessaging/StatusCode.h"
#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicVariation.h"

// Tool handles
#include "AsgTools/AnaToolHandle.h"
#include "MuonAnalysisInterfaces/IMuonLRTOverlapRemovalTool.h"
#include "EgammaAnalysisInterfaces/IElectronLRTOverlapRemovalTool.h"

// Container read handle
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "CxxUtils/checker_macros.h"

// Configuration
#include "TEnv.h"

// Map for config file names
#include <map>
// Set for properties
#include <set>
// Various uses in function arguments and parameters
#include <string>
#include <vector>
#include <mutex>

// Tool interfaces
// Toolbox, which holds the tools
//#include "AssociationUtils/ORToolBox.h"
#include "AssociationUtils/ToolBox.h"
#include "BoostedJetTaggers/SmoothedWZTagger.h"
#include "BoostedJetTaggers/JSSWTopTaggerDNN.h"
#include "ParticleJetTools/JetTruthLabelingTool.h"
#include "ParticleJetTools/JetPileupLabelingTool.h"
#include "JetMomentTools/JetVertexTaggerTool.h"
#include "JetMomentTools/JetVertexNNTagger.h"
#include "JetAnalysisInterfaces/IJvtEfficiencyTool.h"
#include "PATCore/IAsgSelectionTool.h"
#include "JetCPInterfaces/ICPJetCorrectionTool.h"

#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"

class IJetCalibrationTool;
class IJERTool;
class IJERSmearingTool;
class ICPJetUncertaintiesTool;
class IJetSelector;
class IJetModifier;

class IAsgElectronLikelihoodTool;
class IEGammaAmbiguityTool;

class IAsgDeadHVCellRemovalTool;
class IAsgPhotonIsEMSelector;
class IElectronPhotonShowerShapeFudgeTool;

class IBTaggingEfficiencyTool;
class IBTaggingSelectionTool;

class IMETMaker;
class IMETSystematicsTool;
class IMETSignificance;

class ITrigGlobalEfficiencyCorrectionTool;

namespace CP {
  class IMuonSelectionTool;
  class IMuonCalibrationAndSmearingTool;
  class IMuonEfficiencyScaleFactors;

  class IEgammaCalibrationAndSmearingTool;
  class IEfficiencyScaleFactorTool;
  class IIsolationSelectionTool;
  class IIsolationLowPtPLVTool;
  class IIsolationCloseByCorrectionTool;
  class IIsolationCorrectionTool;
  class IPileupReweightingTool;
  class IJvtEfficiencyTool;
}

namespace InDet {
  class IInclusiveTrackFilterTool;
}

namespace TauAnalysisTools {
  class ITauSelectionTool;
  class ITauSmearingTool;
  class ITauEfficiencyCorrectionsTool;
  class ITauTruthMatchingTool;
}

class ITauToolBase;

namespace ORUtils {
  class IOverlapTool;
}

namespace TrigConf {
  class ITrigConfigTool;
}
namespace Trig {
  // Need the TrigDecisionTool directly for getChainGroup, features, and GetPreScale
  class TrigDecisionTool;
  class IMatchingTool;
  class IMatchScoringTool;
  class FeatureContainer;
}

class IWeightTool;

namespace ST {


  //  class SUSYObjDef;
  class SUSYObjDef_xAOD : public virtual ISUSYObjDef_xAODTool,
			  public asg::AsgMetadataTool {

    // Create a proper constructor for Athena
    ASG_TOOL_CLASS( SUSYObjDef_xAOD, ST::ISUSYObjDef_xAODTool)

    public:
    SUSYObjDef_xAOD( const std::string& name );
    ~SUSYObjDef_xAOD();

    // Function initialising the tool
    StatusCode initialize() override final;
    // NB that there is no function for finalising the tool.
    //  An IAsgTool does not have a finalize method, so we can
    //  only override finalize in athena.  To clean up, delete me.

    bool isData() const override final {return m_dataSource == Data;}
    bool isAtlfast() const override final {return m_dataSource == AtlfastII;}

    StatusCode setBoolProperty(const std::string& name, const bool& property) override final;

    // Hack to make thisx configurable from python
    // 0 = data, 1 = full sim, 2 = AF-II
    void setDataSource(int source);

    // Apply the correction on a modifyable object
    StatusCode FillJet(xAOD::Jet& input, const bool doCalib = true, bool isFat = false, bool doLargeRdecorations = false) override final;
    StatusCode FillTrackJet(xAOD::Jet& input) override final;
    StatusCode FillTau(xAOD::TauJet& input) override final;
    StatusCode FillMuon(xAOD::Muon& input, const float ptcut, const float etacut) override final;
    StatusCode FillElectron(xAOD::Electron& input, const float etcut, const float etacut) override final;
    StatusCode FillPhoton(xAOD::Photon& input, const float ptcut, const float etacut) override final;

    const xAOD::Vertex* GetPrimVtx() const override final;

    StatusCode GetJets(xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = true, const std::string& jetkey = "", const xAOD::JetContainer* containerToBeCopied = nullptr) override final;
    StatusCode GetTrackJets(xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = true, const std::string& jetkey = "", const xAOD::JetContainer* containerToBeCopied = nullptr) override final;
    StatusCode GetJetsSyst(const xAOD::JetContainer& calibjets, xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = true, const std::string& jetkey = "") override final;
    StatusCode GetFatJets(xAOD::JetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = false, const std::string& jetkey = "", const bool doLargeRdecorations = false, const xAOD::JetContainer* containerToBeCopied = nullptr) override final;
    StatusCode GetTaus(xAOD::TauJetContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = true, const std::string& taukey = "TauJets", const xAOD::TauJetContainer* containerToBeCopied = nullptr) override final;
    StatusCode GetMuons(xAOD::MuonContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = true, const std::string& muonkey = "Muons", const std::string& lrtmuonkey = "MuonsLRT", const xAOD::MuonContainer* containerToBeCopied = nullptr) override final;
    StatusCode GetElectrons(xAOD::ElectronContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = true, const std::string& elekey = "Electrons", const std::string& lrtelekey = "LRTElectrons", const xAOD::ElectronContainer* containerToBeCopied = nullptr) override final;
    StatusCode GetPhotons(xAOD::PhotonContainer*& copy, xAOD::ShallowAuxContainer*& copyaux, const bool recordSG = true, const std::string& photonkey = "Photons", const xAOD::PhotonContainer* containerToBeCopied = nullptr) override final;
    StatusCode GetMET(xAOD::MissingETContainer& met,
                      const xAOD::JetContainer* jet,
                      const xAOD::ElectronContainer* elec = nullptr,
                      const xAOD::MuonContainer* muon = nullptr,
                      const xAOD::PhotonContainer* gamma = nullptr,
                      const xAOD::TauJetContainer* taujet = nullptr,
                      bool doTST = true, bool doJVTCut = true,
                      const xAOD::IParticleContainer* invis = nullptr) override final;

    StatusCode GetTrackMET(xAOD::MissingETContainer& met,
                           const xAOD::JetContainer* jet,
                           const xAOD::ElectronContainer* elec = nullptr,
                           const xAOD::MuonContainer* muon = nullptr
			   ) override final;

    StatusCode GetMETSig(xAOD::MissingETContainer& met,
                         double& metSignificance,
                         bool doTST = true, bool doJVTCut = true
		         ) override final;

    StatusCode MergeMuons(const  xAOD::MuonContainer & muons, const std::vector<bool> &writeMuon, xAOD::MuonContainer* outputCol) const override final;
    const xAOD::MuonContainer* prompt_muons = nullptr;
    const xAOD::MuonContainer* lrt_muons = nullptr;

    StatusCode prepareLRTMuons(const xAOD::MuonContainer* inMuons, xAOD::MuonContainer* copy) const override final;

    StatusCode MergeElectrons(const  xAOD::ElectronContainer & electrons, xAOD::ElectronContainer* outputCol, const std::set<const xAOD::Electron *> &ElectronsToRemove) const override final;
    const xAOD::ElectronContainer* prompt_electrons = nullptr;
    const xAOD::ElectronContainer* lrt_electrons = nullptr;

    StatusCode prepareLRTElectrons(const xAOD::ElectronContainer* inMuons, xAOD::ElectronContainer* copy) const override final;

    StatusCode SetBtagWeightDecorations(const xAOD::Jet& input, const asg::AnaToolHandle<IBTaggingSelectionTool>& btagSelTool, const std::string& btagTagger) const override final;

    bool IsSignalJet(const xAOD::Jet& input, const float ptcut, const float etacut) const override final;

    bool IsBadJet(const xAOD::Jet& input) const override final;

    bool IsBJetLoose(const xAOD::Jet& input) const override final;
    bool JetPassJVT(xAOD::Jet& input) override final;


    bool IsSignalMuon(const xAOD::Muon& input, const float ptcut, const float d0sigcut, const float z0cut, const float etacut = DUMMYDEF) const override final;

    bool IsSignalElectron(const xAOD::Electron& input, const float etcut, const float d0sigcut, const float z0cut, const float etacut = DUMMYDEF) const override final;

    bool IsCosmicMuon(const xAOD::Muon& input, const float z0cut, const float d0cut) const override final;

    bool IsHighPtMuon(const xAOD::Muon& input) const override final;
    // See https://indico.cern.ch/event/371499/contribution/1/material/slides/0.pdf and https://indico.cern.ch/event/397325/contribution/19/material/slides/0.pdf and https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionTool

    bool IsSignalTau(const xAOD::TauJet& input, const float ptcut, const float etacut) const override final;

    bool IsBadMuon(const xAOD::Muon& input, const float qopcut) const override final;

    bool IsSignalPhoton(const xAOD::Photon& input, const float ptcut, const float etacut = DUMMYDEF) const override final;

    bool IsBJet(const xAOD::Jet& input) const override final;

    bool IsTrackBJet(const xAOD::Jet& input) const override final;

    bool IsTruthBJet(const xAOD::Jet& input) const override final;

    int IsBJetContinuous(const xAOD::Jet& input) const override final;

    int IsTrackBJetContinuous(const xAOD::Jet& input) const override final;

    float BtagSF(const xAOD::JetContainer* jets) override final;

    float BtagSFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig) override final;

    float BtagSF_trkJet(const xAOD::JetContainer* trkjets) override final;

    float BtagSFsys_trkJet(const xAOD::JetContainer* trkjets, const CP::SystematicSet& systConfig) override final;

    double JVT_SF(const xAOD::JetContainer* jets) override final;

    double JVT_SFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig) override final;

    double FJVT_SF(const xAOD::JetContainer* jets) override final;

    double FJVT_SFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig) override final;

    //SF helpers
    //muons
    float GetSignalMuonSF(const xAOD::Muon& mu, const bool recoSF = true, const bool isoSF = true, const bool doBadMuonHP = true, const bool warnOVR = true) override final;

    double GetMuonTriggerEfficiency(const xAOD::Muon& mu, const std::string& trigExpr, const bool isdata = false ) override final;

    double GetTotalMuonTriggerSF(const xAOD::MuonContainer& sfmuons, const std::string& trigExpr) override final;

    double GetTotalMuonSF(const xAOD::MuonContainer& muons, const bool recoSF = true, const bool isoSF = true, const std::string& trigExpr="HLT_mu20_iloose_L1MU15_OR_HLT_mu50", const bool bmhptSF = true) override final;

    double GetTotalMuonSFsys(const xAOD::MuonContainer& muons, const CP::SystematicSet& systConfig, const bool recoSF = true, const bool isoSF = true, const std::string& trigExpr = "HLT_mu20_iloose_L1MU15_OR_HLT_mu50", const bool bmhptSF = true) override final;

    //electrons
    float GetSignalElecSF(const xAOD::Electron& el, const bool recoSF = true, const bool idSF = true, const bool triggerSF = true, const bool isoSF = true, const std::string& trigExpr = "singleLepton", const bool ecidsSF = false, const bool cidSF = false) override final;

    double GetEleTriggerEfficiency(const xAOD::Electron& el, const std::string& trigExpr = "SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0") const override final;

    double GetTriggerGlobalEfficiency(const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons, const std::string& trigExpr = "diLepton") override final;

    double GetTriggerGlobalEfficiency(const xAOD::PhotonContainer& photons, const std::string& trigExpr = "diPhoton") override final;

    double GetEleTriggerEfficiencySF(const xAOD::Electron& el, const std::string& trigExpr = "SINGLE_E_2015_e24_lhmedium_L1EM20VH_OR_e60_lhmedium_OR_e120_lhloose_2016_2018_e26_lhtight_nod0_ivarloose_OR_e60_lhmedium_nod0_OR_e140_lhloose_nod0") const override final;

    double GetTriggerGlobalEfficiencySF(const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons, const std::string& trigExpr = "diLepton") override final;

    double GetTriggerGlobalEfficiencySF(const xAOD::PhotonContainer& photons, const std::string& trigExpr = "diPhoton") override final;

    double GetTriggerGlobalEfficiencySFsys(const xAOD::ElectronContainer& electrons, const xAOD::MuonContainer& muons, const CP::SystematicSet& systConfig, const std::string& trigExpr = "diLepton") override final;

    double GetTriggerGlobalEfficiencySFsys(const xAOD::PhotonContainer& photons, const CP::SystematicSet& systConfig, const std::string& trigExpr = "diPhoton") override final;

    float GetTotalElectronSF(const xAOD::ElectronContainer& electrons, const bool recoSF = true, const bool idSF = true, const bool triggerSF = true, const bool isoSF = true, const std::string& trigExpr = "singleLepton", const bool ecidsSF = false, const bool cidSF = false) override final; // singleLepton == Ele.TriggerSFStringSingle value

    float GetTotalElectronSFsys(const xAOD::ElectronContainer& electrons, const CP::SystematicSet& systConfig, const bool recoSF = true, const bool idSF = true, const bool triggerSF = true, const bool isoSF = true, const std::string& trigExpr = "singleLepton", const bool ecidsSF = false, const bool cidSF = false) override final; // singleLepton == Ele.TriggerSFStringSingle value

    //taus
    double GetSignalTauSF(const xAOD::TauJet& tau, const bool idSF = true, const bool triggerSF = true, const std::string& trigExpr = "tau25_medium1_tracktwo") override final;

    double GetSignalTauSFsys(const xAOD::TauJet& tau, const CP::SystematicSet& systConfig, const bool idSF = true, const bool triggerSF = true, const std::string& trigExpr = "tau25_medium1_tracktwo") override final;

    double GetTauTriggerEfficiencySF(const xAOD::TauJet& tau, const std::string& trigExpr = "tau25_medium1_tracktwo") override final;

    double GetTotalTauSF(const xAOD::TauJetContainer& taus, const bool idSF = true, const bool triggerSF = true,  const std::string& trigExpr = "tau25_medium1_tracktwo") override final;

    double GetTotalTauSFsys(const xAOD::TauJetContainer& taus, const CP::SystematicSet& systConfig, const bool idSF = true, const bool triggerSF = true, const std::string& trigExpr = "tau25_medium1_tracktwo") override final;

    //photons
    double GetSignalPhotonSF(const xAOD::Photon& ph, const bool effSF = true, const bool isoSF = true, const bool triggerSF = false) const override final;

    double GetSignalPhotonSFsys(const xAOD::Photon& ph, const CP::SystematicSet& systConfig, const bool effSF = true, const bool isoSF = true, const bool triggerSF = false) override final;

    double GetTotalPhotonSF(const xAOD::PhotonContainer& photons, const bool effSF = true, const bool isoSF = true, const bool triggerSF = false) const override final;

    double GetTotalPhotonSFsys(const xAOD::PhotonContainer& photons, const CP::SystematicSet& systConfig, const bool effSF = true, const bool isoSF = true, const bool triggerSF = false) override final;

    //jets
    double GetTotalJetSF(const xAOD::JetContainer* jets, const bool btagSF = true, const bool jvtSF = true, const bool fjvtSF = false) override final;

    double GetTotalJetSFsys(const xAOD::JetContainer* jets, const CP::SystematicSet& systConfig, const bool btagSF = true, const bool jvtSF = true, const bool fjvtSF = false) override final;

    //Trigger
    bool IsMETTrigPassed(unsigned int runnumber = 0, bool j400_OR = false) const override final;
    bool IsMETTrigPassed(const std::string& triggerName, bool j400_OR = false, const std::string& L1_name = "L1_XE50") const override final;

    bool IsTrigPassed(const std::string&, unsigned int condition=TrigDefs::Physics) const override final;

    bool IsTrigMatched(const xAOD::IParticle *part, const std::string& tr_item) override final;
    bool IsTrigMatched(const xAOD::IParticle *part1, const xAOD::IParticle *part2, const std::string& tr_item) override final;
    bool IsTrigMatched(const std::vector<const xAOD::IParticle*>& v, const std::string& tr_item) override final;
    bool IsTrigMatched(const std::initializer_list<const xAOD::IParticle*> &v, const std::string& tr_item) override final;

    void TrigMatch(const xAOD::IParticle* p, std::initializer_list<std::string>::iterator, std::initializer_list<std::string>::iterator) override final;
    void TrigMatch(const xAOD::IParticle* p, const std::vector<std::string>& items) override final;
    void TrigMatch(const xAOD::IParticle* p, const std::initializer_list<std::string>& items) override final;
    void TrigMatch(const xAOD::IParticleContainer* v, const std::vector<std::string>& items) override final;
    void TrigMatch(const xAOD::IParticleContainer* v, const std::initializer_list<std::string>& items) override final;
    void TrigMatch(const std::initializer_list<const xAOD::IParticle*>& v, const std::vector<std::string>& items) override final;
    void TrigMatch(const std::initializer_list<const xAOD::IParticle*>& v, const std::initializer_list<std::string>& items) override final;
    void TrigMatch(const xAOD::IParticle* p, const std::string& item) override final;
    void TrigMatch(const xAOD::IParticleContainer* v,  const std::string& item) override final;
    void TrigMatch(const std::initializer_list<const xAOD::IParticle*> &v, const std::string& item) override final;

    // Trigger helpers
    float GetTrigPrescale(const std::string&) const override final;
    const Trig::ChainGroup* GetTrigChainGroup(const std::string&) const override final;
    std::vector<std::string> GetTriggerOR(const std::string& trigExpr) const;
    void GetTriggerTokens(std::string, std::vector<std::string>& , std::vector<std::string>& , std::vector<std::string>& , std::vector<std::string>& ,std::vector<std::string>& ) const;
    Trig::FeatureContainer GetTriggerFeatures(const std::string& chainName = "EF_.*", unsigned int condition = TrigDefs::Physics) const;

    const xAOD::EventInfo* GetEventInfo() const override final;

    float GetPileupWeight() override final;

    float GetPileupWeightPrescaledTrigger(const std::string & trigger_expr) override final;

    ULong64_t GetPileupWeightHash() override final;

    float GetDataWeight(const std::string&) override final;

    float GetCorrectedAverageInteractionsPerCrossing(bool includeDataSF=false) override final;

    float GetCorrectedActualInteractionsPerCrossing(bool includeDataSF=false) override final;

    double GetSumOfWeights(int channel) override final;

    unsigned int GetRandomRunNumber(bool muDependentRRN = true) override final;

    StatusCode ApplyPRWTool(bool muDependentRRN = true) override final;

    unsigned int GetRunNumber() const override final;

    const xAOD::TrackParticleContainer& GetInDetLargeD0Tracks(const EventContext &ctx) const override final;

    const xAOD::TrackParticleContainer& GetInDetLargeD0GSFTracks(const EventContext &ctx) const override final;

    StatusCode ApplyLRTUncertainty() override final;

    int treatAsYear(const int runNumber=-1) const override final;

    StatusCode OverlapRemoval(const xAOD::ElectronContainer *electrons, const xAOD::MuonContainer *muons, const xAOD::JetContainer *jets,
                              const xAOD::PhotonContainer* gamma = nullptr, const xAOD::TauJetContainer* taujet = nullptr, const xAOD::JetContainer *fatjets = nullptr) override final;

    StatusCode NearbyLeptonCorrections( xAOD::ElectronContainer *electrons = nullptr, xAOD::MuonContainer *muons = nullptr) const override final;

    StatusCode resetSystematics() override final;

    const CP::SystematicSet& currentSystematic() const;

    StatusCode applySystematicVariation( const CP::SystematicSet& systConfig ) override final;


    //truth helpers
    bool isPrompt(const xAOD::IParticle* part) const override final;

    StatusCode FindSusyHP(int& pdgid1, int& pdgid2) const;
    StatusCode FindSusyHP(const xAOD::TruthParticleContainer *truthP, int& pdgid1, int& pdgid2, bool isTruth3=false) const override final;
    StatusCode FindSusyHP(const xAOD::TruthEvent *truthE, int& pdgid1, int& pdgid2) const override final;
    static bool FindSusyHardProc(const xAOD::TruthParticleContainer *truthP, int& pdgid1, int& pdgid2, bool isTruth3=false);
    static bool FindSusyHardProc(const xAOD::TruthEvent *truthE, int& pdgid1, int& pdgid2);

    //trigger helpers
    std::string TrigSingleLep() const override final;

    //systematics helpers
    bool isNominal(const CP::SystematicSet& syst) const;
    bool isWeight(const CP::SystematicSet& systSet) const;
    bool isVariation(const CP::SystematicSet& syst) const;
    bool currentSystematicIsNominal() const;
    bool currentSystematicIsVariation() const;
    bool currentSystematicIsWeight() const;

    ST::SystInfo getSystInfo(const CP::SystematicVariation& sys) const override final;

    std::vector<ST::SystInfo> getSystInfoList() const override final;

    // Temporary function for Sherpa 2.2 V+jets n-jets reweighting
    // (see https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/CentralMC15ProductionList#NEW_Sherpa_v2_2_V_jets_NJet_rewe)
    float getSherpaVjetsNjetsWeight() const override final;
    float getSherpaVjetsNjetsWeight(const std::string& jetContainer) const override final;

    // Helper for b-tagging weights
    int getMCShowerType(const std::string& sample_name="", const std::string& tagger="") const override final { return ST::getMCShowerType(sample_name, tagger); }


  private:

    StatusCode SUSYToolsInit();

    //map ConfigFile names to Property names
    std::map<std::string, std::string> m_conf_to_prop;

    //book boolean properties already set
    std::set<std::string> m_bool_prop_set;

    // These variables just cache decisions so it's OK to make them mutable
    // Store trigger emulation functions so they're only setup once
    mutable std::map<std::string, std::function<bool()>> m_metTriggerFuncs ATLAS_THREAD_SAFE;
    // Store whether the trigger was in the TDT
    mutable std::map<std::string, bool> m_checkedTriggers ATLAS_THREAD_SAFE;
    mutable std::mutex m_triggerCacheMutex;
    bool emulateHLT(const std::string& triggerName) const;
    bool isTrigInTDT(std::scoped_lock<std::mutex>& lock,
                     const std::string& triggerName) const;

    //book trigger chains for matching
    std::vector<std::string> m_v_trigs15_cache_singleEle;
    std::vector<std::string> m_v_trigs16_cache_singleEle;
    std::vector<std::string> m_v_trigs17_cache_singleEle;
    std::vector<std::string> m_v_trigs18_cache_singleEle;
    std::vector<std::string> m_v_trigs22_cache_singleEle;

    std::vector<std::string> m_v_trigs15_cache_singleLep;
    std::vector<std::string> m_v_trigs16_cache_singleLep;
    std::vector<std::string> m_v_trigs17_cache_singleLep;
    std::vector<std::string> m_v_trigs18_cache_singleLep;
    std::vector<std::string> m_v_trigs22_cache_singleLep;

    std::vector<std::string> m_v_trigs15_cache_diLep;
    std::vector<std::string> m_v_trigs16_cache_diLep;
    std::vector<std::string> m_v_trigs17_cache_diLep;
    std::vector<std::string> m_v_trigs18_cache_diLep;
    std::vector<std::string> m_v_trigs22_cache_diLep;

    std::vector<std::string> m_v_trigs15_cache_multiLep;
    std::vector<std::string> m_v_trigs16_cache_multiLep;
    std::vector<std::string> m_v_trigs17_cache_multiLep;
    std::vector<std::string> m_v_trigs18_cache_multiLep;
    std::vector<std::string> m_v_trigs22_cache_multiLep;

    // Read Handles
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_LRTCollectionName{this, "LRTCollectionName", "InDetLargeD0TrackParticles", "LRT collection name"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_GSFLRTCollectionName{this, "GSFLRTCollectionName", "LRTGSFTrackParticles", "GSF LRT collection name"};

  protected:

    // autoconfiguration of pileup-reweighting tool
    StatusCode autoconfigurePileupRWTool(const std::string& PRWfilesDir = "dev/PileupReweighting/share/", const std::string& PRWfileName = "", bool usePathResolver = true, bool RPVLLmode = false, bool Combinedmode = false, const std::string& HFFilter = "");

    StatusCode readConfig() override final;
    StatusCode validConfig(bool strict = false) const;

    const std::vector<std::string> split(const std::string& s, const std::string& delim) const;

    std::string getDefaultJetUncConfig();

    void getTauConfig(const std::string& tauConfigPath, std::vector<float>& pT_window, std::vector<float>& eta_window, bool &eleOLR, bool &muVeto, bool &muOLR) const;

    void configFromFile(bool& property, const std::string& propname, TEnv& rEnv,
                        bool defaultValue);
    void configFromFile(double& property, const std::string& propname, TEnv& rEnv,
                        double defaultValue);
    void configFromFile(int& property, const std::string& propname, TEnv& rEnv,
                        int defaultValue);
    void configFromFile(std::string& property, const std::string& propname, TEnv& rEnv,
                        const std::string& defaultValue, bool allowEmpty=false);

    //little helpers for WP configurations / handling
    bool check_isOption(const std::string& wp, const std::vector<std::string>& list) const;
    bool check_isTighter(const std::string& wp1, const std::string& wp, const std::vector<std::string>& list) const;

    std::string EG_WP(const std::string& wp) const; //translate our WPs to make egamma selectors happy

    std::vector<std::string> getElSFkeys(const std::string& mapFile) const;

#ifdef XAOD_STANDALONE // more convenient for property setting
    DataSource m_dataSource;
    xAOD::JetInput::Type m_jetInputType; // permit switching between LC, PFlow, EM jets
#else
    int m_dataSource;
    int m_jetInputType;
#endif
    std::string m_configFile;

    bool m_force_noElId;
    bool m_force_noMuId;
    bool m_doTTVAsf;
    bool m_doModifiedEleId;
    bool m_upstreamTriggerMatching; /// Use composite trigger matching tool if matching was done upstream
    std::string m_trigMatchingPrefix; /// Prefix for trigger matchiing container name

    std::string m_jetUncertaintiesConfig;
    std::string m_jetUncertaintiesAnalysisFile;
    std::string m_jetUncertaintiesCalibArea;
    std::string m_jetUncertaintiesMCType;
    bool m_jetUncertaintiesPDsmearing;

    bool m_useBtagging;
    bool m_useBtagging_trkJet;
    bool m_debug;

    bool m_strictConfigCheck;

    std::string m_badJetCut;

    std::string m_fatJetUncConfig;
    bool m_fatJetUncertaintiesPDsmearing;
    std::string m_fatJetUncVars;

    TEnv m_WconfigReader;
    TEnv m_ZconfigReader;
    TEnv m_TopconfigReader;
    std::string m_WDecorName;
    std::string m_ZDecorName;
    std::string m_TopDecorName;
    std::string m_WtagConfig;
    std::string m_ZtagConfig;
    std::string m_ToptagConfig;
    std::string m_WZTaggerCalibArea;
    std::string m_TopTaggerCalibArea;
    std::string m_WTagUncConfig;
    std::string m_ZTagUncConfig;
    std::string m_TopTagUncConfig;
    std::string m_JetTruthLabelName;

    bool m_tool_init;
    bool m_subtool_init;

    std::string m_eleTerm    ;
    std::string m_gammaTerm  ;
    std::string m_tauTerm    ;
    std::string m_jetTerm    ;
    std::string m_muonTerm   ;
    std::string m_inputMETSuffix;
    std::string m_inputMETMap;
    std::string m_inputMETCore;
    std::string m_inputMETRef;
    std::string m_outMETTerm;
    bool m_metRemoveOverlappingCaloTaggedMuons;
    bool m_metDoSetMuonJetEMScale;
    bool m_metDoRemoveMuonJets;
    bool m_metUseGhostMuons;
    bool m_metDoMuonEloss;
    bool m_metGreedyPhotons;
    bool m_metVeryGreedyPhotons;
    std::string m_metsysConfigPrefix;

    bool m_trkMETsyst;
    bool m_caloMETsyst;
    bool m_trkJetsyst;

    int m_softTermParam;
    bool m_treatPUJets;
    bool m_doPhiReso;

    bool m_autoconfigPRW;
    std::string m_autoconfigPRWPath;
    std::string m_autoconfigPRWFile;
    bool m_autoconfigPRWCombinedmode;
    bool m_autoconfigPRWRPVmode;
    std::string m_autoconfigPRWRtags;
    std::string m_mcCampaign;
    int m_mcChannel;

    bool m_useCommonPRWFiles;
    std::string m_commonPRWFileMC20a;
    std::string m_commonPRWFileMC20d;
    std::string m_commonPRWFileMC20e;
    std::string m_commonPRWFileMC21a;
    std::string m_commonPRWFileMC23a;
    std::string m_commonPRWFileMC23c;
    std::string m_commonPRWFileMC23d;
    std::string m_commonPRWFileMC23e;

    std::vector<std::string> m_prwConfFiles;
    std::vector<std::string> m_prwLcalcFiles;
    std::string m_prwActualMu2017File;
    std::string m_prwActualMu2018File;
    std::string m_prwActualMu2022File;
    std::string m_prwActualMu2023File;
    std::string m_prwActualMu2024File;

    double m_prwDataSF;
    double m_prwDataSF_UP;
    double m_prwDataSF_DW;
    bool   m_runDepPrescaleWeightPRW;

    // bookkeep supported configurations (in increasing order of tightness)
    std::vector<std::string> m_el_id_support;
    std::vector<std::string> m_ph_id_support;
    int m_mu_id_support; //max value
    std::vector<std::string> m_tau_id_support;
    std::vector<std::string> m_el_iso_support;
    std::vector<std::string> m_mu_iso_support;

    // Iso SF mapping file (temporary while not all SFs are available)
    std::map<std::string, std::string> m_el_iso_fallback;
    std::map<std::string, std::string> m_mu_iso_fallback;

    // strings needed for dealing with 2015+2016 electron trigger SFs
    std::string m_electronTriggerSFStringSingle;

    std::map<std::string, std::string> m_tau_trig_support;

    std::string m_eleId;
    std::string m_eleIdBaseline;
    std::string m_eleConfig;
    std::string m_eleConfigBaseline;
    std::string m_eleBaselineIso_WP;
    bool        m_eleIdExpert;
    int         m_muId;
    int         m_muIdBaseline;
    std::string m_photonId;
    std::string m_photonIdBaseline;
    std::string m_tauId;
    std::string m_tauIdBaseline;
    std::string m_eleIso_WP;
    std::string m_eleIsoHighPt_WP;
    double      m_eleIsoHighPtThresh;
    std::string m_eleChID_WP;
    bool m_eleLRT;
    int m_eleLRT_strat;
    bool        m_eleChIso; // use Charge ID SF with/without Iso applied
    bool        m_eleChID_signal; // allows to run ECID but remove it from signal definition
    bool        m_runECIS; //run ChargeIDSelector if valid WP was selected
    std::string m_photonBaselineIso_WP;
    std::string m_photonIso_WP;
    std::string m_photonTriggerName;
    std::string m_muBaselineIso_WP;
    std::string m_muIso_WP;
    std::string m_muIsoHighPt_WP;
    double      m_muIsoHighPtThresh;
    bool        m_muHighPtExtraSmear;
    bool        m_muEffCorrForce1D;
    std::string m_muTriggerSFCalibRelease;
    std::string m_muTriggerSFCalibFilename;
    std::string m_BtagWP;
    std::string m_BtagTagger;
    double m_BtagMinPt;
    std::string m_BtagKeyOverride;
    std::string m_BtagSystStrategy;
    std::string m_EigenvectorReductionB;
    std::string m_EigenvectorReductionC;
    std::string m_EigenvectorReductionLight;
    std::string m_BtagWP_trkJet;
    std::string m_BtagTagger_trkJet;
    double m_BtagMinPt_trkJet;

    //configurable cuts here
    double m_eleBaselinePt;
    double m_eleBaselineEta;
    bool   m_eleBaselineCrackVeto;
    double m_elePt;
    double m_eleEta;
    bool   m_eleCrackVeto;
    double m_eled0sig;
    double m_elez0;
    double m_elebaselined0sig;
    double m_elebaselinez0;
    std::string m_eleEffMapFilePath;
    std::string m_eleEffMapFilePathRun2;
    bool   m_eleAllowRun3TrigSFFallback;
    bool   m_eleForceFullSimCalib;


    double m_muBaselinePt;
    double m_muBaselineEta;
    double m_muPt;
    double m_muEta;
    double m_mud0sig;
    double m_muz0;
    double m_mubaselined0sig;
    double m_mubaselinez0;
    bool   m_murequirepassedHighPtCuts;
    double m_muCosmicz0;
    double m_muCosmicd0;
    double m_badmuQoverP;
    int    m_muCalibrationMode;
    bool m_muLRT;

    double m_photonBaselinePt;
    double m_photonBaselineEta;
    double m_photonEta;
    double m_photonPt;
    bool   m_photonBaselineCrackVeto;
    bool   m_photonCrackVeto;
    bool   m_photonAllowLate;

    double m_tauPrePtCut;
    double m_tauPt;
    double m_tauEta;
    std::string m_tauConfigPath;
    std::string m_tauConfigPathBaseline;
    bool   m_tauDoTTM;
    std::string m_tauSmearingToolRecommendationTag;
    std::string m_tauEffToolRecommendationTag;
    std::string m_tauSmearingToolGenerator;
    bool m_ApplyMVATESQualityCheck;
    TEnv m_tauConfigReader;

    double m_jetPt;
    double m_jetEta;
    double m_jetJvt;
    std::string m_JvtWP;
    double m_JvtPtMax;
    std::string m_JvtConfigRun2;
    std::string m_JvtConfigRun3;

    double m_trkJetPt;
    double m_trkJetEta;

    bool   m_doFwdJVT;
    std::string m_fJvtWP;
    double m_fJvtPtMax;
    double m_fJvtEtaMin;
    std::string m_fJvtConfigRun2;
    std::string m_fJvtConfigRun3;

    bool m_JMScalib;

    /// Overlap removal options
    bool   m_orDoTau;
    bool   m_orDoPhoton;
    bool   m_orDoEleJet;
    bool   m_orDoElEl;
    bool   m_orDoElMu;
    bool   m_orDoMuonJet;
    bool   m_orDoBjet;
    bool   m_orDoElBjet;
    bool   m_orDoMuBjet;
    bool   m_orDoTauBjet;
    bool   m_orDoBoostedElectron;
    double m_orBoostedElectronC1;
    double m_orBoostedElectronC2;
    double m_orBoostedElectronMaxConeSize;
    bool   m_orDoBoostedMuon;
    double m_orBoostedMuonC1;
    double m_orBoostedMuonC2;
    double m_orBoostedMuonMaxConeSize;
    bool   m_orApplyRelPt;
    double m_orMuJetPtRatio;
    double m_orMuJetTrkPtRatio;
    double m_orMuJetInnerDR;
    bool   m_orDoMuonJetGhostAssociation;
    bool   m_orRemoveCaloMuons;
    std::string m_orBtagWP;
    std::string m_orInputLabel;
    bool   m_orPhotonFavoured;
    double m_orBJetPtUpperThres;
    bool m_orLinkOverlapObjects;

    bool   m_orDoFatjets;
    double m_EleFatJetDR;
    double m_JetFatJetDR;

    bool m_doIsoSignal;
    bool m_doElIsoSignal;
    bool m_doPhIsoSignal;
    bool m_doMuIsoSignal;

    bool m_useSigLepForIsoCloseByOR;
    std::string m_IsoCloseByORpassLabel;

    bool m_useTRUTH3;

    std::map<std::string,bool> m_slices;
    bool m_isRun3;
    bool m_isPHYSLITE;

    std::string m_metJetSelection;

    int  m_showerType;

    std::string m_defaultJets;
    std::string m_defaultTrackJets;
    std::string m_fatJets;
    std::string m_defaultTruthJets;

    CP::SystematicSet m_defaultSyst = CP::SystematicSet();
    CP::SystematicSet m_currentSyst;
    std::vector<CP::SystematicSet> m_fatjetFFSmearingSyst; // as we need to keep track of the systematics of the FFJetSmearingTool

    std::string m_EG_corrModel;
    std::string m_EG_corrFNList;
    bool m_applyJVTCut;

    std::string m_bTaggingCalibrationFilePath;

    std::map<std::string,std::string> m_legsPerTool;
    std::map<std::string,std::string> m_legsPerTool_ph;

    asg::AnaToolHandle<IJetCalibrationTool> m_jetCalibTool;
    asg::AnaToolHandle<IJetCalibrationTool> m_jetFatCalibTool;
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesTool;
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_jetUncertaintiesPDSmearTool;
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_fatjetUncertaintiesTool;
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_fatjetUncertaintiesPDSmearTool;
    asg::AnaToolHandle<ICPJetCorrectionTool> m_fatjetFFSmearingTool;
    asg::AnaToolHandle<IJetSelector> m_jetCleaningTool;

    asg::AnaToolHandle<JetPileupLabelingTool>  m_jetPileupLabelingTool;
    asg::AnaToolHandle<JetVertexTaggerTool> m_jetJvtMomentTool;
    asg::AnaToolHandle<JetPileupTag::JetVertexNNTagger>  m_jetNNJvtMomentTool;
    asg::AnaToolHandle<IAsgSelectionTool> m_jetNNJvtSelectionTool;
    asg::AnaToolHandle<CP::IJvtEfficiencyTool> m_jetNNJvtEfficiencyTool;
    asg::AnaToolHandle<IAsgSelectionTool> m_jetfJvtSelectionTool;
    asg::AnaToolHandle<CP::IJvtEfficiencyTool> m_jetfJvtEfficiencyTool;

    asg::AnaToolHandle<SmoothedWZTagger> m_WTaggerTool;
    asg::AnaToolHandle<SmoothedWZTagger> m_ZTaggerTool;
    asg::AnaToolHandle<JSSWTopTaggerDNN> m_TopTaggerTool;
    asg::AnaToolHandle<JetTruthLabelingTool> m_jetTruthLabelingTool;
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_WTagjetUncertaintiesTool;
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_ZTagjetUncertaintiesTool;
    asg::AnaToolHandle<ICPJetUncertaintiesTool> m_TopTagjetUncertaintiesTool;

    SG::ReadDecorHandleKey<xAOD::JetContainer> m_label_truthKey;

    //
    std::string m_jesConfig;
    std::string m_jesConfigJMS;
    std::string m_jesConfigJMSData;
    std::string m_jesConfigAFII;
    std::string m_jesConfigFat;
    std::string m_jesConfigFatData;
    std::string m_jesCalibSeq;
    std::string m_jesCalibSeqJMS;
    std::string m_jesCalibSeqFat;
    std::string m_jesCalibArea;
    //
    asg::AnaToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool;
    asg::AnaToolHandle<CP::IMuonSelectionTool> m_muonSelectionHighPtTool;
    asg::AnaToolHandle<CP::IMuonSelectionTool> m_muonSelectionToolBaseline;
    asg::AnaToolHandle<CP::IMuonCalibrationAndSmearingTool> m_muonCalibTool;
    asg::AnaToolHandle<CP::IMuonEfficiencyScaleFactors> m_muonEfficiencySFTool;
    asg::AnaToolHandle<CP::IMuonEfficiencyScaleFactors> m_muonEfficiencyBMHighPtSFTool;
    asg::AnaToolHandle<CP::IMuonEfficiencyScaleFactors> m_muonTTVAEfficiencySFTool;
    asg::AnaToolHandle<CP::IMuonEfficiencyScaleFactors> m_muonIsolationSFTool;
    asg::AnaToolHandle<CP::IMuonEfficiencyScaleFactors> m_muonHighPtIsolationSFTool;
    asg::AnaToolHandle<CP::IMuonTriggerScaleFactors> m_muonTriggerSFTool;
    ToolHandleArray<CP::IMuonTriggerScaleFactors> m_muonTrigSFTools;
    asg::AnaToolHandle<CP::IMuonLRTOverlapRemovalTool> m_muonLRTORTool;
    SG::WriteHandleKey<xAOD::MuonContainer>     m_outMuonLocation{this, "OutputMuonLocation", "StdWithLRTMuons", "name of the muon container to write"};   /** Combined muon collection.   */
    //
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> m_elecEfficiencySFTool_reco;
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> m_elecEfficiencySFTool_id;
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> m_elecEfficiencySFTool_trig_singleLep;
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> m_elecEfficiencySFTool_trigEff_singleLep;
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> m_elecEfficiencySFTool_iso;
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> m_elecEfficiencySFTool_isoHighPt;
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> m_elecEfficiencySFTool_chf;
    //
    std::vector<asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool>> m_elecEfficiencySFTool_trig_mixLep;
    std::vector<asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool>> m_elecEfficiencySFTool_trigEff_mixLep;
    ToolHandleArray<IAsgElectronEfficiencyCorrectionTool> m_elecTrigSFTools;
    ToolHandleArray<IAsgElectronEfficiencyCorrectionTool> m_elecTrigEffTools;
    asg::AnaToolHandle<CP::IElectronLRTOverlapRemovalTool> m_elecLRTORTool;
    SG::WriteHandleKey<xAOD::ElectronContainer>     m_outElectronLocation{this, "OutputElectronLocation", "StdWithLRTElectrons", "name of the muon container to write"};   /** Combined electron collection.   */

    //
    asg::AnaToolHandle<CP::IEgammaCalibrationAndSmearingTool> m_egammaCalibTool;
    asg::AnaToolHandle<IAsgElectronLikelihoodTool> m_elecSelLikelihood;
    asg::AnaToolHandle<IAsgElectronLikelihoodTool> m_elecSelLikelihoodBaseline;
    asg::AnaToolHandle<IAsgPhotonIsEMSelector>     m_photonSelIsEM;
    asg::AnaToolHandle<IAsgPhotonIsEMSelector>     m_photonSelIsEMBaseline;
    asg::AnaToolHandle<IAsgDeadHVCellRemovalTool>  m_deadHVTool;
    asg::AnaToolHandle<IAsgPhotonEfficiencyCorrectionTool> m_photonEfficiencySFTool;
    asg::AnaToolHandle<IAsgPhotonEfficiencyCorrectionTool> m_photonIsolationSFTool;
    asg::AnaToolHandle<IAsgPhotonEfficiencyCorrectionTool> m_photonTriggerSFTool;
    //
    std::vector<asg::AnaToolHandle<IAsgPhotonEfficiencyCorrectionTool>> m_photonEfficiencySFTool_trigSF_AsymDiphoton;
    std::vector<asg::AnaToolHandle<IAsgPhotonEfficiencyCorrectionTool>> m_photonEfficiencySFTool_trigEff_AsymDiphoton;
    ToolHandleArray<IAsgPhotonEfficiencyCorrectionTool> m_photonTrigSFTools;
    ToolHandleArray<IAsgPhotonEfficiencyCorrectionTool> m_photonTrigEffTools;
    //
    asg::AnaToolHandle<IEGammaAmbiguityTool> m_egammaAmbiguityTool;
    asg::AnaToolHandle<IAsgElectronLikelihoodTool> m_elecChargeIDSelectorTool;
    asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool>      m_elecChargeEffCorrTool;
    //
    asg::AnaToolHandle<TauAnalysisTools::ITauSelectionTool> m_tauSelTool;
    asg::AnaToolHandle<TauAnalysisTools::ITauSelectionTool> m_tauSelToolBaseline;
    asg::AnaToolHandle<TauAnalysisTools::ITauSmearingTool> m_tauSmearingTool;
    asg::AnaToolHandle<TauAnalysisTools::ITauTruthMatchingTool> m_tauTruthMatch;
    asg::AnaToolHandle<TauAnalysisTools::ITauEfficiencyCorrectionsTool> m_tauEffTool;
    std::vector<asg::AnaToolHandle<TauAnalysisTools::ITauEfficiencyCorrectionsTool>> m_tauTrigEffTool;
    //
    asg::AnaToolHandle<IBTaggingEfficiencyTool> m_btagEffTool;
    asg::AnaToolHandle<IBTaggingSelectionTool> m_btagSelTool;
    asg::AnaToolHandle<IBTaggingSelectionTool> m_btagSelTool_OR;
    asg::AnaToolHandle<IBTaggingEfficiencyTool> m_btagEffTool_trkJet;
    asg::AnaToolHandle<IBTaggingSelectionTool> m_btagSelTool_trkJet;
    //
    asg::AnaToolHandle<IMETMaker> m_metMaker;
    asg::AnaToolHandle<IMETSystematicsTool> m_metSystTool;
    asg::AnaToolHandle<IMETSignificance> m_metSignif;
    //
    std::string m_trig2015combination_singleLep;
    std::string m_trig2016combination_singleLep;
    std::string m_trig2017combination_singleLep;
    std::string m_trig2018combination_singleLep;
    std::string m_trig2022combination_singleLep;
    //
    int m_trigNToys_diLep;
    std::string m_trig2015combination_diLep;
    std::string m_trig2016combination_diLep;
    std::string m_trig2017combination_diLep;
    std::string m_trig2018combination_diLep;
    std::string m_trig2022combination_diLep;
    asg::AnaToolHandle<ITrigGlobalEfficiencyCorrectionTool> m_trigGlobalEffCorrTool_diLep;
    //
    int m_trigNToys_multiLep;
    std::string m_trig2015combination_multiLep;
    std::string m_trig2016combination_multiLep;
    std::string m_trig2017combination_multiLep;
    std::string m_trig2018combination_multiLep;
    std::string m_trig2022combination_multiLep;
    asg::AnaToolHandle<ITrigGlobalEfficiencyCorrectionTool> m_trigGlobalEffCorrTool_multiLep;
    //
    int m_trigNToys_diPhoton;
    std::string m_trig2015combination_diPhoton;
    std::string m_trig2016combination_diPhoton;
    std::string m_trig2017combination_diPhoton;
    std::string m_trig2018combination_diPhoton;
    std::string m_trig2022combination_diPhoton;
    asg::AnaToolHandle<ITrigGlobalEfficiencyCorrectionTool> m_trigGlobalEffCorrTool_diPhoton;
    //
    asg::AnaToolHandle<TrigConf::ITrigConfigTool> m_trigConfTool;
    asg::AnaToolHandle<Trig::TrigDecisionTool> m_trigDecTool;
    asg::AnaToolHandle<Trig::IMatchingTool> m_trigMatchingTool;
    asg::AnaToolHandle<Trig::IMatchScoringTool> m_trigMatchScoringTool;
    asg::AnaToolHandle<Trig::IMatchScoringTool> m_trigDRScoringTool;
    //
    asg::AnaToolHandle<CP::IIsolationCorrectionTool> m_isoCorrTool;
    asg::AnaToolHandle<CP::IIsolationSelectionTool> m_isoTool;
    //disable asg::AnaToolHandle<CP::IIsolationLowPtPLVTool> m_isoToolLowPtPLV;
    asg::AnaToolHandle<CP::IIsolationSelectionTool> m_isoBaselineTool;
    asg::AnaToolHandle<CP::IIsolationSelectionTool> m_isoHighPtTool;
    asg::AnaToolHandle<CP::IIsolationCloseByCorrectionTool> m_isoCloseByTool;
    //
    asg::AnaToolHandle<CP::IPileupReweightingTool> m_prwTool;
    //
    asg::AnaToolHandle<InDet::IInclusiveTrackFilterTool> m_LRTuncTool;
    //
    asg::AnaToolHandle<ORUtils::IOverlapTool> m_tauJetORtool;
    ORUtils::ToolBox m_orToolbox;
    //
    asg::AnaToolHandle<IWeightTool> m_pmgSHnjetWeighter;
    asg::AnaToolHandle<IWeightTool> m_pmgSHnjetWeighterWZ;
    //
    std::string m_eleIdBaselineDFName;
    std::string m_eleIdDFName;
    std::string m_photonIdBaselineDFName;
    std::string m_photonIdDFName;
    std::string m_jetCleanDFName;
    //
    SG::ConstAccessor<char> m_acc_eleIdBaseline;
    SG::ConstAccessor<char> m_acc_eleId;
    SG::ConstAccessor<char> m_acc_photonIdBaseline;
    SG::ConstAccessor<char> m_acc_photonId;
    SG::ConstAccessor<char> m_acc_jetClean;

  }; // Class SUSYObjDef_xAOD

  // decorators
  const static SG::Decorator<unsigned> dec_isEM("isEM");
  const static SG::Decorator<char> dec_baseline("baseline");
  const static SG::Decorator<char> dec_selected("selected"); //for priority-aware OR of baseline objects
  const static SG::Decorator<char> dec_signal("signal");
  const static SG::Decorator<char> dec_isol("isol");
  const static SG::Decorator<char> dec_isolHighPt("isolHighPt"); // use different WPs for low-pt and high-pt
  const static SG::Decorator<char> dec_passOR("passOR");
  const static SG::Decorator<double> dec_effscalefact("effscalefact");
  const static SG::Decorator<char> dec_signal_less_JVT("signal_less_JVT"); //!< Decorator for signal jets without a JVT requirement
  const static SG::Decorator<char> dec_lrtFilter("passLRTFilter");


  // const accessors for reading decorations that we set
  const static SG::ConstAccessor<char> acc_baseline("baseline");
  const static SG::ConstAccessor<char> acc_selected("selected"); //for priority-aware OR of baseline objects
  const static SG::ConstAccessor<char> acc_signal("signal");
  const static SG::ConstAccessor<char> acc_isol("isol");
  const static SG::ConstAccessor<char> acc_isolHighPt("isolHighPt"); // use different WPs for low-pt and high-pt
  const static SG::ConstAccessor<char> acc_passOR("passOR");
  const static SG::ConstAccessor<char> acc_signal_less_JVT("signal_less_JVT"); //!< Accessor for signal jets without a JVT requirement
  const static SG::ConstAccessor<char> acc_bjet("bjet");
  const static SG::ConstAccessor<double> acc_btag_weight("btag_weight");
  const static SG::ConstAccessor<float> acc_btag_dl1pb("btag_dl1pb");
  const static SG::ConstAccessor<float> acc_btag_dl1pc("btag_dl1pc");
  const static SG::ConstAccessor<float> acc_btag_dl1pu("btag_dl1pu");
  const static SG::ConstAccessor<int> acc_wtagged("wtagged");
  const static SG::ConstAccessor<int> acc_ztagged("ztagged");
  const static SG::ConstAccessor<int> acc_toptagged("toptagged");
  const static SG::ConstAccessor<char> acc_bad("bad");
  const static SG::ConstAccessor<char> acc_trigmatched("trigmatched");
  const static SG::ConstAccessor<double> acc_effscalefact("effscalefact");
  const static SG::ConstAccessor<char> acc_lrtFilter("passLRTFilter");
  const static SG::ConstAccessor<float> acc_d0sig("d0sig");
  const static SG::ConstAccessor<float> acc_z0sinTheta("z0sinTheta");
  const static SG::ConstAccessor<char> acc_cosmic("cosmic");
  const static SG::ConstAccessor<char> acc_passedHighPtCuts("passedHighPtCuts");

  // more decorations that are set externally
  const static SG::ConstAccessor<unsigned int> acc_OQ("OQ");
  const static SG::ConstAccessor<int> acc_truthType("truthType");
  const static SG::ConstAccessor<int> acc_truthOrigin("truthOrigin");
  const static SG::ConstAccessor<int> acc_bkgTruthOrigin("bkgTruthOrigin");
  const static SG::ConstAccessor<char> acc_passPhCleaning("DFCommonPhotonsCleaning");
  const static SG::ConstAccessor<char> acc_passPhCleaningNoTime("DFCommonPhotonsCleaningNoTime");
  const static SG::ConstAccessor<unsigned int> randomrunnumber("RandomRunNumber");
  const static SG::ConstAccessor<float> acc_DetEta("DetectorEta");


} // namespace ST


#endif // not SUSYTOOLS_SUSYOBJDEF_XAOD_H
