/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Hagen Möbius, hagen.mobius@cern.ch
#ifndef SSV_Weights_ALG_H
#define SSV_Weights_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"

// Framework includes
#include <xAODJet/JetContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include <xAODTruth/TruthParticle.h>
#include <xAODTruth/TruthParticleContainer.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>

// Additionally include

#include <nlohmann/json.hpp>

namespace CP{
  class SSVWeightsAlg final : public EL::AnaAlgorithm{

  public:
    SSVWeightsAlg(const std::string &name, ISvcLocator *pSvcLocator);
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    Gaudi::Property<std::string> m_jsonConfigPath_SSVWeightsAlg {this, "JsonConfigFile_SSVWeightsAlg","", "Path to the JSON config file that contains the SSV calibration results which are needed to calculate the SSV weights"};
    Gaudi::Property<std::string> m_BTaggingWP {this, "BTaggingWP", "", "b-tagging working point that is used to count the number of b-jets in the event for b-jet based SSV weight calculation"};
    Gaudi::Property<std::string> m_EfficiencyMethod {this, "EfficiencyMethod", "", "efficiency definition that will be used to calculate the SSV weights, string can be 'Bhadron_pT_eta_based' or 'bjet_based'"};
    Gaudi::Property<std::string> m_nFMethod {this, "nFMethod", "", "average number of fake SSV definition that will be used to calculate the SSV weights, string can be 'pileup_bjet_based','pileup_based_linearfit' or 'pileup_based_binned'"};
    Gaudi::Property<std::string> m_OutputVariableSize {this, "OutputVariableSize", "", "number of variables that will be saved to the output, string can be 'standard','extended','additional' or 'all'"};

    nlohmann::json m_jsonConfig_SSVWeightsAlg;
    const double m_lowMuHighMuThreshold = 42.93;

    enum class EfficiencyMethodType {
      unknown,
      bjet_based,
      Bhadron_pT_eta_based,
    };

    enum class nFMethodType {
      unknown,
      pileup_bjet_based,
      pileup_based_linearfit,
      pileup_based_binned
    };

    enum class OutputVariableSizeType {
      unknown,
      standard,
      extended,
      additional,
      all
    };

    EfficiencyMethodType m_EfficiencyMethodType{EfficiencyMethodType::unknown};
    nFMethodType m_nFMethodType{nFMethodType::unknown};
    OutputVariableSizeType m_OutputVariableSizeType{OutputVariableSizeType::unknown};

    std::vector<const xAOD::Vertex*> create_good_SSVs(
      const std::vector<const xAOD::Jet*> &jets,
      const std::vector<const xAOD::Electron*> &electrons,
      const std::vector<const xAOD::Muon*> &muons,
      const std::vector<const xAOD::Vertex*> &SSVs)const;

    std::vector<const xAOD::TruthParticle*> create_accepted_truthBhs(
      const std::vector<const xAOD::TruthParticle*> &truthBhs,
      const std::vector<const xAOD::Jet*> &jets)const;
  
    int count_number_of_fake_SSVs(
      const std::vector<const xAOD::TruthParticle*> &truthBhs,
      const std::vector<const xAOD::Vertex*> &SSVs)const;

    std::vector<bool> truthBh_to_SSV_matching(
      const std::vector<const xAOD::TruthParticle*> &truthBhs,
      const std::vector<const xAOD::Vertex*> &SSVs)const;

    const std::vector<const xAOD::TruthParticle*> construct_not_matched_vectors(
      const std::vector<const xAOD::TruthParticle*> &truthBhs,
      const std::vector<bool> &matched_vector)const;

    double compute_DeltaR_between_SSV_and_particle(
      const xAOD::Vertex* vtx, 
      const xAOD::IParticle * part) const;

    double poisson_pmf(
      const int k,
      const double lambda)const; 

    int count_matched_objects(
      const std::vector<bool> &matching_vector)const;

    int count_not_matched_objects(
      const std::vector<bool> &matching_vector)const;

    bool isHFHadronFinalState(
      const xAOD::TruthParticle *part,
      const int type) const;

    double calculate_P_ineff_Bhadron_pt_eta_based(
      const std::vector<const xAOD::TruthParticle*> &accepted_truthBh,
      const std::vector<bool> &truthBh_to_SSV_matched,
      double SF_eff) const;

    double calculate_P_ineff_bjet_based(
      const int b_jet_count,
      const int N_missed,
      const double SF_eff) const;

    double calculate_P_fake_pileup_bjet_based(
      const double muactual,
      const int b_jet_count,
      const int N_fake,
      const double SF_fake_low,
      const double SF_fake_high) const;

    double calculate_P_fake_pileup_based_linearfit(
      const double muactual,
      const int N_fake) const;

    double calculate_P_fake_pileup_based_binned(
      const double muactual,
      const int N_fake,
      const double SF_fake_low,
      const double SF_fake_high) const;

    CP::SysListHandle m_systematicsList{this};
    CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle{
      this, "eventInfo", "EventInfo", "the EventInfo container"};

    CP::SysReadHandle<xAOD::TruthParticleContainer> m_truthParticlesHandle{
      this, "TruthParticleContainer", "TruthParticles", "input TruthParticles container"};

    CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle{
      this, "jets", "", "the jet container to use"};

    CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
      this, "electrons", "", "the electron container to use"};

    CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
      this, "muons", "", "the muon container to use"};

    CP::SysReadHandle<xAOD::VertexContainer> m_ssvHandle{
      this, "NVSI_WP", "", "The NewVrtSecInclusiveTool output container to use (NewVrtSecInclusiveTool = algorithm that constructs the soft secondary vertices (SSVs))"};

    CP::SysReadSelectionHandle m_jetSelection {
      this, "jetSelection", "", "the jet selection to apply on the jets that are used to check if they overlap with a SSV or a b-hadron"};

    CP::SysReadSelectionHandle m_electronSelection {
      this, "electronSelection", "", "the electron selection to apply on the electrons that are used to check if they overlap with a SSV or a b-hadron"};

    CP::SysReadSelectionHandle m_muonSelection {
      this, "muonSelection", "", "the muon selection to apply on the muons that are used to check if they overlap with a SSV or a b-hadron"};

    static const SG::AuxElement::ConstAccessor<float> m_ssv_pt_accessor;
    static const SG::AuxElement::ConstAccessor<float> m_ssv_m_accessor;
    static const SG::AuxElement::ConstAccessor<float> m_ssv_eta_accessor;
    static const SG::AuxElement::ConstAccessor<float> m_ssv_phi_accessor;

    CP::SysWriteDecorHandle<float> m_SSV_weight_decor{this, "SSV_weight", "SSV_weight_%SYS%", "SSV weight defined as a product of the correction factors: SSV_weight = P_eff * P_ineff * P_fake"};

    CP::SysWriteDecorHandle<float> m_N_matched_decor{this, "N_matched", "N_matched_%SYS%", "number of matched b-hadrons in an event; so number of b-hadrons in acceptance that satisfy ΔR(b-hadron in acceptance, good SSV)<0.3"};
    CP::SysWriteDecorHandle<float> m_N_missed_decor{this, "N_missed", "N_missed_%SYS%", "number of missed b-hadrons in an event; so number of b-hadrons that do not satisfy ΔR(b-hadron in acceptance, good SSV)<0.3"};
    CP::SysWriteDecorHandle<float> m_N_fake_decor{this, "N_fake", "N_fake_%SYS%", "number of fake SSVs in an event; so number of good SSVs in acceptance that do not satisfy ΔR(b-hadron in acceptance, good SSV)<0.3"};

    CP::SysWriteDecorHandle<float> m_P_eff_decor{this, "P_eff", "P_eff_%SYS%", "efficiency correction factor"};
    CP::SysWriteDecorHandle<float> m_P_ineff_decor{this, "P_ineff", "P_ineff_%SYS%", "inefficiency correction factor"};
    CP::SysWriteDecorHandle<float> m_P_fake_decor{this, "P_fake", "P_fake_%SYS%", "fake correction factor"};

    CP::SysWriteDecorHandle<float> m_number_of_bjets_decor{this, "number_of_bjets", "number_of_bjets_%SYS%", "number of b-jets in an event"};
    CP::SysWriteDecorHandle<float> m_number_of_accepted_Bhadrons_decor{this, "number_of_accepted_Bhadrons", "number_of_accepted_Bhadrons_%SYS%", "number of b-hadrons in acceptance in an event"};
    CP::SysWriteDecorHandle<float> m_number_of_good_SSVs_decor{this, "number_of_good_SSVs", "number_of_good_SSVs_%SYS%", "number of good SSVs in an event"};

    CP::SysWriteDecorHandle<float> m_P_ineff_bjet_based_decor{this, "P_ineff_bjet_based", "P_ineff_bjet_based_%SYS%", "inefficiency correction factor calculated according to the 'bjet_based' EfficiencyMethod"};
    CP::SysWriteDecorHandle<float> m_P_ineff_pt_eta_based_decor{this, "P_ineff_pt_eta_based", "P_ineff_pt_eta_based_%SYS%", "inefficiency correction factor calculated according to the 'Bhadron_pT_eta_based' EfficiencyMethod"};
    CP::SysWriteDecorHandle<float> m_P_fake_pileup_bjet_based_decor{this, "P_fake_pileup_bjet_based", "P_fake_pileup_bjet_based_%SYS%", "fake correction factor calculated according to the 'pileup_bjet_based' nFMethod"};
    CP::SysWriteDecorHandle<float> m_P_fake_pileup_based_linearfit_decor{this, "P_fake_pileup_based_linearfit", "P_fake_pileup_based_linearfit_%SYS%", "fake correction factor calculated according to the 'pileup_based_linearfit' nFMethod"};
    CP::SysWriteDecorHandle<float> m_P_fake_pileup_based_binned_decor{this, "P_fake_pileup_based_binned", "P_fake_pileup_based_binned_%SYS%", "fake correction factor calculated accoring to the 'pileup_based_binned' nFMethod"};
  };
}
#endif