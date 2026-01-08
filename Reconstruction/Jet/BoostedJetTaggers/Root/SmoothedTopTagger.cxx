/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BoostedJetTaggers/SmoothedTopTagger.h"

SmoothedTopTagger::SmoothedTopTagger( const std::string& name ) :
  JSSTaggerBase( name )
{
  // minimum and maximum pT of jets to tag
  declareProperty( "JetPtMin",              m_jetPtMin = 350.0e3);
  declareProperty( "JetPtMax",              m_jetPtMax = 3000.0e3);

  // cut functions that describe the tagger cuts that will be made
  declareProperty( "VarCutFuncs", m_varCutExprs={}, "") ;
  // names of the variables that are used for cuts
  declareProperty( "VarCutNames", m_varCutNames={}, "") ;


}

StatusCode SmoothedTopTagger::initialize() {
  
  ATH_MSG_INFO( "Initializing SmoothedTopTagger tool" );

  if( ! m_configFile.empty() ) {

    /// Get configReader
    ATH_CHECK( getConfigReader() );

    std::string prefix = "";
    if ( ! m_wkpt.empty() ) prefix = m_wkpt+".";

    // read the number of variables for tagger from file
    m_numTaggerVars = std::stoi(m_configReader.GetValue( (prefix+"NumVars").c_str(), ""));

    ATH_MSG_DEBUG("Number of variables used by tagger is " << std::to_string(m_numTaggerVars));

    std::string varName, varCutExpr;
    for (int i = 1; i <= m_numTaggerVars; i++) {
      // read the cut name corresponding to this variable
      varName = m_configReader.GetValue( (prefix+"Var"+std::to_string(i)).c_str(), "");

      if (varName.empty()) {
        ATH_MSG_ERROR("Config file does not specify Var" << std::to_string(i) << "!") ; 
        return StatusCode::FAILURE;
      }

      if (std::find(m_recognisedCuts.begin(), m_recognisedCuts.end(), varName) == m_recognisedCuts.end()) {
        ATH_MSG_ERROR("Unrecognised variable " << varName << " in config file!") ;
        return StatusCode::FAILURE;
      }

      m_varCutNames.push_back(varName);

      // read cut expression
      varCutExpr = m_configReader.GetValue( (prefix+m_varCutNames.back()+"Cut").c_str(), "");

      if (varCutExpr.empty()) {
        ATH_MSG_ERROR("Config file does not specify Var" << std::to_string(i) << " cut!") ; 
        return StatusCode::FAILURE;
      }

      m_varCutExprs.push_back(varCutExpr);

      ATH_MSG_DEBUG("Read Var" << std::to_string(i) << " from file:");
      ATH_MSG_DEBUG("    Variable: " << m_varCutNames.back());
      ATH_MSG_DEBUG("    Cut: " << m_varCutExprs.back());
    }

    // get the decoration name
    m_decorationName = m_configReader.GetValue("DecorationName" ,"");
  } else { // no config file

    // determine number of tagger variables from size of vectors
    m_numTaggerVars = int(m_varCutNames.size());

    // make sure all of the tagger variables are recognised
    for (const std::string& var: m_varCutNames) {
      if (std::find(m_recognisedCuts.begin(), m_recognisedCuts.end(), var) == m_recognisedCuts.end()) {
        ATH_MSG_ERROR("Unrecognised variable " << var << " provided!") ;
        return StatusCode::FAILURE;
      }
    }
  } // if config file

  // make sure cut names vector is not empty
  if (m_varCutNames.empty()) {
    ATH_MSG_ERROR( "Tagger variable names vector is empty." ) ;
    return StatusCode::FAILURE;
  }

  // make sure cut expressions vector is not empty
  if (m_varCutExprs.empty()) {
    ATH_MSG_ERROR( "Tagger variable cuts vector is empty." ) ;
    return StatusCode::FAILURE;
  }

  // make sure all vectors have the same length
  if (int(m_varCutNames.size()) != int(m_varCutExprs.size())) {
    ATH_MSG_ERROR( "Tagger variable names and cuts vectors don't have the same size." ) ;
    return StatusCode::FAILURE;
  }  

  // make cut functions into TFI objects
  for (int i = 0; i < m_numTaggerVars; i++) {
    m_varCutFuncs.push_back(
      std::make_unique<TF1>(m_varCutNames[i].c_str(),  m_varCutExprs[i].c_str())
    );
    ATH_MSG_DEBUG("Configured " << m_varCutNames[i] << " : " << m_varCutExprs[i] << " cut TF1");
  }

  ATH_MSG_INFO( "Smoothed top Tagger tool initialized with cuts:" );
  for (int i = 0; i < m_numTaggerVars; i++) {
    ATH_MSG_INFO( m_varCutNames[i]+"   cut : "<< m_varCutExprs[i] );
  }
  ATH_MSG_INFO( "DecorationName  : "<< m_decorationName );

  // add cuts for the output TAccept
  // initialize decorators as decorationName+_decorator
  ATH_MSG_INFO( "Additional decorators that will be attached to jet :" );
  
  if (std::find(m_varCutNames.begin(), m_varCutNames.end(), "Mass") != m_varCutNames.end() || std::find(m_varCutNames.begin(), m_varCutNames.end(), "mass") != m_varCutNames.end()) {
    m_acceptInfo.addCut( "PassMass"       , "mJet > mCut"  );
    // initialize decorators for passing cuts
    // this uses m_decPassMassKey inherited from JSSTaggerBase
    m_decPassMassKey = m_containerName + "." + m_decorationName + "_" + m_decPassMassKey.key();
    ATH_CHECK( m_decPassMassKey.initialize() );
    m_dec_mcut = m_containerName + "." + m_decorationName + "_" + m_dec_mcut.key();
    ATH_CHECK( m_dec_mcut.initialize() );
    ATH_MSG_INFO( "  " << m_dec_mcut.key() << " : mass cut" );
    ATH_MSG_INFO( "  " << m_decPassMassKey.key() << " : pass mass cut");
  }

  if (std::find(m_varCutNames.begin(), m_varCutNames.end(), "Sphericity") != m_varCutNames.end() || std::find(m_varCutNames.begin(), m_varCutNames.end(), "sphericity") != m_varCutNames.end()) {
    m_acceptInfo.addCut( "PassSphericity" , "SphericityJet > SphericityCut"   );
    m_decPassSphericityKey = m_containerName + "." + m_decorationName + "_" + m_decPassSphericityKey.key();
    ATH_CHECK( m_decPassSphericityKey.initialize() );
    m_dec_sphericitycut = m_containerName + "." + m_decorationName + "_" + m_dec_sphericitycut.key();
    ATH_CHECK( m_dec_sphericitycut.initialize() );
    ATH_MSG_INFO( "  " << m_dec_sphericitycut.key() << " : Sphericity cut" );
    ATH_MSG_INFO( "  " << m_decPassSphericityKey.key() << " : pass Sphericity cut" );
  }

  /// Call base class initialize
  ATH_CHECK( JSSTaggerBase::initialize() );

  /// Loop over and print out the cuts that have been configured
  printCuts();

  return StatusCode::SUCCESS;
} // end initialize()

StatusCode SmoothedTopTagger::tag( const xAOD::Jet& jet ) const {

  ATH_MSG_DEBUG( "Obtaining smoothed top result" );

  /// Create asg::AcceptData object
  asg::AcceptData acceptData( &m_acceptInfo );

  /// Reset the AcceptData cut results
  ATH_CHECK( resetCuts( acceptData ) );

  /// Check basic kinematic selection
  ATH_CHECK( checkKinRange( jet, acceptData ) );

  // get the relevant attributes of the jet
  // mass and pt - note that this will depend on the configuration of the calibration used
  float jet_pt   = jet.pt()/1000.0;
  float jet_mass = jet.m()/1000.0;

  /// Calculate NSubjettiness and ECF ratios
  calculateJSSRatios(jet);

  // configure decorators from JSSTaggerBase class
  SG::WriteDecorHandle<xAOD::JetContainer, bool> decValidJetContent(m_decValidJetContentKey);
  SG::WriteDecorHandle<xAOD::JetContainer, bool> decTagged(m_decTaggedKey);

  // initialize for use in other statements
  bool passCuts = true;
  float cut_var;
  for (int i = 0; i < m_numTaggerVars; i++) {
    // evaluate the cut value on this variable
    cut_var = m_varCutFuncs[i]->Eval(jet_pt);

    // check which variable this cut corresponds to and make the 
    // selection
    // when more taggers are implemented add the variables required
    // into this if-else ladder
    if (m_varCutNames[i] == "Mass" || m_varCutNames[i] == "mass") {
      // decorators for jet after applying cuts
      SG::WriteDecorHandle<xAOD::JetContainer, float> decMCut(m_dec_mcut);
      SG::WriteDecorHandle<xAOD::JetContainer, bool> decPassMass(m_decPassMassKey);

      // decorate cut
      decMCut(jet) = cut_var;

      // make cut and decorate on jet
      if(jet_mass > cut_var) {
        acceptData.setCutResult("PassMass",true);
      }
      decPassMass(jet) = acceptData.getCutResult("PassMass");
      passCuts = passCuts && acceptData.getCutResult("PassMass");
    }
    else if (m_varCutNames[i] == "Sphericity" || m_varCutNames[i] == "sphericity") {
      float sphericity = 0;

      // setup read/write handles for sphericity cut decorations & reading variables
      SG::ReadDecorHandle<xAOD::JetContainer, float> readSphericity(m_readSphericityKey);
      SG::WriteDecorHandle<xAOD::JetContainer, float> decSphericityCut(m_dec_sphericitycut);
      SG::WriteDecorHandle<xAOD::JetContainer, bool> decPassSphericity(m_decPassSphericityKey);

      // decorate cut
      decSphericityCut(jet) = cut_var;

      // read sphericity variable
      if ( !readSphericity.isAvailable() ) {
        ATH_MSG_VERBOSE( "The Sphericity variable is not available in your file" );
        acceptData.setCutResult("ValidJetContent", false);
        decValidJetContent(jet) = false;
      } else {
        // get sphericity only if it is decorated
        sphericity = readSphericity(jet);  
      }

      // make cut and decorate results
      if (sphericity > cut_var) {
        acceptData.setCutResult("PassSphericity", true);
      }
      decPassSphericity(jet) = acceptData.getCutResult("PassSphericity");
      // incorporate cut into complete set of selections
      passCuts = passCuts && acceptData.getCutResult("PassSphericity");
    }
  }

  // decorate jets with tagging information and whether content is valid
  decValidJetContent(jet) = acceptData.getCutResult("ValidJetContent");
  decTagged(jet) = passCuts;

  return StatusCode::SUCCESS;
}

