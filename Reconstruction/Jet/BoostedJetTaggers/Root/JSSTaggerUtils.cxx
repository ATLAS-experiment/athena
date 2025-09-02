/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/JSSTaggerUtils.h"

#include "CxxUtils/fpcompare.h"
bool DescendingPtSorterConstituents(const xAOD::JetConstituent p1, const xAOD::JetConstituent p2)
{
  return CxxUtils::fpcompare::greater(p1.pt(), p2.pt());
}

float Clip(float in, float low=1.e-36, float high=1.e+30){
  float out;

  if(in < low) out = low;
  else if(in > high) out = high;
  else out = in;

  return out;
}

int Sign(int in){

  int out;

  if(in<0) out = -1;
  else if(in>0) out = 1;
  else out = in;

  return out;

}

JSSTaggerUtils::JSSTaggerUtils( const std::string& name ) :
  JSSTaggerBase( name )
{
  declareProperty("MLBosonTagger", m_MLBosonTagger, "Tool to manage the data pre-processing and inference of the Const model");
  declareProperty("MLBosonTaggerHL", m_MLBosonTagger_HL, "Tool to manage the data pre-processing and inference of the High-Level model");
  declareProperty("nPixelsEta", m_nbins_eta);
  declareProperty("nPixelsPhi", m_nbins_phi);
  declareProperty("nColors", m_ncolors);
  declareProperty("MinEtaRange", m_min_eta);
  declareProperty("MaxEtaRange", m_max_eta);
  declareProperty("MinPhiRange", m_min_phi);
  declareProperty("MaxPhiRange", m_max_phi);
  declareProperty("DoRScaling", m_dorscaling);
  declareProperty("RScaling_p0", m_rscaling_p0);
  declareProperty("RScaling_p1", m_rscaling_p1);
}

StatusCode JSSTaggerUtils::initialize(){

  ATH_MSG_INFO( "Initializing JSSTaggerUtils tool" );

  ATH_MSG_INFO( " m_calibArea : " << m_calibArea );
  ATH_MSG_INFO( " m_configFile : " << m_configFile );

  if ( ! m_configFile.empty() ) {

    /// Get configReader
    ATH_CHECK( getConfigReader() );

    /// Get the decoration name
    m_decorationName = m_configReader.GetValue("DecorationName", "");

    m_UseConstTagger = !((std::string)m_configReader.GetValue("ConstTaggerFileName", "")).empty();
    m_UseHLTagger = !((std::string)m_configReader.GetValue("HLTaggerFileName", "")).empty();

    std::string ConstTaggerFileName = m_configReader.GetValue("ConstTaggerFileName", "aaa");
    std::string HLTaggerFileName = m_configReader.GetValue("HLTaggerFileName", "aaa");

    if(m_UseConstTagger && m_MLBosonTagger.empty()){
      // init tool
      std::string ModelPath = "";
      if ( m_calibArea.compare("Local") == 0 ) {
        ModelPath = PathResolverFindCalibFile(ConstTaggerFileName);
      }
      else if ( m_calibArea.find("eos") != std::string::npos) {
        ModelPath = (ConstTaggerFileName);    
      }
      else{
        ModelPath = PathResolverFindCalibFile(("BoostedJetTaggers/" + m_calibArea + "/" + ConstTaggerFileName).c_str());
      }
      ATH_MSG_INFO("JSSTaggerUtils::MLBosonTagger()" << "   + ModelPath " << ModelPath );

      asg::AsgToolConfig config ("AthONNX::JSSMLTool/MLBosonTagger");
      ATH_CHECK( config.setProperty("ModelPath", ModelPath));
    
      // get model paramters from the config file
      ATH_MSG_INFO("JSSTaggerUtils::MLBosonTagger() read value from config" );

      // set parameters
      ATH_CHECK( config.setProperty("nPixelsX", (int)m_configReader.GetValue("nPixelsEta", -99)) );
      ATH_CHECK( config.setProperty("nPixelsY", (int)m_configReader.GetValue("nPixelsPhi", -99)) );
      ATH_CHECK( config.setProperty("nPixelsZ", (int)m_configReader.GetValue("nColors", -99)) );

      m_nbins_eta = (int)m_configReader.GetValue("nPixelsEta", -99);
      m_nbins_phi = (int)m_configReader.GetValue("nPixelsPhi", -99);
      m_ncolors = m_configReader.GetValue("nColors", -99);
      m_min_eta = m_configReader.GetValue("aEta", -99.);
      m_max_eta = m_configReader.GetValue("bEta", -99.);
      m_min_phi = m_configReader.GetValue("aPhi", -99.);
      m_max_phi = m_configReader.GetValue("bPhi", -99.);
      m_dorscaling = (bool)m_configReader.GetValue("DoRScaling", -99);
      m_rscaling_p0 = m_configReader.GetValue("RScaling_p0", -99.);
      m_rscaling_p1 = m_configReader.GetValue("RScaling_p1", -99.);

      ATH_CHECK( config.makePrivateTool(m_MLBosonTagger) );
      ATH_CHECK( m_MLBosonTagger.retrieve() );

    }
    if(m_UseHLTagger){
      // init tool
      std::string ModelPath = "";
      if ( m_calibArea.compare("Local") == 0 ) {
        ModelPath = PathResolverFindCalibFile(HLTaggerFileName);
      }
      else if ( m_calibArea.find("eos") != std::string::npos) {
        ModelPath = (HLTaggerFileName);    
      }
      else{
        ModelPath = PathResolverFindCalibFile(("BoostedJetTaggers/" + m_calibArea + "/" + HLTaggerFileName).c_str());
      }
      ATH_MSG_INFO("JSSTaggerUtils::MLBosonTagger()" << "   + ModelPath " << ModelPath );

      asg::AsgToolConfig config ("AthONNX::JSSMLTool/MLBosonTaggerHL");
      ATH_CHECK( config.setProperty("ModelPath", ModelPath));
      
      ATH_CHECK( config.makePrivateTool(m_MLBosonTagger_HL) );
      ATH_CHECK( m_MLBosonTagger_HL.retrieve() );
    
      // get model paramters from the config file
      ATH_MSG_INFO("JSSTaggerUtils::MLBosonTagger() read value from config" );
      ATH_CHECK( ReadScaler() );

    }

  }

  /// Initialize decorators
  ATH_MSG_INFO( "Decorators that will be attached to jet :" );
  ATH_CHECK(JSSTaggerBase::initialize());

  m_decNConstituentsKey = m_containerName + "." + m_decorationName + "_" + m_decNConstituentsKey.key();
  ATH_CHECK( m_decNConstituentsKey.initialize() );
  m_decNTopoTowersKey = m_containerName + "." + m_decorationName + "_" + m_decNTopoTowersKey.key();
  ATH_CHECK( m_decNTopoTowersKey.initialize() );

  m_decConstScoreKey = m_containerName + "." + m_decorationName + "_" + m_decConstScoreKey.key();
  ATH_CHECK( m_decConstScoreKey.initialize() );

  m_constScoreAcc = std::make_unique<SG::ConstAccessor<float>>(m_decorationName + "_ConstScore");

  m_decHLScoreKey = m_containerName + "." + m_decorationName + "_" + m_decHLScoreKey.key();
  ATH_CHECK( m_decHLScoreKey.initialize() );

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::tag( const xAOD::Jet& jet ) const {

  ATH_MSG_DEBUG( "Obtaining JSS Tagger Utils result   " << jet.pt() << "   " << jet.m() );

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::GetImageScore(const xAOD::JetContainer& jets) const {

  SG::WriteDecorHandle<xAOD::JetContainer, float> decConstScore(m_decConstScoreKey);

  for(const xAOD::Jet* jet : jets){

    // init value
    float score (-99.);

    // preliminary actions for constituents
    // add a dedicated function for this?

    // get constituents
    std::vector<xAOD::JetConstituent> constituents = jet -> getConstituents().asSTLVector();
    std::sort( constituents.begin(), constituents.end(), DescendingPtSorterConstituents) ;

    int MaxConstituents (100);
    std::vector<xAOD::JetConstituent> constituentsForModel;

    if( constituents.size() > 100 )
      constituentsForModel = std::vector<xAOD::JetConstituent> (constituents.begin(), constituents.begin() + MaxConstituents);
    else 
      constituentsForModel = constituents;

    // constituents - charged
    std::vector<xAOD::JetConstituent> csts_charged = constituentsForModel;
    csts_charged.erase( std::remove_if( csts_charged.begin(), csts_charged.end(),
                        [] (xAOD::JetConstituent constituent) -> bool {
                          const xAOD::FlowElement* ufo = dynamic_cast<const xAOD::FlowElement*>(constituent.rawConstituent());
                          return ufo -> signalType() != xAOD::FlowElement::SignalType::Charged;
                        }), csts_charged.end()) ;

    // constituents - neutral
    std::vector<xAOD::JetConstituent> csts_neutral = constituentsForModel;
    csts_neutral.erase( std::remove_if( csts_neutral.begin(), csts_neutral.end(),
                        [] (xAOD::JetConstituent constituent) -> bool {
                          const xAOD::FlowElement* ufo = dynamic_cast<const xAOD::FlowElement*>(constituent.rawConstituent());
                          return ufo -> signalType() != xAOD::FlowElement::SignalType::Neutral;
                        }), csts_neutral.end()) ;

    // constituents - combined
    std::vector<xAOD::JetConstituent> csts_combined = constituentsForModel;
    csts_combined.erase( std::remove_if( csts_combined.begin(), csts_combined.end(),
                        [] (xAOD::JetConstituent constituent){
                          const xAOD::FlowElement* ufo = dynamic_cast<const xAOD::FlowElement*>(constituent.rawConstituent());
                          return ufo -> signalType() != xAOD::FlowElement::SignalType::Combined;
                        }), csts_combined.end()) ;

    // use ML tool on constituents
    TH2D ImageCharged  = MakeJetImage("Charged" , jet, csts_charged );
    TH2D ImageNeutral  = MakeJetImage("Neutral" , jet, csts_neutral );
    TH2D ImageCombined = MakeJetImage("Combined", jet, csts_combined);

    std::vector<TH2D> Images = {ImageCharged, ImageNeutral, ImageCombined};

    // evaluate the model
    score = m_MLBosonTagger -> retrieveConstituentsScore(Images);

    // save decorator
    decConstScore(*jet) = score;
  }

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::GetConstScore(const xAOD::JetContainer& jets) const {

  SG::WriteDecorHandle<xAOD::JetContainer, float> decConstScore(m_decConstScoreKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> decNConstituents(m_decNConstituentsKey);
  SG::WriteDecorHandle<xAOD::JetContainer, float> decNTopoTowers(m_decNTopoTowersKey);

  for(const xAOD::Jet *jet : jets){

    // init value
    float score (-99.);

    // get constituents
    std::vector<xAOD::JetConstituent> constituents = jet -> getConstituents().asSTLVector();
    std::sort( constituents.begin(), constituents.end(), DescendingPtSorterConstituents) ;

    int MaxConstituents (100);
    std::vector<xAOD::JetConstituent> constituentsForModel;

    if( constituents.size() > 100 )
      constituentsForModel = std::vector<xAOD::JetConstituent> (constituents.begin(), constituents.begin() + MaxConstituents);
    else 
      constituentsForModel = constituents;

    // get towers
    std::vector<const xAOD::CaloCluster*> towers;
    SG::AuxElement::ConstAccessor<std::vector<ElementLink<DataVector<xAOD::IParticle>>>> towersAcc("GhostTower");
    if (towersAcc.isAvailable(*jet)){
      // Vector of towers linked to jets
      std::vector<ElementLink<DataVector<xAOD::IParticle>>> towerLinks = towersAcc(*jet);
      for (auto link_itr : towerLinks){
        if (!link_itr.isValid()) continue;
        towers.push_back(dynamic_cast<const xAOD::CaloCluster *>(*link_itr));
      }
    }
    std::sort( towers.begin(), towers.end(), DescendingPtSorterConstituents);

    // use ML tool on constituents
    std::vector<float> m, pT, eta, phi, E;
    for(auto cnst : constituents){
      m.push_back( cnst -> m() );
      pT.push_back( cnst -> pt() );
      eta.push_back( cnst -> eta() );
      phi.push_back( cnst -> phi() );
      E.push_back( cnst -> e() );
    }
    std::vector<std::vector<float>> constituents_packed = {m, pT, eta, phi};

    m.clear(); pT.clear(); eta.clear(); phi.clear();
    for(auto cnst : towers){
      m.push_back( cnst -> m() );
      pT.push_back( cnst -> pt() );
      eta.push_back( cnst -> eta() );
      phi.push_back( cnst -> phi() );
      E.push_back( cnst -> e() );
    }
    std::vector<std::vector<float>> towers_packed = {m, pT, eta, phi};

    // pack for the ML tool
    std::vector<std::vector<float>> inputs_packed = {
      constituents_packed.at(0), constituents_packed.at(1), constituents_packed.at(2), constituents_packed.at(3),
      towers_packed.at(0), towers_packed.at(1), towers_packed.at(2), towers_packed.at(3),
    };

    // evaluate the model
    if( (constituents.size() + towers.size()) > 1 )
      score = m_MLBosonTagger -> retrieveConstituentsScore(inputs_packed);

    // save decorator
    decConstScore(*jet) = score;

    // and inputs as well
    decNConstituents(*jet) = constituents.size();
    decNTopoTowers(*jet) = towers.size();

  }

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::GetQGConstScore(const xAOD::JetContainer& jets) const {

  SG::WriteDecorHandle<xAOD::JetContainer, float> decConstScore(m_decConstScoreKey);

  for(const xAOD::Jet *jet : jets){

    // init value
    float score (-99.);

    // get constituents
    std::vector<xAOD::JetConstituent> constituents = jet -> getConstituents().asSTLVector();
    std::sort( constituents.begin(), constituents.end(), DescendingPtSorterConstituents) ;

    // get towers
    std::vector<const xAOD::CaloCluster*> towers;
    SG::AuxElement::ConstAccessor<std::vector<ElementLink<DataVector<xAOD::IParticle>>>> towersAcc("GhostTower");
    if (towersAcc.isAvailable(*jet)){
      // Vector of towers linked to jets
      std::vector<ElementLink<DataVector<xAOD::IParticle>>> towerLinks = towersAcc(*jet);
      for (auto link_itr : towerLinks){
        if (!link_itr.isValid()) continue;
        towers.push_back(dynamic_cast<const xAOD::CaloCluster *>(*link_itr));
      }
    }
    std::sort( towers.begin(), towers.end(), DescendingPtSorterConstituents) ;

    // use ML tool on constituents
    std::vector<float> m, pT, eta, phi, E, isTower, px, py, pz;
    for(auto cnst : constituents){
      m.push_back( cnst -> m() );
      pT.push_back( cnst -> pt() );
      eta.push_back( cnst -> eta() );
      phi.push_back( cnst -> phi() );
      E.push_back( cnst -> e() );
      isTower.push_back(0.);
      px.push_back( cnst -> pt() * std::cos(cnst -> phi()) );
      py.push_back( cnst -> pt() * std::sin(cnst -> phi()) );
      pz.push_back( cnst -> pt() * std::sinh(cnst -> eta()) );
    }

    for(auto cnst : towers){
      m.push_back( cnst -> m() );
      pT.push_back( cnst -> pt() );
      eta.push_back( cnst -> eta() );
      phi.push_back( cnst -> phi() );
      E.push_back( cnst -> e() );
      isTower.push_back(1.);
      px.push_back( cnst -> pt() * std::cos(cnst -> phi()) );
      py.push_back( cnst -> pt() * std::sin(cnst -> phi()) );
      pz.push_back( cnst -> pt() * std::sinh(cnst -> eta()) );
    }

    // put together constituents + towers
    std::vector<std::vector<float>> features_packed;

    for(long unsigned int f=0; f<pT.size(); f++){
      std::vector<float> features = { m.at(f), pT.at(f), eta.at(f), phi.at(f), E.at(f), isTower.at(f), px.at(f), py.at(f), pz.at(f) };
      features_packed.push_back(features);
    }

    // sort them
    std::sort(features_packed.begin(), features_packed.end(), [](const auto& i, const auto& j) { return i.at(1) > j.at(1); });

    // global aux variables
    float sum_features_px = std::accumulate(px.begin(), px.end(), 0);
    float sum_features_py = std::accumulate(py.begin(), py.end(), 0);
    float sum_features_pz = std::accumulate(pz.begin(), pz.end(), 0);

    float sum_features_pT = sqrt( sum_features_px*sum_features_px + sum_features_py*sum_features_py);
    float sum_features_eta = std::asinh( sum_features_pz / Clip(sum_features_pT) );
    float sum_features_phi = std::atan2( sum_features_py, sum_features_px );

    // build constituents and interaction variables
    std::vector<std::vector<float>> const_vars;
    std::vector<std::vector<std::vector<float>>> inter_vars;

    for(auto const &feature_i : features_packed){

      // calculate variables: constituents
      float log_pT = log( Clip(feature_i.at(1) / sum_features_pT) );
      float log_E = log( Clip(feature_i.at(4) / sum_features_pT) );
      float eta = feature_i.at(2) - sum_features_eta;
      float phi = feature_i.at(3) - sum_features_phi;
      float DR = sqrt((feature_i.at(2) - sum_features_eta)*(feature_i.at(2) - sum_features_eta) + (feature_i.at(3) - sum_features_phi)*(feature_i.at(3) - sum_features_phi));
      float log_m = log(Clip(feature_i.at(0)));
      float type = feature_i.at(5);

      std::vector<float> vars = {log_pT, log_E, eta, phi, DR, log_m, type};
      const_vars.push_back(vars);

      // calculate variables: interactions
      std::vector<std::vector<float>> inter_vars_int;
      for(auto const &feature_j : features_packed){

        // preparing variables
        float delta = sqrt((feature_i.at(2)-feature_j.at(2))*(feature_i.at(2)-feature_j.at(2)) + (feature_i.at(3)-feature_j.at(3))*(feature_i.at(3)-feature_j.at(3)));
        float min = feature_i.at(1) != feature_j.at(1) ? std::min(feature_i.at(1), feature_j.at(1)): 0.;
        float mass2 = (feature_i.at(4)/sum_features_pT + feature_j.at(4)/sum_features_pT) * (feature_i.at(4)/sum_features_pT + feature_j.at(4)/sum_features_pT);
        mass2 -= (feature_i.at(6)/sum_features_pT + feature_j.at(6)/sum_features_pT) * (feature_i.at(6)/sum_features_pT + feature_j.at(6)/sum_features_pT);
        mass2 -= (feature_i.at(7)/sum_features_pT + feature_j.at(7)/sum_features_pT) * (feature_i.at(7)/sum_features_pT + feature_j.at(7)/sum_features_pT);
        mass2 -= (feature_i.at(8)/sum_features_pT + feature_j.at(8)/sum_features_pT) * (feature_i.at(8)/sum_features_pT + feature_j.at(8)/sum_features_pT);

        // final values
        float log_delta = log(Clip(delta));
        float log_mindelta = log(Clip(min * delta  / sum_features_pT));
        float min_over_pT = min / (feature_i.at(1) + feature_j.at(1));
        float log_mass = log(Clip(mass2));

        // set the diagonal to 0
        if(feature_i==feature_j){
          log_delta = 0;
          log_mindelta = 0;
          min_over_pT = 0;
          log_mass = 0;
        }

        std::vector<float> vars = { log_delta,
                                    log_mindelta,
                                    min_over_pT,
                                    log_mass
                                  };
        inter_vars_int.push_back(vars);

      }

      inter_vars.push_back(inter_vars_int);
    }

    // evaluate the model
    if( (constituents.size() + towers.size()) > 1 ) 
      score = m_MLBosonTagger -> retrieveConstituentsScore(const_vars, inter_vars);

    // save decorator
    decConstScore(*jet) = score;

  }

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::GetHLScore(const xAOD::JetContainer& jets) const {

  SG::WriteDecorHandle<xAOD::JetContainer, float> decHLScore(m_decHLScoreKey);

  // make available JSS variables
  decorateJSSRatios(jets);

  for(const xAOD::Jet* jet : jets){

    // init value
    float score (-99.);
    
    // get input variables
    std::map<std::string, double> JSSVars = GetJSSVars(*jet);

    // evaluate the model
    score = m_MLBosonTagger_HL -> retrieveHighLevelScore(JSSVars);
    
    // save decorator
    decHLScore(*jet) = score;

  }

  return StatusCode::SUCCESS;

}

std::map<std::string, double> JSSTaggerUtils::GetJSSVars(const xAOD::Jet& jet) const {

  std::map<std::string, double> JSSVars;

  // retrieve ungroomed tracks multiplicity
  int nUngrTracks (-1);
  int pv_location = findPV();
  if(pv_location != -1)
    nUngrTracks = GetUnGroomTracks(jet, pv_location);

  // store input variables
  JSSVars["pT"] = jet.pt();
  JSSVars["nTracks"] = nUngrTracks;
  JSSVars["CNN"] = (*m_constScoreAcc)(jet);

  // define the decorator readers
  SG::ReadDecorHandle<xAOD::JetContainer, float> readSplit12(m_readSplit12Key);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readSplit23(m_readSplit23Key);

  SG::ReadDecorHandle<xAOD::JetContainer, float> readTau1_wta(m_readTau1WTAKey);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readTau2_wta(m_readTau2WTAKey);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readTau3_wta(m_readTau3WTAKey);

  SG::ReadDecorHandle<xAOD::JetContainer, float> readECF1(m_readECF1Key);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readECF2(m_readECF2Key);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readECF3(m_readECF3Key);

  SG::ReadDecorHandle<xAOD::JetContainer, float> readQw(m_readQwKey);

  // define the ConstAccessor
  static const SG::ConstAccessor<float> FoxWolfram0Acc("FoxWolfram0");
  static const SG::ConstAccessor<float> FoxWolfram2Acc("FoxWolfram2");
  static const SG::ConstAccessor<float> PlanarFlowAcc("PlanarFlow");
  static const SG::ConstAccessor<float> AngularityAcc("Angularity");
  static const SG::ConstAccessor<float> AplanarityAcc("Aplanarity");
  static const SG::ConstAccessor<float> ZCut12Acc("ZCut12");
  static const SG::ConstAccessor<float> KtDRAcc("KtDR");
  static const SG::ConstAccessor<float> D2Acc("D2");

  // split
  JSSVars["Split12"] = readSplit12(jet);
  JSSVars["Split23"] = readSplit23(jet);

  // Energy Correlation Functions
  JSSVars["D2"] = D2Acc(jet);

  // Tau123 WTA
  JSSVars["Tau1_wta"] = readTau1_wta(jet);
  JSSVars["Tau2_wta"] = readTau2_wta(jet);
  JSSVars["Tau3_wta"] = readTau3_wta(jet);

  // ECF
  JSSVars["ECF1"] = readECF1(jet);
  JSSVars["ECF2"] = readECF2(jet);
  JSSVars["ECF3"] = readECF3(jet);

  // Qw
  JSSVars["Qw"] = readQw(jet);

  // Other moments
  JSSVars["FoxWolfram0"] = FoxWolfram0Acc.withDefault(jet, -99.);
  JSSVars["FoxWolfram2"] = FoxWolfram2Acc.withDefault(jet, -99.);
  JSSVars["PlanarFlow"] = PlanarFlowAcc.withDefault(jet, -99.);
  JSSVars["Angularity"] = AngularityAcc.withDefault(jet, -99.);
  JSSVars["Aplanarity"] = AplanarityAcc.withDefault(jet, -99.);
  JSSVars["ZCut12"] = ZCut12Acc.withDefault(jet, -99.);
  JSSVars["KtDR"] = KtDRAcc.withDefault(jet, -99.);

  return JSSVars;

}

StatusCode JSSTaggerUtils::ReadScaler(){

  // input list
  std::vector<std::string> vars_list = {
    "pT","CNN","D2","nTracks","ZCut12",
    "Tau1_wta","Tau2_wta","Tau3_wta",
    "KtDR","Split12","Split23",
    "ECF1","ECF2","ECF3",
    "Angularity","FoxWolfram0","FoxWolfram2",
    "Aplanarity","PlanarFlow","Qw",
  };

  // loop and read
  for(const std::string & var : vars_list){
    std::string s_mean = var + "_mean";
    std::string s_std  = var + "_std";
    double mean = m_configReader.GetValue(s_mean.c_str(), -99.);
    double std  = m_configReader.GetValue(s_std.c_str() , -99.);

    if(mean==-99. && std==-99.){
      ATH_MSG_ERROR("ERROR: one of the parameter for " << var << " is missing, please, double check the config!!!");
      return StatusCode::FAILURE;
    }
    else if(mean==-99. || std==-99.){
      ATH_MSG_ERROR("ERROR: parameters for " << var << " are missing, please, double check the config!!!");
      return StatusCode::FAILURE;
    }
    else{
      m_scaler[var] = {mean, std};
    }

    // pass the features scaling paramters to the ML tool
    // we need to apply the data pre-processing 
    // before to apply the model inference via ONNX

    // ToDo: change this to a property
    ATH_CHECK( m_MLBosonTagger_HL -> SetScaler(m_scaler) );

  }

  return StatusCode::SUCCESS;

}

  TH2D JSSTaggerUtils::MakeJetImage(TString TagImage, const xAOD::Jet* jet, std::vector<xAOD::JetConstituent> constituents) const
  {

    double eta (-99.), phi (-99.), pT (-99.), z (-99.);
    int BinEta (-99), BinPhi (-99);

    auto cst_pT = [] ( double sum, xAOD::JetConstituent cst ){
      return sum + cst.pt(); };
    double SumPT = std::accumulate( constituents.begin(), constituents.end(), 0., cst_pT) ;

    auto Image = std::make_unique<TH2D>("Image_" + TagImage, "Image_" + TagImage, 
			   m_nbins_eta, m_min_eta, m_max_eta, m_nbins_phi, m_min_phi, m_max_phi);

    for( auto& cst : constituents ){
      eta = cst -> eta() - jet -> eta() ;
      phi = cst -> phi() - jet -> phi() ;
      pT  = cst -> pt() ;

      // apply r-scaling
      if( m_dorscaling ){
        eta *= 1. / (m_rscaling_p0 + m_rscaling_p1/jet->pt());
        phi *= 1. / (m_rscaling_p0 + m_rscaling_p1/jet->pt());
      }

      BinEta = Image -> GetXaxis() -> FindBin(eta);
      BinPhi = Image -> GetYaxis() -> FindBin(phi);

      z = pT / SumPT ;

      int x = m_nbins_phi+1-BinPhi, y = BinEta; // transpose + flip

      double current_z = Image -> GetBinContent( x, y );

      if( eta>m_min_eta && eta<m_max_eta && phi>m_min_phi && phi<m_max_phi ) // avoid overflow pixels
	      Image -> SetBinContent( x, y, current_z += z );
    }

    return *Image;

  }

StatusCode JSSTaggerUtils::GetTopConstScore(const xAOD::JetContainer& jets) const {

  SG::WriteDecorHandle<xAOD::JetContainer, float> decConstScore(m_decConstScoreKey);

  for(const xAOD::Jet *jet : jets){

    // init value
    float score (-99.);
    int parity (1);

    // get constituents
    std::vector<xAOD::JetConstituent> constituents = jet -> getConstituents().asSTLVector();
    std::sort( constituents.begin(), constituents.end(), DescendingPtSorterConstituents) ;

    // skim constituents
    // nMaxConstituents: used to compute the sum_pT and sum_E for the pre-processing
    // nInputConstituents: actual input to the network
    long unsigned int nMaxConstituents (100);
    long unsigned int nInputConstituents (80);

    std::vector<float> pT_nMax, E_nMax;
    for(auto cnst : constituents){
      pT_nMax.push_back( cnst -> pt() );
      E_nMax.push_back( cnst -> e() );
      if(pT_nMax.size()==nMaxConstituents) break;
    }

    // remove non physical constituents
    constituents.erase( std::remove_if( constituents.begin(), constituents.end(),
                        [] (xAOD::JetConstituent constituent) -> bool {return log(constituent -> pt()) < 0.1;}), 
                        constituents.end()) ;

    // use ML tool on constituents
    std::vector<float> pT, eta, phi, E, isValid, px, py, pz;
    for(auto cnst : constituents){
      pT.push_back( cnst -> pt() );
      eta.push_back( cnst -> eta() );
      phi.push_back( cnst -> phi() );
      E.push_back( cnst -> e() );
      isValid.push_back(1.);
      px.push_back( cnst -> pt() * std::cos(cnst -> phi()) );
      py.push_back( cnst -> pt() * std::sin(cnst -> phi()) );
      pz.push_back( cnst -> pt() * std::sinh(cnst -> eta()) );
    }
    

    // global aux variables
    float sum_features_E = std::accumulate(E_nMax.begin(), E_nMax.end(), 0.);
    float sum_features_pT_scalar = std::accumulate(pT_nMax.begin(), pT_nMax.end(), 0.);

    TLorentzVector constituent0;
    if(constituents.size() > 0)
      constituent0.SetPtEtaPhiE(pT.at(0), eta.at(0), phi.at(0), E.at(0));

    float angle (0.);
    if(constituents.size() > 1){
      float deta01 = eta.at(1) - eta.at(0);
      float dphi01 = phi.at(1) - phi.at(0);
      if(dphi01 > std::numbers::pi) dphi01 -= 2*std::numbers::pi;
      else if(dphi01 < -std::numbers::pi) dphi01 += 2*std::numbers::pi;

      angle = std::atan2(dphi01, deta01) + std::numbers::pi/2;
    }

    // build constituents, mask and base momentum for interaction variables
    std::vector<std::vector<float>> const_vars;
    std::vector<std::vector<float>> masks_vars;
    std::vector<std::vector<std::vector<float>>> inter_vars;

    for(long unsigned int i=0; i<constituents.size(); i++){

      // up to nInputConstituents constituents
      if(const_vars.size()==nInputConstituents) break;

      // put the constituent in a tlv for help
      TLorentzVector constituent_i;
      constituent_i.SetPtEtaPhiE(pT.at(i), eta.at(i), phi.at(i), E.at(i));

      // flip/rot of constituents: center
      float eta_center = eta.at(i) - eta.at(0);
      float phi_center = constituent_i.DeltaPhi(constituent0);

      // flip/rot of constituents: rotate
      // ToDo: this does not make sense when nConst == 1, 
      // but it also true that the score is not retrieved for those jets,
      // should we add a further protection?
      float eta_rot =  eta_center * cos(angle) + phi_center * sin(angle);
      float phi_rot = -eta_center * sin(angle) + phi_center * cos(angle);

      // flip/rot of constituents: parity
      if(i==2 && eta_rot<0.) parity = -1;
      float eta_flip = eta_rot * parity;
      
      // calculate constituents variables
      float log_pT = log( pT.at(i));
      float log_E = log( E.at(i));
      float log_pT_rel = log( pT.at(i) / sum_features_pT_scalar);
      float log_E_rel = log( E.at(i) / sum_features_E);
      float Deta = eta_flip;
      float Dphi = phi_rot;
      float DR = sqrt(Deta*Deta + Dphi*Dphi);

      // pack: constituents variables
      std::vector<float> vars = {log_pT, log_E, log_pT_rel, log_E_rel, DR, Deta, Dphi};
      const_vars.push_back(vars);

      // pack: mask variable
      vars = {1.};
      masks_vars.push_back(vars);
      
      // explict interaction variables
      // calculate variables: interactions
      std::vector<std::vector<float>> inter_vars_int;
      for(long unsigned int j=0; j<constituents.size(); j++){

        // tlv for constituents
        // todo: harmonise with the previous one
        TLorentzVector constituent_j;
        constituent_j.SetPtEtaPhiE(pT.at(j), eta.at(j), phi.at(j), E.at(j));
  
        // preparing variables

        // custom rapidity calculation
        double rap_i = 0.5 * log( 1 + (2 * pz.at(i)) / Clip(E.at(i) - pz.at(i), 1.e-8) );
        double rap_j = 0.5 * log( 1 + (2 * pz.at(j)) / Clip(E.at(j) - pz.at(j), 1.e-8) );
        double deltaY = rap_i - rap_j;
        double deltaPhi = TVector2::Phi_mpi_pi(constituent_i.Phi() - constituent_j.Phi());
        double delta = std::sqrt(deltaY * deltaY + deltaPhi * deltaPhi);

        float min = pT.at(i) != pT.at(j) ? std::min(pT.at(i), pT.at(j)): pT.at(i);
        float mass2 = (E.at(i) + E.at(j)) * (E.at(i) + E.at(j));
        mass2 -= (px.at(i) + px.at(j)) * (px.at(i) + px.at(j));
        mass2 -= (py.at(i) + py.at(j)) * (py.at(i) + py.at(j));
        mass2 -= (pz.at(i) + pz.at(j)) * (pz.at(i) + pz.at(j));

        // final values
        float log_delta = log(Clip(delta, 1.e-8));
        float log_mindelta = log(Clip(min * delta, 1.e-8));
        float log_min_over_pT = log(Clip(min / (pT.at(i) + pT.at(j)), 1.e-8));
        float log_mass = log(Clip(mass2, 1.e-8));

        std::vector<float> vars = { log_mindelta,
                                    log_min_over_pT,
                                    log_delta,
                                    log_mass
                                  };
        inter_vars_int.push_back(vars);

      }
      inter_vars.push_back(inter_vars_int);

    }

    for(long unsigned int i=constituents.size(); i<nInputConstituents; i++){
      // pack: constituents variables
      std::vector<float> vars = {-18.4207, -18.4207, -18.4207, -18.4207, -18.4207, -18.4207, -18.4207};
      const_vars.push_back(vars);

      // pack: interaction variables
      vars = {-18.4207, -18.4207, -18.4207, -18.4207};
      std::vector<std::vector<float>> vars_inter;
      for(long unsigned int j=0; j<nInputConstituents; j++){
        vars_inter.push_back(vars);
      }
      inter_vars.push_back(vars_inter);

      // pack: mask variable
      vars = {0.};
      masks_vars.push_back(vars);
    }

    // further adjustment for interaction variables
    for(long unsigned int i=0; i<constituents.size(); i++){
      std::vector<float> vars = {0., 0., 0., 0.};
      vars = {-18.4207, -18.4207, -18.4207, -18.4207};
      for(long unsigned int j=constituents.size(); j<nInputConstituents; j++){
        inter_vars.at(i).push_back(vars);
      }
    }

    // evaluate the model
    if( constituents.size() > 1 ) 
      score = m_MLBosonTagger -> retrieveConstituentsScore(const_vars, inter_vars, masks_vars);

    // save decorator
    // the model return the qcd node score
    // therefore, we use 1 - score to have 
    // top jet: 1, q/g jet: 0
    decConstScore(*jet) = 1 - score;

  }

  return StatusCode::SUCCESS;

}
