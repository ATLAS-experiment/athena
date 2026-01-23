/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Hagen Möbius, hagen.mobius@cern.ch
#include <FTagAnalysisAlgorithms/SSVWeightsAlg.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <PathResolver/PathResolver.h>
#include <boost/math/distributions/poisson.hpp>
using json = nlohmann::json;

namespace CP{
  SSVWeightsAlg::SSVWeightsAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : EL::AnaAlgorithm(name, pSvcLocator){
  }

  StatusCode SSVWeightsAlg::initialize() {
    ANA_MSG_INFO("Initialising SSVWeightsAlg");
    ANA_MSG_INFO("WARNING: The Run3 SSV calibration has not been performed yet -> the scale factors are not usable yet");

    ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_electronsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_muonsHandle.initialize(m_systematicsList));
    ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
    ANA_CHECK(m_truthParticlesHandle.initialize(m_systematicsList));
    ANA_CHECK(m_SSV_weight_decor.initialize(m_systematicsList, m_eventInfoHandle));
    ANA_CHECK(m_ssvHandle.initialize(m_systematicsList));
    ANA_CHECK(m_jetSelection.initialize (m_systematicsList, m_jetsHandle, SG::AllowEmpty));
    ANA_CHECK(m_electronSelection.initialize (m_systematicsList, m_electronsHandle, SG::AllowEmpty));
    ANA_CHECK(m_muonSelection.initialize (m_systematicsList, m_muonsHandle, SG::AllowEmpty));

    if (m_OutputVariableSize == "standard") {
      m_OutputVariableSizeType = OutputVariableSizeType::standard;
    }
    else if (m_OutputVariableSize == "extended") {
      m_OutputVariableSizeType = OutputVariableSizeType::extended;
    }
    else if (m_OutputVariableSize == "additional") {
      m_OutputVariableSizeType = OutputVariableSizeType::additional;
    }
    else if (m_OutputVariableSize == "all") {
      m_OutputVariableSizeType = OutputVariableSizeType::all;
    }
    else {
      ATH_MSG_ERROR("Unknown OutputVariableSizeType: " << m_OutputVariableSize <<" , accepted options are: 'standard', 'extended', 'additional', 'all'" );
      return StatusCode::FAILURE;
    }

    if (m_OutputVariableSizeType == OutputVariableSizeType::extended || m_OutputVariableSizeType == OutputVariableSizeType::additional || m_OutputVariableSizeType == OutputVariableSizeType::all){
      ANA_CHECK(m_P_eff_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_P_ineff_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_P_fake_decor.initialize(m_systematicsList, m_eventInfoHandle));
    }

    if ( m_OutputVariableSizeType == OutputVariableSizeType::additional || m_OutputVariableSizeType == OutputVariableSizeType::all){
      ANA_CHECK(m_N_matched_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_N_missed_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_N_fake_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_number_of_bjets_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_number_of_accepted_Bhadrons_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_number_of_good_SSVs_decor.initialize(m_systematicsList, m_eventInfoHandle));
    }

    if (m_OutputVariableSizeType == OutputVariableSizeType::all){
      ANA_CHECK(m_P_ineff_bjet_based_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_P_ineff_pt_eta_based_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_P_fake_pileup_bjet_based_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_P_fake_pileup_based_linearfit_decor.initialize(m_systematicsList, m_eventInfoHandle));
      ANA_CHECK(m_P_fake_pileup_based_binned_decor.initialize(m_systematicsList, m_eventInfoHandle));
    }

    ANA_CHECK(m_systematicsList.initialize());

    //retrieve the JSON file
    std::string json_file_SSVWeightsAlg = PathResolverFindCalibFile(m_jsonConfigPath_SSVWeightsAlg);
    std::ifstream jsonFile_SSVWeightsAlg(json_file_SSVWeightsAlg);
    if (!jsonFile_SSVWeightsAlg.is_open()) {
      ATH_MSG_ERROR("Could not open JSON file: " << m_jsonConfigPath_SSVWeightsAlg);
      return StatusCode::FAILURE;
    }

    m_jsonConfig_SSVWeightsAlg = json::parse(jsonFile_SSVWeightsAlg);
    jsonFile_SSVWeightsAlg.close();

    // Check that b-tagging working point is the same as in the calibration
    if (m_BTaggingWP.value() != m_jsonConfig_SSVWeightsAlg["CalibrationInformation"]["btaggingWP"].get<std::string>()){
      ANA_MSG_ERROR("WARNING: You are using b-tagging working point: "<< m_BTaggingWP.value() <<" , which is different to the one used in the SSV Calibration: " << m_jsonConfig_SSVWeightsAlg["CalibrationInformation"]["btaggingWP"].get<std::string>());
      return StatusCode::FAILURE;
    }

    // retrieve scale factors
    m_SF_eff = m_jsonConfig_SSVWeightsAlg["CalibrationScaleFactors"]["SF_eff"];
    m_SF_fake_low = m_jsonConfig_SSVWeightsAlg["CalibrationScaleFactors"]["SF_fake"]["mu_low"];
    m_SF_fake_high = m_jsonConfig_SSVWeightsAlg["CalibrationScaleFactors"]["SF_fake"]["mu_high"];

    // Initialize EfficiencyMethodClass
    if (m_EfficiencyMethod == "bjet_based") {
      m_EfficiencyMethodType = EfficiencyMethodType::bjet_based;
      m_EfficiencyMethodBJetBasedPtr = std::make_unique<EfficiencyMethodBJetBasedClass>( m_jsonConfig_SSVWeightsAlg );
    }
    else if (m_EfficiencyMethod == "Bhadron_pT_eta_based") {
      m_EfficiencyMethodType = EfficiencyMethodType::Bhadron_pT_eta_based;
      m_EfficiencyMethodBhadronPtEtaBasedPtr = std::make_unique<EfficiencyMethodBhadronPtEtaBasedClass>( m_jsonConfig_SSVWeightsAlg );
    }
    else {
      ATH_MSG_ERROR("Unknown efficiency method: " << m_EfficiencyMethod << " , accepted efficiency methods are: 'bjet_based','Bhadron_pT_eta_based'");
      return StatusCode::FAILURE;
    }

    //Initialize nFMethodClass
    if (m_nFMethod == "pileup_bjet_based") {
      m_nFMethodType = nFMethodType::pileup_bjet_based;
      m_nFPileupBJetBasedPtr = std::make_unique<nFMethodPileupBJetBasedClass>( m_jsonConfig_SSVWeightsAlg );
    }
    else if (m_nFMethod == "pileup_based_linearfit") {
      m_nFMethodType = nFMethodType::pileup_based_linearfit;
      m_nFPileupBasedLinearFitPtr = std::make_unique<nFMethodPileupBasedLinearFitClass>( m_jsonConfig_SSVWeightsAlg );
    }
    else if (m_nFMethod == "pileup_based_binned") {
      m_nFMethodType = nFMethodType::pileup_based_binned;
      m_nFPileupBasedBinnedPtr = std::make_unique<nFMethodPileupBasedBinnedClass>( m_jsonConfig_SSVWeightsAlg );
    }
    else {
      ATH_MSG_ERROR("Unknown nF method: " << m_nFMethod << " , accepted nF methods are: 'pileup_bjet_based', 'pileup_based_linearfit', 'pileup_based_binned'");
      return StatusCode::FAILURE;
    }
    // If OutputVariableSize = all -> Initialize all EfficiencyMethods and nFMethods
    // Also check if pointers have already been created (see code just above) 
    // as depending on the user method settings some of them might have been set already 
    if ( m_OutputVariableSizeType == OutputVariableSizeType::all ){
      if (!m_EfficiencyMethodBJetBasedPtr){
        m_EfficiencyMethodBJetBasedPtr = std::make_unique<EfficiencyMethodBJetBasedClass>( m_jsonConfig_SSVWeightsAlg );
      }
      if (!m_EfficiencyMethodBhadronPtEtaBasedPtr){
        m_EfficiencyMethodBhadronPtEtaBasedPtr = std::make_unique<EfficiencyMethodBhadronPtEtaBasedClass>( m_jsonConfig_SSVWeightsAlg );
      }
      if (!m_nFPileupBJetBasedPtr){
        m_nFPileupBJetBasedPtr = std::make_unique<nFMethodPileupBJetBasedClass>( m_jsonConfig_SSVWeightsAlg );
      }
      if (!m_nFPileupBasedLinearFitPtr){
        m_nFPileupBasedLinearFitPtr = std::make_unique<nFMethodPileupBasedLinearFitClass>( m_jsonConfig_SSVWeightsAlg );
      }
      if (!m_nFPileupBasedBinnedPtr){
        m_nFPileupBasedBinnedPtr = std::make_unique<nFMethodPileupBasedBinnedClass>( m_jsonConfig_SSVWeightsAlg );
      }
    }


    return StatusCode::SUCCESS;
  }

  StatusCode SSVWeightsAlg::execute() {
  
    for (const auto &sys : m_systematicsList.systematicsVector()){ 
      const xAOD::EventInfo *evtInfo = nullptr;
      ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

      const xAOD::VertexContainer* vertices = nullptr;
      ANA_CHECK(m_ssvHandle.retrieve(vertices, sys));

      // create SSVs
      std::vector<const xAOD::Vertex*> SSVs;
      for(const xAOD::Vertex* ssvvtx : *vertices){
        SSVs.push_back(ssvvtx);
      }

      //create jets
      const xAOD::JetContainer *jets = nullptr;
      ANA_CHECK(m_jetsHandle.retrieve(jets, sys));

      std::vector<const xAOD::Jet*> jets_Selected;
      int b_jet_count=0;
      static const SG::AuxElement::ConstAccessor<char> jet_btag_accessor(m_BTaggingWP);

      //create jets that pass your jet selection
      for(const xAOD::Jet* jet : *jets){
        if (m_jetSelection.getBool (*jet, sys)){
          jets_Selected.push_back(jet);
          
          // Count number of bjets
          if (jet_btag_accessor(*jet)){
            b_jet_count = b_jet_count+1;
          }
        }
      }

      // create electrons
      const xAOD::ElectronContainer *electrons = nullptr;
      ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
      
      std::vector<const xAOD::Electron*> electrons_Selected;

      //create electrons that pass your electron selection
      for(const xAOD::Electron* electron : *electrons){
        if (m_electronSelection.getBool (*electron, sys)){
          electrons_Selected.push_back (electron);
        }
      }

      //create muons
      const xAOD::MuonContainer *muons = nullptr;
      ANA_CHECK(m_muonsHandle.retrieve(muons, sys));
      std::vector<const xAOD::Muon*> muons_Selected;

      //create muons that pass your muon selection
      for(const xAOD::Muon* muon : *muons){
        if (m_muonSelection.getBool (*muon, sys)){
          muons_Selected.push_back( muon );
        }
      }

      // create good SSVs
      std::vector<const xAOD::Vertex*> good_SSVs = create_good_SSVs(jets_Selected, electrons_Selected, muons_Selected, SSVs);

      //create truth b-hadrons (truthBhs)
      std::vector<const xAOD::TruthParticle*> truthBhs;

      const xAOD::TruthParticleContainer *particles = nullptr;
      ANA_CHECK(m_truthParticlesHandle.retrieve(particles, sys));

      for (const xAOD::TruthParticle *part : *particles){
        if ( part->isBottomHadron() && isHFHadronFinalState(part, 5) ){ 
          truthBhs.push_back(part);
        }
      }


      //create truthBhs in acceptance
      std::vector<const xAOD::TruthParticle*> accepted_truthBhs = create_accepted_truthBhs(truthBhs, jets_Selected);

      //do the DeltaR matching between truthBh and SSV
      std::vector<bool> truthBh_to_SSV_matched = truthBh_to_SSV_matching(accepted_truthBhs, good_SSVs);

      //count matched truthBh,missed truthBh (not matched truthBh) and number of fake SSV (not matched SSV)
      int N_matched = count_matched_objects(truthBh_to_SSV_matched);
      int N_missed = count_not_matched_objects(truthBh_to_SSV_matched);
      int N_fake = count_number_of_fake_SSVs(accepted_truthBhs, good_SSVs);

      // retrieve pileup
      double muactual = evtInfo->actualInteractionsPerCrossing();

      // calculate P_eff
      double P_eff = std::pow(m_SF_eff, N_matched);

      //calculate P_ineff according to the chosen method
      double P_ineff = 1;
      if (m_EfficiencyMethodType == EfficiencyMethodType::bjet_based){
        P_ineff = m_EfficiencyMethodBJetBasedPtr->getPIneff(b_jet_count, N_missed,m_SF_eff);
      }
      else if (m_EfficiencyMethodType == EfficiencyMethodType::Bhadron_pT_eta_based){
        P_ineff = m_EfficiencyMethodBhadronPtEtaBasedPtr->getPIneff(accepted_truthBhs, truthBh_to_SSV_matched, m_SF_eff);
      }
      else {
        ATH_MSG_ERROR("Unknown efficiency method: " << m_EfficiencyMethod << " , accepted efficiency methods are: 'bjet_based','Bhadron_pT_eta_based'");
        return StatusCode::FAILURE;
      } 

      // calculate P_fake according to the chosen method
      double P_fake = 1;
      if (m_nFMethodType == nFMethodType::pileup_bjet_based){
        P_fake = m_nFPileupBJetBasedPtr->getPFake(muactual, b_jet_count, N_fake,m_SF_fake_low, m_SF_fake_high);
      }
      else if (m_nFMethodType == nFMethodType::pileup_based_linearfit){
        P_fake = m_nFPileupBasedLinearFitPtr->getPFake(muactual,N_fake);
      }
      else if (m_nFMethodType == nFMethodType::pileup_based_binned){
        P_fake = m_nFPileupBasedBinnedPtr->getPFake(muactual, N_fake, m_SF_fake_low, m_SF_fake_high);
      }
      else { 
        ATH_MSG_ERROR("Unknown nF method: " << m_nFMethod << " , accepted nF methods are: 'pileup_bjet_based', 'pileup_based_linearfit', 'pileup_based_binned'");
        return StatusCode::FAILURE;
      }

      //calculate SSV_weight
      double SSV_weight = P_eff * P_ineff * P_fake;

      // decorate SSV weight
      m_SSV_weight_decor.set(*evtInfo, SSV_weight, sys);

      if (m_OutputVariableSizeType == OutputVariableSizeType::extended || m_OutputVariableSizeType == OutputVariableSizeType::additional || m_OutputVariableSizeType == OutputVariableSizeType::all){
        // decorate P factors 
        m_P_eff_decor.set(*evtInfo, P_eff, sys);
        m_P_ineff_decor.set(*evtInfo, P_ineff, sys);
        m_P_fake_decor.set(*evtInfo, P_fake, sys);
      }
      if (m_OutputVariableSizeType == OutputVariableSizeType::additional || m_OutputVariableSizeType == OutputVariableSizeType::all){
        //decorate additional information
        m_N_matched_decor.set(*evtInfo, N_matched, sys);
        m_N_missed_decor.set(*evtInfo, N_missed, sys);
        m_N_fake_decor.set(*evtInfo, N_fake, sys);
        m_number_of_bjets_decor.set(*evtInfo, b_jet_count, sys);
        m_number_of_accepted_Bhadrons_decor.set(*evtInfo, accepted_truthBhs.size(), sys);
        m_number_of_good_SSVs_decor.set(*evtInfo, good_SSVs.size(), sys);
      }
      //decorate all possible P factors
      if (m_OutputVariableSizeType == OutputVariableSizeType::all){
        double P_ineff_bjet_based = m_EfficiencyMethodBJetBasedPtr->getPIneff(b_jet_count, N_missed,m_SF_eff);
        double P_ineff_pt_eta_based = m_EfficiencyMethodBhadronPtEtaBasedPtr->getPIneff(accepted_truthBhs, truthBh_to_SSV_matched, m_SF_eff);
        double P_fake_pileup_bjet_based = m_nFPileupBJetBasedPtr->getPFake(muactual, b_jet_count, N_fake, m_SF_fake_low, m_SF_fake_high);
        double P_fake_pileup_based_linearfit = m_nFPileupBasedLinearFitPtr->getPFake(muactual,N_fake);
        double P_fake_pileup_based_binned = m_nFPileupBasedBinnedPtr->getPFake(muactual, N_fake, m_SF_fake_low, m_SF_fake_high);

        m_P_ineff_bjet_based_decor.set(*evtInfo, P_ineff_bjet_based, sys);
        m_P_ineff_pt_eta_based_decor.set(*evtInfo, P_ineff_pt_eta_based, sys);
        m_P_fake_pileup_bjet_based_decor.set(*evtInfo, P_fake_pileup_bjet_based, sys);
        m_P_fake_pileup_based_linearfit_decor.set(*evtInfo, P_fake_pileup_based_linearfit, sys);
        m_P_fake_pileup_based_binned_decor.set(*evtInfo, P_fake_pileup_based_binned, sys);
      }
    }
    return StatusCode::SUCCESS;
  }

  // create vector that indicates which SSV is a so-called good SSV
  std::vector<const xAOD::Vertex*> SSVWeightsAlg::create_good_SSVs(
    const std::vector<const xAOD::Jet*> &jets,
    const std::vector<const xAOD::Electron*> &electrons,
    const std::vector<const xAOD::Muon*> &muons,
    const std::vector<const xAOD::Vertex*> &SSVs) const {

    static const SG::AuxElement::ConstAccessor<float> ssv_pt_accessor(("bvrtPt"));
    static const SG::AuxElement::ConstAccessor<float> ssv_m_accessor("bvrtM");
    static const SG::AuxElement::ConstAccessor<float> ssv_eta_accessor("bvrtEta");

    std::vector<const xAOD::Vertex*> good_SSVs;

    for (const xAOD::Vertex* SSV : SSVs) {
      bool overlaps = false;

      //check if SSV fails good SSV definition
      if ( (ssv_pt_accessor(*SSV) < 3000) || (ssv_m_accessor(*SSV) < 600) || (std::abs(ssv_eta_accessor(*SSV)) > 2.5) ){
        continue;
      }

      //check if SSV overlaps with jet
      for (const xAOD::Jet* jet : jets) {
        double DeltaR_jet = compute_DeltaR_between_SSV_and_particle( SSV , jet );
        if (DeltaR_jet < 0.6){
          overlaps = true;
          break;
        }
      }

      if (overlaps == true){
        continue;
      }
      //check if SSV overlaps with electron
      for (const xAOD::Electron* electron : electrons) {
        double DeltaR_el = compute_DeltaR_between_SSV_and_particle( SSV , electron );
        if (DeltaR_el < 0.2){
          overlaps = true;
          break;
        }
      }

      if (overlaps == true){
        continue;
      }

      //check if SSV overlaps with muon
      for (const xAOD::Muon* muon : muons) {
        double DeltaR_mu = compute_DeltaR_between_SSV_and_particle( SSV , muon );
        if (DeltaR_mu < 0.2){
          overlaps = true;
          break;
        }
      }

      if (overlaps == true){
        continue;
      }
      good_SSVs.push_back(SSV);
    }
    return good_SSVs;
  };

  // You construct a vector to see if the truthBh is an acceptance. An entry in this vector is true if the truth Bh is in acceptance and false if not
  std::vector<const xAOD::TruthParticle*> SSVWeightsAlg::create_accepted_truthBhs(
    const std::vector<const xAOD::TruthParticle*> &truthBhs,
    const std::vector<const xAOD::Jet*> &jets) const {

    std::vector<const xAOD::TruthParticle*> accepted_truthBhs;
  
    for (const xAOD::TruthParticle* truthBh : truthBhs) {
      // Check if truthBh fails truthBh in acceptance definition
      if (truthBh->pt() < 2000 || (std::abs(truthBh->eta()) > 2.8)){
        continue;
      }

      // check if truthBh overlaps with jet
      bool overlaps = false;
      for (const xAOD::Jet* jet : jets) {
        double  DeltaR = truthBh->p4().DeltaR(jet->p4());
        if (DeltaR<0.6){
          overlaps = true;
          break;
        }
      }
      
      if (overlaps == true){
        continue;
      }

      accepted_truthBhs.push_back(truthBh);
    }
    return accepted_truthBhs;
  };

  int SSVWeightsAlg::count_number_of_fake_SSVs(
    const std::vector<const xAOD::TruthParticle*> &truthBhs,
    const std::vector<const xAOD::Vertex*> &SSVs) const {
    
    int N_fake_SSV = 0;
    for (const xAOD::Vertex* SSV : SSVs){      
      bool foundMatch = false;
      for (const xAOD::TruthParticle* truthBh : truthBhs){
        double DeltaR = compute_DeltaR_between_SSV_and_particle(SSV, truthBh);
        if (DeltaR < 0.3){
          foundMatch = true;
          break;
        }
      }
      if (!foundMatch){
        // In this case no match was found between the current SSV and any truth particle 
        // Hence it is a fake SSV 
        // Increase the number of fake SSV counter 
        N_fake_SSV = N_fake_SSV + 1;
      }
    }
    return N_fake_SSV;
  }




  // create a vector that indicates if a truthBh got matched to a SSV
  std::vector<bool> SSVWeightsAlg::truthBh_to_SSV_matching(
    const std::vector<const xAOD::TruthParticle*> &truthBhs,
    const std::vector<const xAOD::Vertex*> &SSVs) const {
  
    std::vector<bool> matched_vector(truthBhs.size(), false);
    for (size_t i = 0; i < truthBhs.size(); ++i){
      const xAOD::TruthParticle* truthBh = truthBhs[i];
      for (size_t j = 0; j < SSVs.size(); ++j){
        const xAOD::Vertex* SSV = SSVs[j];  
        double DeltaR = compute_DeltaR_between_SSV_and_particle(SSV, truthBh);
        if (DeltaR < 0.3){
          matched_vector[i] = true;
          break;
        }
      }
    }
    return matched_vector;
  };

  // compute the DeltaR between a SSV and another particle (jet,electron,truthparticle etc.)
  double SSVWeightsAlg::compute_DeltaR_between_SSV_and_particle(
    const xAOD::Vertex* vtx,
    const xAOD::IParticle * part) const {

    static const SG::AuxElement::ConstAccessor<float> ssv_eta_accessor("bvrtEta");      
    static const SG::AuxElement::ConstAccessor<float> ssv_phi_accessor("bvrtPhi");      
    // Compute delta eta between vertex and particle 
    double eta_diff = ssv_eta_accessor(*vtx) - part->eta() ;

    // Compute delta phi between vertex and particle 
    // See TLorentzVector::DeltaR function 
    double phi_diff = TVector2::Phi_mpi_pi(ssv_phi_accessor(*vtx) - part->phi() );

    // Compute deltaR between vertex and particle
    return std::sqrt( eta_diff*eta_diff + phi_diff*phi_diff );
  }


  // count number of matched objects
  int SSVWeightsAlg::count_matched_objects(
    const std::vector<bool> &matching_vector) const {

    // Count the number of times true appears in the vector  
    return std::count(matching_vector.begin(), matching_vector.end(), true);
  }


  // count number of objects that are not matched
  int SSVWeightsAlg::count_not_matched_objects(
    const std::vector<bool> &matching_vector) const {

    return matching_vector.size() - count_matched_objects(matching_vector);
  }

  const std::vector<const xAOD::TruthParticle*> SSVWeightsAlg::construct_not_matched_vectors(
    const std::vector<const xAOD::TruthParticle*> &truthBhs,
    const std::vector<bool> &matched_vector) {

    std::vector<const xAOD::TruthParticle*> missed_vector;
    for (size_t i = 0; i < truthBhs.size(); ++i){
      if (matched_vector[i] == false){
        missed_vector.push_back(truthBhs[i]);
      }
    }
    return missed_vector;
  };

  // Indicate if hadron is heavy flavour hadron and in the final state in the truthparticle tree
  bool SSVWeightsAlg::isHFHadronFinalState(
    const xAOD::TruthParticle *part,
    const int type) const {

    for (unsigned int i = 0; i < part->nChildren(); ++i){
      const xAOD::TruthParticle *child = part->child(i);
      if (!child){
        continue;
      }
      if (type == 5){
        if (child->isBottomHadron()){
          return false;
        }
        if (child->isGenStable()){
          if (!isHFHadronFinalState(child, type)){
            return false;
          }
        }
      }
    
      if (type == 4){
        if (child->isCharmHadron()){
          return false;
        }
        if (child->isGenStable()){
          if (!isHFHadronFinalState(child, type)){
            return false;
          }
        }
      }
    }
    return true;
  }

  double SSVWeightsAlg::poisson_pmf(
    const int k,
    const double lambda){
    // Returns $P(k;\lambda) = \frac{e^{-\lambda}\lambda^k}{k!}$ 
    boost::math::poisson distrib(lambda);
    return boost::math::pdf(distrib, k);
  }

  SSVWeightsAlg::EfficiencyMethodBhadronPtEtaBasedClass::EfficiencyMethodBhadronPtEtaBasedClass( const nlohmann::json & jsonConfig )
    : m_ptbins(jsonConfig["efficiency_Bhadron_pT_eta_based"]["pt_bins"].get<std::vector<double>>())
  {
    // Extract information from JSON file for EfficiencyMethod EfficiencyMethodBhadronPtEtaBased
    for (size_t i = 0; i < m_ptbins.size() - 1; ++i) {
      std::string pT_bin_key = "pt_bin_" + std::to_string((int)m_ptbins[i]) + "_" + std::to_string((int)m_ptbins[i+1]);
      m_BhadronPtEtaEfficiencyMap[pT_bin_key] = jsonConfig["efficiency_Bhadron_pT_eta_based"][pT_bin_key];
    }
    m_upperboundpT = m_ptbins[m_ptbins.size()-1];
  }

  //calculate P_ineff based on the Bhadron pT and eta
  double SSVWeightsAlg::EfficiencyMethodBhadronPtEtaBasedClass::getPIneff(
    const std::vector<const xAOD::TruthParticle*> &accepted_truthBhs,
    const std::vector<bool> &truthBh_to_SSV_matched,
    double SF_eff) const{
    //construct missed truthBhs
    const std::vector<const xAOD::TruthParticle*> missed_truthBhs = construct_not_matched_vectors(accepted_truthBhs, truthBh_to_SSV_matched);

    //read off pt bins from JSON file
    const std::vector<double> &ptbins = m_ptbins;

    double P_ineff = 1;
    for (size_t i = 0; i < missed_truthBhs.size(); ++i) { 
      //retrieve pt,eta of missed truthBh
      double pt = missed_truthBhs[i]->pt();
      double eta = std::abs(missed_truthBhs[i]->eta());
      std::string pt_bin_of_truthBh = "";
      // iterate pt bins to find appropriate efficiency bin for the truthBh pT
      for (size_t j = 0; j < ptbins.size() - 1; ++j) {
        if (pt >= ptbins[j] && pt < ptbins[j+1]) {
          //construct pt bin name
          pt_bin_of_truthBh = "pt_bin_" + std::to_string((int)ptbins[j]) + "_" + std::to_string((int)ptbins[j+1]);
        }
        else if (pt > m_upperboundpT){
          pt_bin_of_truthBh = "pt_bin_" + std::to_string(m_upperboundpT) + "plus";
        }
      }
      if (pt_bin_of_truthBh == ""){
        //no pt bin found or no missed truthBh"
        continue;
      }
      //retrieve eta and efficiency bins for the pT bin
      const std::vector<double>& eta_bins = m_BhadronPtEtaEfficiencyMap.at(pt_bin_of_truthBh).at("eta");
      const std::vector<double>& efficiencies = m_BhadronPtEtaEfficiencyMap.at(pt_bin_of_truthBh).at("efficiency");

      double efficiency = 1;

      //iterate eta bins to find appropriate eta bin for truthBh eta
      for (size_t k = 0; k < eta_bins.size() - 1; ++k) {
        double eta_low = eta_bins[k];
        double eta_up = eta_bins[k+1];
        if (eta >= eta_low && eta < eta_up) {
          //eta bin found -> read off corresponding efficiency
          efficiency = efficiencies[k];
        }
      }
      //calculate P_ineff using the found efficiency
      P_ineff = P_ineff*(1-SF_eff*efficiency)/(1-efficiency);
    }
    return P_ineff;
  }

  SSVWeightsAlg::EfficiencyMethodBJetBasedClass::EfficiencyMethodBJetBasedClass( const nlohmann::json & jsonConfig )
    : m_bjetEfficiencyMap(jsonConfig["efficiency_bjet_based"])
  {
    // Extract information from JSON file for EfficiencyMethodBJetBased
    std::map<std::string, double>::iterator lastItem = std::prev(m_bjetEfficiencyMap.end());
    std::string lastItemKey = lastItem->first;
    m_upperboundNbjets = std::stoi(lastItemKey);
  }


  //calculate P_ineff based on the bjet multiplicity
  double SSVWeightsAlg::EfficiencyMethodBJetBasedClass::getPIneff(
    const int b_jet_count,
    const int N_missed,
    const double SF_eff) const{

    double P_ineff = 1;  

    // Get the value
    double epsilon = 1;

    //retrieve efficiency and average number of fake SSV depending on number of jets in event
    if (b_jet_count < m_upperboundNbjets){
      // Build the bjets key string
      std::string bjets_key = std::to_string(b_jet_count) + "_bjets";
      epsilon = m_bjetEfficiencyMap.at(bjets_key);
    }
    else{
      std::string bjets_key = std::to_string(m_upperboundNbjets) + "p_bjets";
      epsilon = m_bjetEfficiencyMap.at(bjets_key);      
    }

    P_ineff = std::pow((1-SF_eff*epsilon)/(1-epsilon), N_missed);

    return P_ineff;
  }

  SSVWeightsAlg::nFMethodPileupBJetBasedClass::nFMethodPileupBJetBasedClass( const nlohmann::json & jsonConfig )
    : m_nFPileupBJetMap(jsonConfig["nF_pileup_bjet_based"])
  {
    // Extract information from JSON file for nFMethodPileupBJetBased
    std::map<std::string, double>::iterator lastItem = std::prev(m_nFPileupBJetMap.at("high_muactual").end());
    std::string lastItemKey = lastItem->first;
    m_upperboundNbjets = std::stoi(lastItemKey);
    m_lowMuHighMuThreshold = jsonConfig["CalibrationInformation"]["lowMuHighMuThreshold"];
  }

  //calculate P_fake based on the bjet multiplicity in the high pileup and low pileup region
  double SSVWeightsAlg::nFMethodPileupBJetBasedClass::getPFake(
    const double muactual,
    const int b_jet_count,
    const int N_fake,
    const double SF_fake_low,
    const double SF_fake_high) const{

    double P_fake = 1;  
    // 2D map muactual and Nbjets
    std::string mu_key = (muactual >= m_lowMuHighMuThreshold) ? "high_muactual" : "low_muactual";

    // Get the value
    double n_F_value = 0;

    if (b_jet_count < m_upperboundNbjets){
      // Build the bjets key string
      std::string bjets_key = std::to_string(b_jet_count) + "_bjets";
      n_F_value = m_nFPileupBJetMap.at(mu_key).at(bjets_key);
    }
    else{
      // Build the bjets key string
      std::string bjets_key = std::to_string(m_upperboundNbjets) + "p_bjets";
      n_F_value = m_nFPileupBJetMap.at(mu_key).at(bjets_key);
    }

    if (muactual >= m_lowMuHighMuThreshold){
      P_fake = (poisson_pmf(N_fake, SF_fake_high*n_F_value))/poisson_pmf(N_fake, n_F_value);
    }
    else {
      P_fake = (poisson_pmf(N_fake, SF_fake_low*n_F_value))/poisson_pmf(N_fake, n_F_value);
    }

    return P_fake;
  }

  SSVWeightsAlg::nFMethodPileupBasedLinearFitClass::nFMethodPileupBasedLinearFitClass( const nlohmann::json & jsonConfig ){
    // Extract information from JSON file for nFMethodPileupBasedLinearFit
    m_slopeUnscaled = jsonConfig["nF_pileup_based_linearfit"]["unscaled"]["slope"];
    m_interceptUnscaled = jsonConfig["nF_pileup_based_linearfit"]["unscaled"]["intercept"];
    m_slopeScaled = jsonConfig["nF_pileup_based_linearfit"]["scaled"]["slope"];
    m_interceptScaled = jsonConfig["nF_pileup_based_linearfit"]["scaled"]["intercept"];
  }

  //calculate P_fake based on a linear fit of the average number of fake SSVs (nF) to the pileup (muactual)
  double SSVWeightsAlg::nFMethodPileupBasedLinearFitClass::getPFake(
    const double muactual,
    const int N_fake) const{
    // Calculate expected counts
    double n_F = m_slopeUnscaled * muactual + m_interceptUnscaled;
    double n_F_scaled = m_slopeScaled * muactual + m_interceptScaled;

    // Calculate P_fake
    double P_fake = poisson_pmf(N_fake, n_F_scaled) / poisson_pmf(N_fake, n_F);

    return P_fake;
  }

  SSVWeightsAlg::nFMethodPileupBasedBinnedClass::nFMethodPileupBasedBinnedClass( const nlohmann::json & jsonConfig )
    : m_muactualBins(jsonConfig["nF_pileup_based_binned"]["muactual_bins"].get<std::vector<double>>()),
      m_nFBins(jsonConfig["nF_pileup_based_binned"]["values"].get<std::vector<double>>())
  {
    // Extract information from JSON file for nFMethodPileupBasedBinned
    m_lowMuHighMuThreshold = jsonConfig["CalibrationInformation"]["lowMuHighMuThreshold"];
  }


  //calculate P_fake with the pileup binned
  double SSVWeightsAlg::nFMethodPileupBasedBinnedClass::getPFake(
    const double muactual,
    const int N_fake,
    const double SF_fake_low,
    const double SF_fake_high) const{

    double nF = 0;
    double P_fake = 1;

    // Find the correct bin for muactual
    for (size_t j = 0; j < m_muactualBins.size() - 1; ++j) {
      if (muactual >= m_muactualBins[j] && muactual < m_muactualBins[j + 1]) {
        nF = m_nFBins[j];
        if (muactual < m_lowMuHighMuThreshold){
          P_fake = poisson_pmf(N_fake, SF_fake_low * nF) / poisson_pmf(N_fake, nF);
        } else {
          P_fake = poisson_pmf(N_fake, SF_fake_high * nF) / poisson_pmf(N_fake, nF);
        }
        break; // Bin found, no need to continue loop
      }
    }
    return P_fake;
  }
}