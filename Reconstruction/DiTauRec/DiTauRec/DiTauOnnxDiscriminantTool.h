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
#include "AsgDataHandles/WriteDecorHandleKey.h"
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

  // calculate ID variables
  virtual StatusCode executeObj(xAOD::DiTauJet& xDiTau, const EventContext& ctx ) const override;  

  // calculate the score 
  float GetDiTauObjOnnxScore(const xAOD::DiTauJet& ditau) const;

private:
 
  float m_dDefault = -1234;

  struct SubjetTrackingInfo{
    TLorentzVector subjet_p4;
    std::vector<const xAOD::TrackParticle*> vTracks;
    std::vector<const xAOD::TrackParticle*> vIsoTracks;
    std::vector<const xAOD::TrackParticle*> vCoreTracks;
    const xAOD::TrackParticle* leadTrack = nullptr;
  };
  struct DitauTrackingInfo{
    std::vector<const xAOD::TrackParticle*> vTracks;
    std::vector<const xAOD::TrackParticle*> vIsoTracks;
    int nSubjets = 0;
    std::vector<SubjetTrackingInfo> vSubjetInfo;
  };

  int n_subjets         (const xAOD::DiTauJet& xDiTau) const;
  float ditau_pt        (const xAOD::DiTauJet& xDiTau) const;
  float f_core          (const xAOD::DiTauJet& xDiTau, int iSubjet) const;  
  float f_subjet        (const xAOD::DiTauJet& xDiTau, int iSubjet) const;
  float f_subjets       (const xAOD::DiTauJet& xDiTau) const; 
  float R_max           (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  int n_track           (const xAOD::DiTauJet& xDiTau) const;
  float R_isotrack      (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo) const; 
  float R_tracks        (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float mass_core       (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float mass_tracks     (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float d0_leadtrack    (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo, int iSubjet) const;
  float f_isotracks     (const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo) const;

  StatusCode getTrackingInfo(const xAOD::DiTauJet& xDiTau, DitauTrackingInfo& trackingInfo) const;

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
 
};


