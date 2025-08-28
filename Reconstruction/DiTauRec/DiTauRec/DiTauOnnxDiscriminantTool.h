// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

// EDM include(s):
#include "xAODTau/TauxAODHelpers.h"
#include "xAODTau/DiTauJet.h"
#include "DiTauToolBase.h"
#include "GaudiKernel/ToolHandle.h"
#include "PathResolver/PathResolver.h"

#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgTools/PropertyWrapper.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"

#include <onnxruntime_cxx_api.h>


class DiTauOnnxDiscriminantTool 
  : public DiTauToolBase
{
public:

  DiTauOnnxDiscriminantTool( const std::string& type, const std::string& name, const IInterface * parent);

  virtual ~DiTauOnnxDiscriminantTool();

  // initialize the tool
  virtual StatusCode initialize() override;

  //finalize the tool
  virtual StatusCode finalize() override;

  // calculate ID variables
  virtual StatusCode execute(DiTauCandidateData * data, const EventContext& ctx) const override;
  
private:

  Gaudi::Property<std::string> m_onnxModelPath {this, "onnxModelPath", "TrigTauRec/00-11-02/dev/boosted_ditau_omni_model.onnx"};
  Gaudi::Property<size_t>      m_maxTracks     {this, "maxTracks", 10};

  std::unique_ptr<Ort::Env> m_ort_env;
  std::unique_ptr<Ort::Session> m_ort_session;
  const std::vector<std::string> m_input_node_names = {"input_features", "input_points", "input_mask", "input_jet", "input_time"};
  const std::vector<std::string> m_output_node_names = {"output_1", "output_2"};

  struct InferenceOutput {
      std::vector<float> output_1;
      std::vector<float> output_2;
  };

  struct OnnxInputs {
      std::vector<float> input_features;
      std::vector<int64_t> input_features_shape;
      std::vector<float> input_points;
      std::vector<int64_t> input_points_shape;
      std::vector<float> input_mask;
      std::vector<int64_t> input_mask_shape;
      std::vector<float> input_jet;
      std::vector<int64_t> input_jet_shape;
      std::vector<float> input_time;
      std::vector<int64_t> input_time_shape;
  };

  Ort::Value create_tensor(std::vector<float> &data, const std::vector<int64_t> &shape) const;
  InferenceOutput run_inference(OnnxInputs &inputs) const;
  std::vector<float> flatten(const std::vector<std::vector<float>> &vec_2d) const;
  std::vector<float> extract_points(const std::vector<std::vector<float>> &track_features) const;
  std::vector<float> create_mask(const std::vector<std::vector<float>> &track_features) const;
  float GetDiTauObjOnnxScore(const xAOD::DiTauJet& ditau) const;

  // ReadDecorHandleKeys for the DiTau decorations
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_ditau_pt_DecorKey             { this, "DiTauPtDecorName",              "DiTauJets.ditau_pt",          "Name of the DiTau Pt decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_f_core_lead_DecorKey          { this, "DiTauFCoreLeadName",            "DiTauJets.f_core_lead",       "Name of the Ditau leading subjet core energy fraction decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_f_core_sublead_DecorKey       { this, "DiTauFCoreSubLeadName",         "DiTauJets.f_core_subl",       "Name of the Ditau subleading subjet core energy fraction decoration"}; 
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_f_subjet_subl_DecorKey        { this, "DiTauSubjetSublName",           "DiTauJets.f_subjet_subl",     "Name of the Ditau subleading subjet pt fraction decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_f_subjets_DecorKey            { this, "DiTauSubjetsName",              "DiTauJets.f_subjets",         "Name of the DiTau subjets fraction decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_R_max_lead_DecorKey           { this, "DiTauRMaxLeadName",             "DiTauJets.R_max_lead",        "Name of the Ditau Max dR distance track from leading subjet decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_R_max_sublead_DecorKey        { this, "DiTauRMaxSubleadName",          "DiTauJets.R_max_subl",        "Name of the Ditau Max dR distance track from subleading subjet decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_n_track_DecorKey              { this, "DiTauNTrackName",               "DiTauJets.n_track",           "Name of the Ditau number of tracks decoration"};  
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_R_track_all_DecorKey          { this, "DiTauRTrackAllName",            "DiTauJets.R_track_all",       "Name of the Ditau DeltaR tracks over pt in the large region decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_R_isotrack_DecorKey           { this, "DiTauRIsoTrackAllName",         "DiTauJets.R_isotrack",        "Name of the Ditau DeltaR isolated tracks over pt decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_R_track_sublead_DecorKey      { this, "DiTauRTrackSubleadName",        "DiTauJets.R_tracks_subl",     "Name of the Ditau DeltaR tracks over pt in the large region of the subleading subjet decoration"}; 
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_M_core_lead_DecorKey          { this, "DiTauMCoreLeadName",            "DiTauJets.m_core_lead",       "Name of the Ditau mass of tracks in the core region of the leading subjet decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_M_core_sublead_DecorKey       { this, "DiTauMCoreSubleadName",         "DiTauJets.m_core_subl",       "Name of the Ditau mass of tracks in the core region of the leading subjet decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_M_track_lead_DecorKey         { this, "DiTauMTrackLeadName",           "DiTauJets.m_tracks_lead",     "Name of the Ditau mass of tracks in the leading subjet decoration"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_d0_leadtrack_lead_DecorKey    { this, "DiTauD0LeadTrackLeadName",      "DiTauJets.d0_leadtrack_lead", "Name of the DiTau dR between the leading track within the lead subjet with respect to the lead subjet"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_d0_leadtrack_sublead_DecorKey { this, "DiTauD0SubleadTrackLeadName",   "DiTauJets.d0_leadtrack_subl", "Name of the DiTau dR between the leading track within the sublead subjet with respect to the sublead subjet"};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer>      m_f_isotracks_DecorKey          { this, "DiTauFIsotracks",               "DiTauJets.f_isotracks",       "Name of the DiTau energy fraction carried by isolated tracks"};
};
