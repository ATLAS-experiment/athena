/*  
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigADComboHypoTool.h"
#include "xAODJet/JetContainer.h"
#include "xAODJet/Jet.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/Muon.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJet.h"
#include "xAODTrigMissingET/TrigMissingETContainer.h"
#include "xAODTrigMissingET/TrigMissingET.h"
#include "PathResolver/PathResolver.h"
#include <Math/Vector2D.h>
#include <Math/Vector2Dfwd.h>

#include <cmath>

TrigADComboHypoTool::TrigADComboHypoTool(const std::string& type, const std::string& name, const IInterface* parent): ComboHypoToolBase(type, name, parent) {}

StatusCode TrigADComboHypoTool::initialize(){
  
  ATH_CHECK( m_svc.retrieve() );
  std::string model_file_name = PathResolverFindCalibFile(m_modelFileName);

  if (m_modelFileName.empty() || model_file_name.empty()) {
    ATH_MSG_ERROR("Could not find the requested ONNX model file: " << m_modelFileName);
    ATH_MSG_ERROR("Please make sure it exists in the ATLAS calibration area (https://atlas-groupdata.web.cern.ch/atlas-groupdata/), and provide a model file name relative to the root of the calibration area.");

    return StatusCode::FAILURE;
  }

  // initialise session
  Ort::SessionOptions session_options;
  Ort::AllocatorWithDefaultOptions allocator;
  session_options.SetIntraOpNumThreads(1);
  session_options.SetGraphOptimizationLevel(ORT_ENABLE_BASIC);

  m_session = std::make_unique<Ort::Session>(m_svc->env(), model_file_name.c_str(), session_options);

  ATH_MSG_INFO("Created ONNX runtime session with model " << model_file_name);

  size_t num_input_nodes = m_session->GetInputCount();
  m_input_node_names.resize(num_input_nodes);

  for (std::size_t i = 0; i < num_input_nodes; i++) {
      char* input_name = m_session->GetInputNameAllocated(i, allocator).release();
      ATH_MSG_DEBUG("Input " << i << " : " << " name= " << input_name);
      m_input_node_names[i] = input_name;

      Ort::TypeInfo type_info = m_session->GetInputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      ONNXTensorElementDataType type = tensor_info.GetElementType();
      ATH_MSG_DEBUG("Input " << i << " : " << " type= " << type);

      m_input_node_dims = tensor_info.GetShape();
      ATH_MSG_DEBUG("Input " << i << " : num_dims= " << m_input_node_dims.size());
      for (std::size_t j = 0; j < m_input_node_dims.size(); j++) {
          if (m_input_node_dims[j] < 0) m_input_node_dims[j] = 1;
          ATH_MSG_DEBUG("Input " << i << " : dim " << j << "= " << m_input_node_dims[j]);
      }
  }

  std::vector<int64_t> output_node_dims;
  size_t num_output_nodes = m_session->GetOutputCount();
  ATH_MSG_DEBUG("Have output nodes " << num_output_nodes);
  m_output_node_names.resize(num_output_nodes);

  for (std::size_t i = 0; i < num_output_nodes; i++) {
      char* output_name = m_session->GetOutputNameAllocated(i, allocator).release();
      ATH_MSG_DEBUG("Output " << i << " : " << " name= " << output_name);
      m_output_node_names[i] = output_name;

      Ort::TypeInfo type_info = m_session->GetOutputTypeInfo(i);
      auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
      ONNXTensorElementDataType type = tensor_info.GetElementType();
      ATH_MSG_DEBUG("Output " << i << " : " << " type= " << type);

      output_node_dims = tensor_info.GetShape();
      ATH_MSG_DEBUG("Output " << i << " : num_dims= " << output_node_dims.size());
      for (std::size_t j = 0; j < output_node_dims.size(); j++) {
          if (output_node_dims[j] < 0) output_node_dims[j] = 1;
          ATH_MSG_DEBUG("Output" << i << " : dim " << j << "= " << output_node_dims[j]);
      }
  }
  
  
  return StatusCode::SUCCESS;
}

StatusCode TrigADComboHypoTool::decide(Combo::LegDecisionsMap& passingLegs, const EventContext& /*context*/) const{

  ATH_MSG_DEBUG("Size of passingLegs = " << passingLegs.size());

  if (passingLegs.size() == 0) { // if no combinations passed, then exit 
    return StatusCode::SUCCESS;
  }

  std::vector<std::vector<Combo::LegDecision>> legDecisions;
  ATH_CHECK(selectLegs(passingLegs, legDecisions));
  
  // get the lists of each object
  ATH_MSG_DEBUG("Have "<<passingLegs.size()<<" passing legs in AD");
  
  std::vector<const xAOD::Jet*> input_jets;
  std::map<const xAOD::Jet*, std::vector<Combo::LegDecision>> jet_decisions;
  std::vector<const xAOD::Electron*> input_electrons;
  std::map<const xAOD::Electron*, std::vector<Combo::LegDecision>> ele_decisions;
  std::vector<const xAOD::Muon*> input_muons;
  std::map<const xAOD::Muon*, std::vector<Combo::LegDecision>> muon_decisions;
  std::vector<const xAOD::Photon*> input_photons;
  std::map<const xAOD::Photon*, std::vector<Combo::LegDecision>> gam_decisions;
  std::vector<const xAOD::TauJet*> input_taus;
  std::map<const xAOD::TauJet*, std::vector<Combo::LegDecision>> taujet_decisions;
  std::vector<const xAOD::TrigMissingET*> input_mets;
  std::map<const xAOD::TrigMissingET*, std::vector<Combo::LegDecision>> met_decisions;
  
  for(const auto &leg_decs : legDecisions){ // loop over each leg
    for(const auto &dec_pair : leg_decs){ // loop over each object in a leg
      const TrigCompositeUtils::Decision* decision(*(dec_pair.second));
      std::vector<TrigCompositeUtils::LinkInfo<xAOD::JetContainer>> jet_feature_links = TrigCompositeUtils::findLinks<xAOD::JetContainer>(decision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      std::vector<TrigCompositeUtils::LinkInfo<xAOD::ElectronContainer>> ele_feature_links = TrigCompositeUtils::findLinks<xAOD::ElectronContainer>(decision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      std::vector<TrigCompositeUtils::LinkInfo<xAOD::MuonContainer>> muon_feature_links = TrigCompositeUtils::findLinks<xAOD::MuonContainer>(decision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      std::vector<TrigCompositeUtils::LinkInfo<xAOD::PhotonContainer>> gam_feature_links = TrigCompositeUtils::findLinks<xAOD::PhotonContainer>(decision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      std::vector<TrigCompositeUtils::LinkInfo<xAOD::TauJetContainer>> taujet_feature_links = TrigCompositeUtils::findLinks<xAOD::TauJetContainer>(decision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      std::vector<TrigCompositeUtils::LinkInfo<xAOD::TrigMissingETContainer>> met_feature_links = TrigCompositeUtils::findLinks<xAOD::TrigMissingETContainer>(decision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      if(jet_feature_links.size()==1){
	const TrigCompositeUtils::LinkInfo<xAOD::JetContainer> jet_feature_link = jet_feature_links.at(0);
	ATH_CHECK(jet_feature_link.isValid());
	const xAOD::Jet* jet = *(jet_feature_link.link); 
	jet_decisions[jet].push_back(dec_pair);
      }
      if(ele_feature_links.size()==1){
	const TrigCompositeUtils::LinkInfo<xAOD::ElectronContainer> ele_feature_link = ele_feature_links.at(0);
	ATH_CHECK(ele_feature_link.isValid());
	const xAOD::Electron* electron = *(ele_feature_link.link); 
	ele_decisions[electron].push_back(dec_pair);
      }
      if(muon_feature_links.size()==1){
	const TrigCompositeUtils::LinkInfo<xAOD::MuonContainer> muon_feature_link = muon_feature_links.at(0);
	ATH_CHECK(muon_feature_link.isValid());
	const xAOD::Muon* muon = *(muon_feature_link.link); 
	muon_decisions[muon].push_back(dec_pair);
      }
      if(gam_feature_links.size()==1){
	const TrigCompositeUtils::LinkInfo<xAOD::PhotonContainer> gam_feature_link = gam_feature_links.at(0);
	ATH_CHECK(gam_feature_link.isValid());
	const xAOD::Photon* photon = *(gam_feature_link.link); 
	gam_decisions[photon].push_back(dec_pair);
      }
      if(taujet_feature_links.size()==1){
	const TrigCompositeUtils::LinkInfo<xAOD::TauJetContainer> taujet_feature_link = taujet_feature_links.at(0);
	ATH_CHECK(taujet_feature_link.isValid());
	const xAOD::TauJet* taujet = *(taujet_feature_link.link); 
	taujet_decisions[taujet].push_back(dec_pair);
      }
      if(met_feature_links.size()==1){
	const TrigCompositeUtils::LinkInfo<xAOD::TrigMissingETContainer> met_feature_link = met_feature_links.at(0);
	ATH_CHECK(met_feature_link.isValid());
	const xAOD::TrigMissingET* met = *(met_feature_link.link); 
	met_decisions[met].push_back(dec_pair);
      }
    }
  }

  for(const auto &pair : jet_decisions){
    input_jets.push_back(pair.first);
  }
  if(input_jets.size()>1){
    std::sort(input_jets.begin(), input_jets.end(),
	      [](const auto a, const auto b){
		return a->pt() > b->pt();
	      });
  }
  
  for(const auto &pair : ele_decisions){
    input_electrons.push_back(pair.first);
  }
  if(input_electrons.size()>1){
    std::sort(input_electrons.begin(), input_electrons.end(),
	      [](const auto a, const auto b){
		return a->pt() > b->pt();
	      });
  }
  
  for(const auto &pair : muon_decisions){
    input_muons.push_back(pair.first);
  }
  if(input_muons.size()>1){
    std::sort(input_muons.begin(), input_muons.end(),
	      [](const auto a, const auto b){
		return a->pt() > b->pt();
	      });
  }
  
  for(const auto &pair : gam_decisions){
    input_photons.push_back(pair.first);
  }
  if(input_photons.size()>1){
    std::sort(input_photons.begin(), input_photons.end(),
	      [](const auto a, const auto b){
		return a->pt() > b->pt();
	      });
  }
  
  for(const auto &pair : taujet_decisions){
    input_taus.push_back(pair.first);
  }
  if(input_taus.size()>1){
    std::sort(input_taus.begin(), input_taus.end(),
	      [](const auto a, const auto b){
		return a->pt() > b->pt();
	      });
  }
  
  for(const auto &pair : met_decisions){
    input_mets.push_back(pair.first);
  }
  
  bool trigPass = this->getAdDecision(input_jets, input_muons, input_electrons, input_photons, input_taus, input_mets);

  if(!trigPass){
    eraseFromLegDecisionsMap(passingLegs);
  }
  
  return StatusCode::SUCCESS;	
}

bool TrigADComboHypoTool::getAdDecision(
  const std::vector<const xAOD::Jet*> &input_jets,
  const std::vector<const xAOD::Muon*> &input_muons,
  const std::vector<const xAOD::Electron*> &input_electrons,
  const std::vector<const xAOD::Photon*> &input_photons,
  const std::vector<const xAOD::TauJet*> &input_taus,
  const std::vector<const xAOD::TrigMissingET*> &input_mets) const{

  ATH_MSG_DEBUG( "Counting AD input objects in the event ... "
		 << "Jets: " << input_jets.size() << ", "
		 << "Muons: " << input_muons.size() << ", "
		 << "Electrons: " << input_electrons.size() << ", "
		 << "Photons: " << input_photons.size() << ", "
		 << "TauJets: " << input_taus.size() << ", "
		 << "METs: " << input_mets.size());

  // pt1 eta1 phi1 pt2 eta2 phi2 ... for 6 jets, 3 electrons, 3 muons, 3 photons, and MET
  unsigned int metind = (m_maxjs.value()+m_maxes.value()+m_maxms.value()+m_maxgs.value())*3;
  std::vector<float> inputTensor;

  unsigned int jet_count = 0;
  for(const auto &jet : input_jets){
    ATH_MSG_DEBUG( std::setprecision(3) << std::fixed
		   << "jet[" << jet_count << "] = ("
		   << jet->pt()/1000 << ", "
		   << jet->eta() << ", "
		   << jet->phi() << ", "
		   << jet->m()/1000 << ")");
    if (jet_count<m_maxjs.value()) {
      inputTensor.insert(inputTensor.end(), {static_cast<float>(jet->pt()/1000), static_cast<float>(jet->eta()), static_cast<float>(jet->phi())});
      jet_count++;
    }
  }
  inputTensor.insert(inputTensor.end(), 3*(m_maxjs.value()-jet_count), 0.);

  unsigned int ele_count = 0;
  for(const auto &ele : input_electrons){
    ATH_MSG_DEBUG( std::setprecision(3) << std::fixed
		   << "ele[" << ele_count << "] = ("
		   << ele->pt()/1000 << ", "
		   << ele->eta() << ", "
		   << ele->phi() << ", "
		   << ele->m()/1000 << ")");
    if (ele_count<m_maxes.value()) {
      inputTensor.insert(inputTensor.end(), {static_cast<float>(ele->pt()/1000), static_cast<float>(ele->eta()), static_cast<float>(ele->phi())});
      ele_count++;
    }
  }
  inputTensor.insert(inputTensor.end(), 3*(m_maxes.value()-ele_count), 0.);

  unsigned int muon_count = 0;
  for(const auto &muon : input_muons){
    ATH_MSG_DEBUG( std::setprecision(3) << std::fixed
		   << "muon[" << muon_count << "] = ("
		   << muon->pt()/1000 << ", "
		   << muon->eta() << ", "
		   << muon->phi() << ", "
		   << muon->m()/1000 << ")");
    if (muon_count<m_maxms.value()) {
      inputTensor.insert(inputTensor.end(), {static_cast<float>(muon->pt()/1000), static_cast<float>(muon->eta()), static_cast<float>(muon->phi())});
      muon_count++;
    }
  }
  inputTensor.insert(inputTensor.end(), 3*(m_maxms.value()-muon_count), 0.);
	
  unsigned int gam_count = 0;
  for(const auto &gam : input_photons){
    ATH_MSG_DEBUG( std::setprecision(3) << std::fixed
		   << "gam[" << gam_count << "] = ("
		   << gam->pt()/1000 << ", "
		   << gam->eta() << ", "
		   << gam->phi() << ", "
		   << gam->m()/1000 << ")");
    if (gam_count<m_maxgs.value()) {
      inputTensor.insert(inputTensor.end(), {static_cast<float>(gam->pt()/1000), static_cast<float>(gam->eta()), static_cast<float>(gam->phi())});
      gam_count++;
    }
  }
  inputTensor.insert(inputTensor.end(), 3*(m_maxgs.value()-gam_count), 0.);

  inputTensor.insert(inputTensor.end(), {0., 0., 0.});
  for(const auto &met : input_mets){
    ROOT::Math::XYVectorF metv(met->ex(),met->ey());
    float met_phi = metv.phi();
    float met_et = metv.r();

    ATH_MSG_DEBUG( std::setprecision(3) << std::fixed
		   << "MET = ("
		   << met_et/1000 << ", "
		   << met_phi << ")");
    inputTensor[metind] = met_et/1000;
    inputTensor[metind+2] = met_phi;
  }
  
  ATH_MSG_DEBUG("inputTensor size = " << inputTensor.size());
  if(msgLvl(MSG::DEBUG)){
    for (unsigned int i=0; i<inputTensor.size(); i++){
      ATH_MSG_DEBUG("inputTensor[" << i << "] = " << inputTensor[i]);
    }
  }

  float outputScore = runInference(inputTensor);
  ATH_MSG_DEBUG("Computed TrigADScore: " << outputScore);

  bool trigPass = (outputScore > m_adScoreThres);

  return trigPass;
}

float TrigADComboHypoTool::runInference(std::vector<float> &tensor) const {
  
    ATH_MSG_DEBUG("in TrigADComboHypoTool::runInference()");

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    int input_tensor_size = (m_maxjs.value()+m_maxes.value()+m_maxms.value()+m_maxgs.value()+1)*3;
    Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, tensor.data(), input_tensor_size, m_input_node_dims.data(), m_input_node_dims.size());

    // Ort::Session::Run is non-const.
    // However, the onnx authors claim that it is safe to call from multiple threads:
    // https://github.com/Microsoft/onnxruntime/issues/114
    Ort::Session* session ATLAS_THREAD_SAFE = m_session.get();
    auto output_tensors = session->Run(Ort::RunOptions{nullptr}, m_input_node_names.data(), &input_tensor, m_input_node_names.size(), m_output_node_names.data(), m_output_node_names.size());

    float *output_score_array = output_tensors.front().GetTensorMutableData<float>();
    unsigned int output_size = output_tensors.front().GetTensorTypeAndShapeInfo().GetElementCount();

    float output_score = 0.;
    if(output_size!=1){
      ATH_MSG_ERROR("Invalid output tensor size: " << output_size);
    }else{
      output_score = output_score_array[0];
    }

    return output_score;
}
