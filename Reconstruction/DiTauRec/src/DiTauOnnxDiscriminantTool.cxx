/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRec/DiTauOnnxDiscriminantTool.h"

// Core include(s):
#include "AthLinks/ElementLink.h"


// EDM include(s):




using TrackParticleLinks_t = std::vector<ElementLink<xAOD::TrackParticleContainer>>;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauOnnxDiscriminantTool::DiTauOnnxDiscriminantTool( const std::string& type, const std::string& name, const IInterface * parent) :
  DiTauToolBase(type, name, parent),
  m_onnxModelPath(""),
  m_maxTracks(10)
{
  declareInterface<DiTauToolBase > (this);
  declareProperty( "onnxModelPath", m_onnxModelPath = "TrigTauRec/00-11-02/dev/boosted_ditau_omni_model.onnx");
  declareProperty( "maxTracks", m_maxTracks = 10);
}

//______________________________________________________________________________
DiTauOnnxDiscriminantTool::~DiTauOnnxDiscriminantTool() = default;

//______________________________________________________________________________
StatusCode DiTauOnnxDiscriminantTool::initialize()
{
  ATH_MSG_INFO( "Initializing DiTauOnnxDiscriminantTool" );
  ATH_MSG_INFO( "onnxModelPath: " << m_onnxModelPath );
  auto model_path = PathResolverFindCalibFile (m_onnxModelPath);
  if (model_path.empty()) {
    ATH_MSG_ERROR("Could not find model file: " << m_onnxModelPath);
    return StatusCode::FAILURE;
  }
  m_ort_env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "OnnxUtil");
  Ort::SessionOptions session_options;
  session_options.SetIntraOpNumThreads(1);
  session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
  m_ort_session = std::make_unique<Ort::Session>(*m_ort_env, model_path.c_str(), session_options);
  return StatusCode::SUCCESS;
}

StatusCode DiTauOnnxDiscriminantTool::finalize()
{
  ATH_MSG_INFO( "Finalizing DiTauOnnxDiscriminantTool" );
  m_ort_session.reset();
  m_ort_env.reset();
  return StatusCode::SUCCESS;
}

StatusCode DiTauOnnxDiscriminantTool::execute(DiTauCandidateData * data, const EventContext& /*ctx*/) const
{
    static const SG::Accessor<float> omni_scoreDec("omni_score");
    xAOD::DiTauJet* xDitau = data->xAODDiTau;
    ATH_MSG_DEBUG("Inferencing omni DiTau ID score...");
    float score = GetDiTauObjOnnxScore(*xDitau);
    ATH_MSG_DEBUG("DiTau ID score: " << score);
    omni_scoreDec(*xDitau) = score;
    return StatusCode::SUCCESS;
}

std::vector<float> DiTauOnnxDiscriminantTool::flatten(const std::vector<std::vector<float>> &vec_2d) const{
  std::vector<float> flattened;
  flattened.reserve(vec_2d.size() * (vec_2d.empty() ? 0 : vec_2d[0].size()));
  for (const auto &inner : vec_2d) {
    flattened.insert(flattened.end(), inner.begin(), inner.end());
  }
  return flattened;
}

std::vector<float> DiTauOnnxDiscriminantTool::extract_points(const std::vector<std::vector<float>> &track_features) const{
  std::vector<float> points;
  points.reserve(track_features.size() * 2);
  for (const auto &track : track_features) {
    points.push_back(track[0]);  // delta_eta
    points.push_back(track[1]);  // delta_phi
  }
  return points;
}

std::vector<float> DiTauOnnxDiscriminantTool::create_mask(const std::vector<std::vector<float>> &track_features) const{
  std::vector<float> mask;
  mask.reserve(track_features.size());
  std::transform(track_features.begin(), track_features.end(), std::back_inserter(mask), [](const auto &track) {
        return std::abs(track[2]) > 1e-6 ? 1.0f : 0.0f;
  });
  return mask;
}

Ort::Value DiTauOnnxDiscriminantTool::create_tensor(std::vector<float> &data, const std::vector<int64_t> &shape) const{
  Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  return Ort::Value::CreateTensor<float>(memory_info, data.data(), data.size(),shape.data(), shape.size());
}

DiTauOnnxDiscriminantTool::InferenceOutput DiTauOnnxDiscriminantTool::run_inference(OnnxInputs &inputs) const{
  std::vector<Ort::Value> input_tensors;
  input_tensors.reserve(m_input_node_names.size());
  input_tensors.emplace_back(create_tensor(inputs.input_features, inputs.input_features_shape));
  input_tensors.emplace_back(create_tensor(inputs.input_points, inputs.input_points_shape));
  input_tensors.emplace_back(create_tensor(inputs.input_mask, inputs.input_mask_shape));
  input_tensors.emplace_back(create_tensor(inputs.input_jet, inputs.input_jet_shape));
  input_tensors.emplace_back(create_tensor(inputs.input_time, inputs.input_time_shape));

  std::vector<const char *> input_node_names;
  input_node_names.reserve(m_input_node_names.size());
  std::transform(m_input_node_names.begin(), m_input_node_names.end(), std::back_inserter(input_node_names), [](const std::string &name) { return name.c_str(); });

  std::vector<const char *> output_node_names;
  output_node_names.reserve(m_output_node_names.size());
  std::transform(m_output_node_names.begin(), m_output_node_names.end(), std::back_inserter(output_node_names), [](const std::string &name) { return name.c_str(); });

  auto output_tensors = m_ort_session->Run(Ort::RunOptions{nullptr}, input_node_names.data(), input_tensors.data(), input_node_names.size(), output_node_names.data(), output_node_names.size());

  InferenceOutput output;
  for (size_t i = 0; i < output_tensors.size(); ++i) {
    const auto &tensor = output_tensors[i];
    const size_t length = tensor.GetTensorTypeAndShapeInfo().GetElementCount();
    const float *data = tensor.GetTensorData<float>();
    (i == 0 ? output.output_1 : output.output_2) = std::vector<float>(data, data + length);
  } 
  return output;
}

float DiTauOnnxDiscriminantTool::GetDiTauObjOnnxScore(const xAOD::DiTauJet& ditau) const{
    static const SG::ConstAccessor<   float > ditau_ptAcc                 ("ditau_pt");
    static const SG::ConstAccessor<   float > f_core_leadAcc              ("f_core_lead");
    static const SG::ConstAccessor<   float > f_core_sublAcc              ("f_core_subl");
    static const SG::ConstAccessor<   float > f_subjet_sublAcc            ("f_subjet_subl");
    static const SG::ConstAccessor<   float > f_subjetsAcc                ("f_subjets");
    static const SG::ConstAccessor<   float > R_max_leadAcc               ("R_max_lead");
    static const SG::ConstAccessor<   float > R_max_sublAcc               ("R_max_subl");
    static const SG::ConstAccessor<     int > n_trackAcc                  ("n_track");
    static const SG::ConstAccessor<   float > R_isotrackAcc               ("R_isotrack");
    static const SG::ConstAccessor<   float > R_tracks_sublAcc            ("R_tracks_subl");
    static const SG::ConstAccessor<   float > M_core_leadAcc              ("m_core_lead");
    static const SG::ConstAccessor<   float > M_core_sublAcc              ("m_core_subl");
    static const SG::ConstAccessor<   float > M_tracks_leadAcc            ("m_tracks_lead");
    static const SG::ConstAccessor<   float > d0_leadtrack_leadAcc        ("d0_leadtrack_lead");
    static const SG::ConstAccessor<   float > d0_leadtrack_sublAcc        ("d0_leadtrack_subl");
    static const SG::ConstAccessor<   float > f_isotracksAcc              ("f_isotracks");
    static const SG::ConstAccessor< uint8_t > numberOfInrmstPxlLyrHitsAcc ("numberOfInnermostPixelLayerHits");
    static const SG::ConstAccessor< uint8_t > numberOfPixelHitsAcc        ("numberOfPixelHits");
    static const SG::ConstAccessor< uint8_t > numberOfSCTHitsAcc          ("numberOfSCTHits");
    static const SG::ConstAccessor<   float > z0Acc                       ("z0");
    static const SG::ConstAccessor<   float > d0Acc                       ("d0");
    // Input features for Ditau tagger ONNX model
    std::vector<float> jet_vars = {
        R_max_leadAcc                   (ditau),
        R_max_sublAcc                   (ditau),
        R_tracks_sublAcc                (ditau),
        R_isotrackAcc                   (ditau),
        d0_leadtrack_leadAcc            (ditau),
        d0_leadtrack_sublAcc            (ditau),
        f_core_leadAcc                  (ditau),
        f_core_sublAcc                  (ditau),
        f_subjet_sublAcc                (ditau),
        f_subjetsAcc                    (ditau),
        f_isotracksAcc                  (ditau),
        M_core_leadAcc                  (ditau),
        M_core_sublAcc                  (ditau),
        M_tracks_leadAcc                (ditau),
        static_cast<float>( n_trackAcc  (ditau)),
    };
    std::vector<int64_t> jet_shape = {1, static_cast<int64_t>(jet_vars.size())};

    const TrackParticleLinks_t &vTauTracks = ditau.trackLinks();
    std::vector<std::vector<float>> track_features(m_maxTracks, std::vector<float>(11, 0.0f));

    float jet_eta = ditau.eta();
    float jet_phi = ditau.phi();
    size_t num_tracks = std::min(static_cast<size_t>(m_maxTracks), vTauTracks.size());

    for (size_t i = 0; i < num_tracks; ++i) {
        const ElementLink<xAOD::TrackParticleContainer> &trackLink = vTauTracks[i];
        if (!trackLink.isValid()) continue;
        const xAOD::TrackParticle *xTrack = *trackLink;
        float track_eta    = xTrack->eta();
        float track_phi    = xTrack->phi();
        float delta_eta    = track_eta - jet_eta;
        float delta_phi    = std::remainder(track_phi - jet_phi, 2 * M_PI);
        float delta_R      = std::hypot(delta_eta, delta_phi);
        float track_pt     = static_cast<float>(xTrack->pt());
        float pt_log       = std::log(track_pt + 1e-8f);
        float jet_pt       = ditau_ptAcc(ditau);
        float pt_ratio     = track_pt / jet_pt;
        float pt_ratio_log = (pt_ratio <= 1.0f) ? std::log(1.0f - pt_ratio + 1e-8f) : 0.0f;
        float track_charge = xTrack->charge();

        track_features[i] = {
            delta_eta,
            delta_phi,
            pt_log,
            d0Acc(*xTrack),
            pt_ratio_log,
            z0Acc(*xTrack),
            delta_R,
            static_cast<float>(numberOfInrmstPxlLyrHitsAcc(*xTrack)),
            static_cast<float>(numberOfPixelHitsAcc(*xTrack)),
            static_cast<float>(numberOfSCTHitsAcc(*xTrack)),
            track_charge
        };
    }
    std::vector<int64_t> track_shape = {1, static_cast<int64_t>(m_maxTracks), 11};

    // Actual ONNX inference
    OnnxInputs inputs{
        flatten(track_features),
        track_shape,
        extract_points(track_features),
        {1, track_shape[1], 2},
        create_mask(track_features),
        {1, track_shape[1]},
        std::move(jet_vars),
        std::move(jet_shape),
        {0.0f},
        {1, 1}
    };
    auto output = run_inference(inputs);
    return output.output_1[1];
}
