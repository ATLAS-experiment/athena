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
  DiTauToolBase(type, name, parent)
{
  declareInterface<DiTauToolBase > (this);
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
  session_options.DisableCpuMemArena();
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

StatusCode DiTauOnnxDiscriminantTool::executeObj( xAOD::DiTauJet &xDiTau, const EventContext& /*ctx*/) const
{
    static const SG::Accessor<float> omni_scoreDec("omni_score");
    ATH_MSG_DEBUG("Inferencing omni DiTau ID score...");
    float score = GetDiTauObjOnnxScore(xDiTau);
    ATH_MSG_DEBUG("DiTau ID score: " << score);
    omni_scoreDec(xDiTau) = score;
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

    // do the calculation only for ditau with at least 2 subjets
    if(n_subjets(ditau)<2){
        return m_dDefault;
    }

    DitauTrackingInfo ditauTrackingInfo;
    if(!(getTrackingInfo(ditau, ditauTrackingInfo))){
        return m_dDefault;
    }	
	
    // Accessors for reading the necessary features from the xAOD::TrackParticle object
    static const SG::ConstAccessor< uint8_t > numberOfInrmstPxlLyrHitsAcc ("numberOfInnermostPixelLayerHits");
    static const SG::ConstAccessor< uint8_t > numberOfPixelHitsAcc        ("numberOfPixelHits");
    static const SG::ConstAccessor< uint8_t > numberOfSCTHitsAcc          ("numberOfSCTHits");
    static const SG::ConstAccessor<   float > z0Acc                       ("z0");
    static const SG::ConstAccessor<   float > d0Acc                       ("d0");
    // Input features for Ditau tagger ONNX model
    std::vector<float> jet_vars = {
        R_max(ditau, ditauTrackingInfo, 0), 
        R_max(ditau, ditauTrackingInfo, 1), 
        R_tracks(ditau, ditauTrackingInfo, 1),  
        R_isotrack(ditau, ditauTrackingInfo), 
        d0_leadtrack(ditau, ditauTrackingInfo, 0),
        d0_leadtrack(ditau, ditauTrackingInfo, 1), 	
        f_core(ditau,0),                
        f_core(ditau,1),                
        f_subjet(ditau,1),              
        f_subjets(ditau),               
        f_isotracks(ditau, ditauTrackingInfo), 
        mass_core(ditau, ditauTrackingInfo, 0), 
        mass_core(ditau, ditauTrackingInfo, 1), 
        mass_tracks(ditau, ditauTrackingInfo, 0), 
        static_cast<float>( n_track(ditau)),
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
        float jet_pt       = ditau_pt(ditau); //ditau_ptAcc(ditau);
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

// Aux variables calculation
int DiTauOnnxDiscriminantTool::n_subjets(const xAOD::DiTauJet& xDiTau) const {
    int nSubjet = 0;
    while (xDiTau.subjetPt(nSubjet) > 0. ){
        nSubjet++;
    }
    return nSubjet;
}

float DiTauOnnxDiscriminantTool::ditau_pt(const xAOD::DiTauJet& xDiTau) const
{
  return xDiTau.subjetPt(0)+xDiTau.subjetPt(1);
}

float DiTauOnnxDiscriminantTool::f_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const 
{
  return xDiTau.fCore(iSubjet);
}

float DiTauOnnxDiscriminantTool::f_subjet(const xAOD::DiTauJet& xDiTau, int iSubjet) const {
  return xDiTau.subjetPt(iSubjet) / xDiTau.pt();
}

float DiTauOnnxDiscriminantTool::f_subjets(const xAOD::DiTauJet& xDiTau) const
{
  return (xDiTau.subjetPt(0) + xDiTau.subjetPt(1))/ xDiTau.pt();
}

float DiTauOnnxDiscriminantTool::R_max(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const 
{
    const SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    float Rmax = 0;
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) {
        if (subjetInfo.subjet_p4.DeltaR(xTrack->p4()) > Rmax) {
            Rmax = subjetInfo.subjet_p4.DeltaR(xTrack->p4());
        }
    }
    return Rmax;
}

int DiTauOnnxDiscriminantTool::n_track(const xAOD::DiTauJet& xDiTau) const{
    return xDiTau.nTracks();
}

float DiTauOnnxDiscriminantTool::R_isotrack(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo) const
{
    float R_sum = 0;
    float pt = 0;
    for (int i = 0; i < 2; i++) {
        SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(i);
        for (const xAOD::TrackParticle* xTrack: subjetInfo.vIsoTracks) {
            R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4()) * xTrack->pt();
            pt += xTrack->pt();
        }
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;
}

float DiTauOnnxDiscriminantTool::R_tracks(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const { 
    float R_sum = 0;
    float pt = 0;

    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) {
        R_sum += subjetInfo.subjet_p4.DeltaR(xTrack->p4()) * xTrack->pt();
        pt += xTrack->pt();
    }
    if (pt == 0) {
        return m_dDefault;
    }
    return R_sum / pt;
}

float DiTauOnnxDiscriminantTool::mass_core(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const {
    TLorentzVector allCoreTracks_p4;
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vCoreTracks) {
        allCoreTracks_p4 += xTrack->p4();
    }
    float mass = allCoreTracks_p4.M();
    if (mass < 0) {
        return m_dDefault;
    }
    return mass;
}

float DiTauOnnxDiscriminantTool::mass_tracks(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const {
    TLorentzVector allTracks_p4;
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    for (const xAOD::TrackParticle* xTrack: subjetInfo.vTracks) {
        allTracks_p4 += xTrack->p4();
    }
    float mass = allTracks_p4.M();
    if (mass < 0) {
        return m_dDefault;
    }
    return mass;
}

float DiTauOnnxDiscriminantTool::d0_leadtrack(const xAOD::DiTauJet&, const DitauTrackingInfo& ditauInfo, int iSubjet) const {
    SubjetTrackingInfo subjetInfo = ditauInfo.vSubjetInfo.at(iSubjet);
    if (!subjetInfo.leadTrack) {
        return m_dDefault;
    }
    return subjetInfo.leadTrack->d0();
}

float DiTauOnnxDiscriminantTool::f_isotracks(const xAOD::DiTauJet& xDiTau, const DitauTrackingInfo& ditauInfo) const { 
    float iso_pt = 0;
    for (const xAOD::TrackParticle* xTrack: ditauInfo.vIsoTracks) {
        iso_pt += xTrack->pt();
    }
    if( xDiTau.pt() == 0.){
        return m_dDefault;
    } else {
        return iso_pt / xDiTau.pt();
    }
}

StatusCode DiTauOnnxDiscriminantTool::getTrackingInfo(const xAOD::DiTauJet& xDiTau, DitauTrackingInfo& trackingInfo) const {
    static const SG::ConstAccessor<std::vector<ElementLink<xAOD::TrackParticleContainer>>> trackLinksAcc("trackLinks");
    static const SG::ConstAccessor<std::vector<ElementLink<xAOD::TrackParticleContainer>>> isoTrackLinksAcc("isoTrackLinks");
    static const SG::ConstAccessor<float> R_subjetAcc("R_subjet");
    static const SG::ConstAccessor<float> R_coreAcc("R_core");
    

    if (!trackLinksAcc.isAvailable(xDiTau) || !isoTrackLinksAcc.isAvailable(xDiTau)) {
        ATH_MSG_WARNING("Track " << (!trackLinksAcc.isAvailable(xDiTau) ? "DiTauJet.trackLinks" : "DiTauJet.isoTrackLinks") << " links not available.");
        return StatusCode::FAILURE;
    } 

    int nSubjets = n_subjets(xDiTau);
    float Rsubjet = R_subjetAcc(xDiTau);
    float RCore = R_coreAcc(xDiTau);

    trackingInfo.nSubjets = nSubjets;
    trackingInfo.vSubjetInfo.clear();
    trackingInfo.vIsoTracks.clear();
    trackingInfo.vTracks.clear();
    
    // Get the track links from the DiTauJet and store them in the tracking info 
    std::vector<ElementLink<xAOD::TrackParticleContainer>> isoTrackLinks = xDiTau.isoTrackLinks();
    for (const auto &trackLink: isoTrackLinks) {
        if (!trackLink.isValid()) {
        ATH_MSG_WARNING("Iso track link is not valid");
        continue;
        }
        const xAOD::TrackParticle* xTrack = *trackLink;
        trackingInfo.vIsoTracks.push_back(xTrack);
    } 
    std::vector<ElementLink<xAOD::TrackParticleContainer>> trackLinks = xDiTau.trackLinks();
    for (const auto &trackLink : trackLinks)  {
        if (!trackLink.isValid()) {
        ATH_MSG_WARNING("track link is not valid");
        continue;
        }
        const xAOD::TrackParticle* xTrack = *trackLink;
        trackingInfo.vTracks.push_back(xTrack);
    } 
    // store subjet p4
    for (int i=0; i<nSubjets; ++i){
        SubjetTrackingInfo subjetTrackingInfo;
        TLorentzVector subjet_p4 = TLorentzVector();
        subjet_p4.SetPtEtaPhiE( xDiTau.subjetPt(i), xDiTau.subjetEta(i), xDiTau.subjetPhi(i), xDiTau.subjetE(i));
        subjetTrackingInfo.subjet_p4 = subjet_p4;
        trackingInfo.vSubjetInfo.push_back(subjetTrackingInfo);
    }
    for (const auto track : trackingInfo.vTracks) {
        float dRMin = 999;
        int inSubjet = -1;
        for (int i=0; i<nSubjets; ++i){
            float dRTrackSubjet = trackingInfo.vSubjetInfo[i].subjet_p4.DeltaR(track->p4());
            if (dRTrackSubjet < Rsubjet && dRTrackSubjet < dRMin){
                dRMin = dRTrackSubjet;
                inSubjet = i;
            }
        }
        if (inSubjet >= 0){
            trackingInfo.vSubjetInfo[inSubjet].vTracks.push_back(track);
        }
    }
    // find leading track in subjets
    for (int i=0; i<nSubjets; ++i){
        float ptLeadTrack = 0;
        for (const auto track : trackingInfo.vSubjetInfo[i].vTracks){
            if (track->pt() > ptLeadTrack){
                ptLeadTrack = track->pt();
                trackingInfo.vSubjetInfo[i].leadTrack = track;
            }
        }
    }
    // find core track in subjets
    for (int i=0; i<nSubjets; ++i){
        for (const auto track : trackingInfo.vSubjetInfo[i].vTracks){
            auto subjetTrackingInfo = trackingInfo.vSubjetInfo[i];
            if (subjetTrackingInfo.subjet_p4.DeltaR(track->p4()) < RCore){
                trackingInfo.vSubjetInfo[i].vCoreTracks.push_back(track);
            }
        }
    }
    //find isotracks in subjets
    for (const auto track : trackingInfo.vIsoTracks){
        float RIso = 0.4;
        float dRMin = 999;
        int inSubjet = -1;
        for (int i=0; i<nSubjets; ++i){
            float dRTrackSubjet = trackingInfo.vSubjetInfo[i].subjet_p4.DeltaR(track->p4());
            if (dRTrackSubjet > Rsubjet && dRTrackSubjet < RIso && dRTrackSubjet < dRMin){
                dRMin = dRTrackSubjet;
                inSubjet = i;
            }
        }
        if (inSubjet >= 0){
            trackingInfo.vSubjetInfo[inSubjet].vIsoTracks.push_back(track);
        }
    }
    return StatusCode::SUCCESS;
}



