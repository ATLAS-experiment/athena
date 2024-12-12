/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/JSSTaggerUtils.h"

#include "CxxUtils/fpcompare.h"
bool DescendingPtSorterConstituents(const xAOD::JetConstituent p1, const xAOD::JetConstituent p2)
{
  return CxxUtils::fpcompare::greater(p1.pt(), p2.pt());
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
      std::string ModelPath = ("/data/BoostedJetTaggers/SmoothedWZTaggers/" + ConstTaggerFileName);
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
      std::string ModelPath = ("/data/BoostedJetTaggers/SmoothedWZTaggers/" + HLTaggerFileName);
      ATH_MSG_INFO("JSSTaggerUtils::MLBosonTagger()" << "   + ModelPath " << ModelPath );

      asg::AsgToolConfig config ("AthONNX::JSSMLTool/MLBosonTagger_HL");
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

  m_decTaggedKey = m_containerName + "." + m_decorationName + "_" + m_decTaggedKey.key();
  m_decValidPtRangeHighKey = m_containerName + "." + m_decorationName + "_" + m_decValidPtRangeHighKey.key();
  m_decValidPtRangeLowKey = m_containerName + "." + m_decorationName + "_" + m_decValidPtRangeLowKey.key();
  m_decValidEtaRangeKey = m_containerName + "." + m_decorationName + "_" + m_decValidEtaRangeKey.key();
  m_decValidKinRangeKey = m_containerName + "." + m_decorationName + "_" + m_decValidKinRangeKey.key();
  m_decValidJetContentKey = m_containerName + "." + m_decorationName + "_" + m_decValidJetContentKey.key();
  m_decValidEventContentKey = m_containerName + "." + m_decorationName + "_" + m_decValidEventContentKey.key();

  m_decTau21WTAKey = m_containerName + "." + m_decTau21WTAKey.key();
  m_decTau32WTAKey = m_containerName + "." + m_decTau32WTAKey.key();
  m_decTau42WTAKey = m_containerName + "." + m_decTau42WTAKey.key();
  m_decC2Key = m_containerName + "." + m_decC2Key.key();
  m_decD2Key = m_containerName + "." + m_decD2Key.key();
  m_decE3Key = m_containerName + "." + m_decE3Key.key();
  m_decL2Key = m_containerName + "." + m_decL2Key.key();
  m_decL3Key = m_containerName + "." + m_decL3Key.key();

  m_readTau1WTAKey = m_containerName + "." + m_readTau1WTAKey.key();
  m_readTau2WTAKey = m_containerName + "." + m_readTau2WTAKey.key();
  m_readTau3WTAKey = m_containerName + "." + m_readTau3WTAKey.key();
  m_readTau4WTAKey = m_containerName + "." + m_readTau4WTAKey.key();
  m_readECF1Key = m_containerName + "." + m_readECF1Key.key();
  m_readECF2Key = m_containerName + "." + m_readECF2Key.key();
  m_readECF3Key = m_containerName + "." + m_readECF3Key.key();
  m_readD2Key = m_containerName + "." + m_readD2Key.key();
  m_readSplit12Key = m_containerName + "." + m_readSplit12Key.key();
  m_readSplit23Key = m_containerName + "." + m_readSplit23Key.key();
  m_readQwKey = m_containerName + "." + m_readQwKey.key();
  m_readThrustMajKey = m_containerName + "." + m_readThrustMajKey.key();
  m_readSphericityKey = m_containerName + "." + m_readSphericityKey.key();
  m_readECFG331Key = m_containerName + "." + m_readECFG331Key.key();
  m_readECFG311Key = m_containerName + "." + m_readECFG311Key.key();
  m_readECFG212Key = m_containerName + "." + m_readECFG212Key.key();

  m_readParentKey = m_containerName + "." + m_readParentKey.key();
  m_decNtrk500Key = m_containerName + "." + m_decNtrk500Key.key();
  m_readNtrk500Key = m_containerName + "." + m_readNtrk500Key.key();

  ATH_CHECK( m_decTau21WTAKey.initialize() );
  ATH_CHECK( m_decTau32WTAKey.initialize() );
  ATH_CHECK( m_decTau42WTAKey.initialize() );
  ATH_CHECK( m_decC2Key.initialize() );
  ATH_CHECK( m_decD2Key.initialize() );
  ATH_CHECK( m_decE3Key.initialize() );
  ATH_CHECK( m_decL2Key.initialize() );
  ATH_CHECK( m_decL3Key.initialize() );

  ATH_CHECK( m_readTau1WTAKey.initialize() );
  ATH_CHECK( m_readTau2WTAKey.initialize() );
  ATH_CHECK( m_readTau3WTAKey.initialize() );
  ATH_CHECK( m_readTau4WTAKey.initialize() );
  ATH_CHECK( m_readTau21WTAKey.initialize() );
  ATH_CHECK( m_readTau32WTAKey.initialize() );
  ATH_CHECK( m_readECF1Key.initialize() );
  ATH_CHECK( m_readECF2Key.initialize() );
  ATH_CHECK( m_readECF3Key.initialize() );
  ATH_CHECK( m_readD2Key.initialize() );
  ATH_CHECK( m_readSplit12Key.initialize() );
  ATH_CHECK( m_readSplit23Key.initialize() );
  ATH_CHECK( m_readQwKey.initialize() );
  ATH_CHECK( m_readThrustMajKey.initialize() );
  ATH_CHECK( m_readSphericityKey.initialize() );
  ATH_CHECK( m_readECFG331Key.initialize() );
  ATH_CHECK( m_readECFG311Key.initialize() );
  ATH_CHECK( m_readECFG212Key.initialize() );

  ATH_CHECK( m_readParentKey.initialize() );
  ATH_CHECK( m_decNtrk500Key.initialize() );
  ATH_CHECK( m_readNtrk500Key.initialize() );

  m_decNConstituentsKey = m_containerName + "." + m_decorationName + "_" + m_decNConstituentsKey.key();
  ATH_CHECK( m_decNConstituentsKey.initialize() );
  m_decNTopoTowersKey = m_containerName + "." + m_decorationName + "_" + m_decNTopoTowersKey.key();
  ATH_CHECK( m_decNTopoTowersKey.initialize() );

  m_decConstScoreKey = m_containerName + "." + m_decorationName + "_" + m_decConstScoreKey.key();
  ATH_CHECK( m_decConstScoreKey.initialize() );
  m_readConstScoreKey = m_containerName + "." + m_decorationName + "_" + m_readConstScoreKey.key();
  ATH_CHECK( m_readConstScoreKey.initialize() );

  m_decHLScoreKey = m_containerName + "." + m_decorationName + "_" + m_decHLScoreKey.key();
  ATH_CHECK( m_decHLScoreKey.initialize() );

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::tag( const xAOD::Jet& jet ) const {

  ATH_MSG_DEBUG( "Obtaining JSS Tagger Utils result   " << jet.pt() << "   " << jet.m() );

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::GetImageScore(const xAOD::Jet& jet) const {

  // init value
  float score (-99.);

  // preliminary actions for constituents
  // add a dedicated function for this?

  // get constituents
  std::vector<xAOD::JetConstituent> constituents = jet.getConstituents().asSTLVector();
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
  TH2D ImageCharged  = MakeJetImage("Charged" , &jet, csts_charged );
  TH2D ImageNeutral  = MakeJetImage("Neutral" , &jet, csts_neutral );
  TH2D ImageCombined = MakeJetImage("Combined", &jet, csts_combined);

  std::vector<TH2D> Images = {ImageCharged, ImageNeutral, ImageCombined};

  // evaluate the model
  score = m_MLBosonTagger -> retrieveConstituentsScore(Images);

  // save decorator
  SG::WriteDecorHandle<xAOD::Jet, float> decConstScore(m_decConstScoreKey);
  decConstScore(jet) = score;

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::GetConstScore(const xAOD::Jet& jet) const {

  // init value
  float score (-99.);

  // get constituents
  std::vector<xAOD::JetConstituent> constituents = jet.getConstituents().asSTLVector();
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
  if (towersAcc.isAvailable(jet)){
    // Vector of towers linked to jets
    std::vector<ElementLink<DataVector<xAOD::IParticle>>> towerLinks = towersAcc(jet);
    for (auto link_itr : towerLinks){
      if (!link_itr.isValid()) continue;
      towers.push_back(dynamic_cast<const xAOD::CaloCluster *>(*link_itr));
    }
  }
  std::sort( towers.begin(), towers.end(), DescendingPtSorterConstituents) ;

  // use ML tool on constituents
  std::vector<float> m, pT, eta, phi;
  for(auto cnst : constituents){
    m.push_back( cnst -> m() );
    pT.push_back( cnst -> pt() );
    eta.push_back( cnst -> eta() );
    phi.push_back( cnst -> phi() );
  }
  std::vector<std::vector<float>> constituents_packed = {m, pT, eta, phi};

  m.clear(); pT.clear(); eta.clear(); phi.clear();
  for(auto cnst : towers){
    m.push_back( cnst -> m() );
    pT.push_back( cnst -> pt() );
    eta.push_back( cnst -> eta() );
    phi.push_back( cnst -> phi() );
  }
  std::vector<std::vector<float>> towers_packed = {m, pT, eta, phi};

  // pack for the ML tool
  std::vector<std::vector<float>> inputs_packed = {
    constituents_packed.at(0), constituents_packed.at(1), constituents_packed.at(2), constituents_packed.at(3),
    towers_packed.at(0), towers_packed.at(1), towers_packed.at(2), towers_packed.at(3),
  };

  // evaluate the model
  //if( constituents.size()>1 && towers.size()>0 )
  if( constituents.size()>1 )
    score = m_MLBosonTagger -> retrieveConstituentsScore(inputs_packed);

  // save decorator
  SG::WriteDecorHandle<xAOD::Jet, float> decConstScore(m_decConstScoreKey);
  decConstScore(jet) = score;

  // and inputs as well
  SG::WriteDecorHandle<xAOD::Jet, float> decNConstituents(m_decNConstituentsKey);
  decNConstituents(jet) = constituents.size();

  SG::WriteDecorHandle<xAOD::Jet, float> decNTopoTowers(m_decNTopoTowersKey);
  decNTopoTowers(jet) = towers.size();

  return StatusCode::SUCCESS;

}

StatusCode JSSTaggerUtils::GetHLScore(const xAOD::Jet& jet) const {

  // init value
  float score (-99.);
  
  // get input variables
  std::map<std::string, double> JSSVars = GetJSSVars(jet);

  // evaluate the model
  score = m_MLBosonTagger_HL -> retrieveHighLevelScore(JSSVars);
  
  // save decorator
  SG::WriteDecorHandle<xAOD::Jet, float> decHLScore(m_decHLScoreKey);
  decHLScore(jet) = score;

  return StatusCode::SUCCESS;

}

std::map<std::string, double> JSSTaggerUtils::GetJSSVars(const xAOD::Jet& jet) const {

  std::map<std::string, double> JSSVars;

  // make available some missing JSS variables
  calculateJSSRatios(jet);

  int pv_location = findPV();
  if(pv_location != -1)
    GetUnGroomTracks(jet, pv_location);

  // store input variables
  JSSVars["pT"] = jet.pt();
  SG::ReadDecorHandle<xAOD::JetContainer, int> readNtrk500(m_readNtrk500Key);
  JSSVars["nTracks"] = readNtrk500(jet);

  SG::ReadDecorHandle<xAOD::Jet, float> readConstScore(m_readConstScoreKey);
  JSSVars["CNN"] = readConstScore(jet);

  // define the decorator readers
  SG::ReadDecorHandle<xAOD::JetContainer, float> readSplit12(m_readSplit12Key);
  SG::ReadDecorHandle<xAOD::JetContainer, float> readSplit23(m_readSplit23Key);

  SG::ReadDecorHandle<xAOD::JetContainer, float> readD2(m_readD2Key);

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

  // split
  JSSVars["Split12"] = readSplit12(jet);
  JSSVars["Split23"] = readSplit23(jet);

  // Energy Correlation Functions
  JSSVars["D2"] = readD2(jet);

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

        std::cout << " *** " << x << " " << y << " " << current_z << std::endl;
    }

    return *Image;

  }
