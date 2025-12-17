///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Author: Alberto Plebani <alberto.plebani@cern.ch>, based on earlier implementation by M. Leigh and Bill Balunas <balunas@cern.ch>
///////////////////////////////////////////////////////////////////

// STL Includes
#include <math.h>

// ASG includes
#include "AsgDataHandles/ReadHandle.h"

// Tool headers
#include "METUtilities/METNetSig.h"
#include "METUtilities/METHelpers.h"

// Event info and extra EDMs
#include "xAODBase/IParticleHelpers.h"

// ETMiss EDMs
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAuxContainer.h"

// Physics object EDMs
#include "xAODJet/JetContainer.h"

namespace met {

  static const SG::AuxElement::ConstAccessor<std::string> acc_name("name");
  static const SG::AuxElement::ConstAccessor<MissingETBase::Types::bitmask_t> acc_source("source");

  typedef ElementLink<xAOD::IParticleContainer> iplink_t;
  static const SG::AuxElement::ConstAccessor< std::vector<iplink_t > > acc_constitObjLinks("ConstitObjectLinks");
  static const SG::AuxElement::Decorator< std::vector<iplink_t> > dec_constitObjLinks("ConstitObjectLinks");

  // Decorator for passing the vector of inputs to the met object
  static const SG::AuxElement::ConstAccessor< std::vector<float> > acc_inputvalues("input_values");
  static const SG::AuxElement::ConstAccessor< std::vector<std::string> > acc_inputnames("input_names");
  static const SG::AuxElement::Decorator< std::vector<float> > dec_inputvalues("input_values");
  static const SG::AuxElement::Decorator< std::vector<std::string> > dec_inputnames("input_names");

  //////////////////////////////////////////////////////////////////////////////

  METNetSig::METNetSig(const std::string& name):
    AsgTool(name){}

  //////////////////////////////////////////////////////////////////////////////

  METNetSig::~METNetSig() { }

  //////////////////////////////////////////////////////////////////////////////

  StatusCode  METNetSig::initialize() {
    ATH_MSG_INFO( "Initializing " << name() << "..." );

    if (m_netSigLocation.value().find("dummy") != std::string::npos ) {
      ATH_MSG_WARNING( "You are loading a dummy/untrained METNetSig network! This should only be done when testing the code!\n"
                       " -- If you wish to load a trained network please specify a path to an onnx file using the 'NetworkLocation' argument.\n"
                       " -- A list of supported networks can be found at atlas-groupdata.web.cern.ch/atlas-groupdata/METUtilities/run2_13TeV/METNet/" );
    }

    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_pvContainerKey.initialize());
    ATH_CHECK(m_jetContainerKey.initialize());

    // Tool initialisations
    ATH_MSG_INFO("Initialising the METNet ONNX environment using the file " + m_netSigLocation);
    m_metnetsighandler = std::make_unique<METNetSigHandler>( m_netSigLocation );
    if( m_metnetsighandler->initialize() != 0){
      ATH_MSG_ERROR("METNetSig model file not found!");
      return StatusCode::FAILURE;
    }

    if(m_metmaker_loose.empty()){
      asg::AsgToolConfig toolConfig("met::METMaker/metmaker_loose");
      ATH_CHECK( toolConfig.setProperty("JetContainer", m_jetContainerKey.key()) );
      ATH_CHECK( toolConfig.setProperty("DoPFlow", true) );
      ATH_CHECK( toolConfig.setProperty("JetSelection", "Loose") );
      ATH_CHECK( toolConfig.makePrivateTool(m_metmaker_loose) );
    }
    ATH_CHECK( m_metmaker_loose.retrieve() );

    if(m_metmaker_tight.empty()){
      asg::AsgToolConfig toolConfig("met::METMaker/metmaker_tight");
      ATH_CHECK( toolConfig.setProperty("JetContainer", m_jetContainerKey.key()) );
      ATH_CHECK( toolConfig.setProperty("DoPFlow", true) );
      ATH_CHECK( toolConfig.setProperty("JetSelection", "Tight") );
      ATH_CHECK( toolConfig.makePrivateTool(m_metmaker_tight) );
    }
    ATH_CHECK( m_metmaker_tight.retrieve() );

    if(m_metmaker_tghtr.empty()){
      asg::AsgToolConfig toolConfig("met::METMaker/metmaker_tghtr");
      ATH_CHECK( toolConfig.setProperty("JetContainer", m_jetContainerKey.key()) );
      ATH_CHECK( toolConfig.setProperty("DoPFlow", true) );
      ATH_CHECK( toolConfig.setProperty("JetSelection", "Tighter") );
      ATH_CHECK( toolConfig.makePrivateTool(m_metmaker_tghtr) );
    }
    ATH_CHECK( m_metmaker_tghtr.retrieve() );

    if(m_metmaker_tenac.empty()){
      asg::AsgToolConfig toolConfig("met::METMaker/metmaker_tenac");
      ATH_CHECK( toolConfig.setProperty("JetContainer", m_jetContainerKey.key()) );
      ATH_CHECK( toolConfig.setProperty("DoPFlow", true) );
      ATH_CHECK( toolConfig.setProperty("JetSelection", "Tenacious") );
      ATH_CHECK( toolConfig.makePrivateTool(m_metmaker_tenac) );
    }
    ATH_CHECK( m_metmaker_tenac.retrieve() );

    return StatusCode::SUCCESS;
  }

  //////////////////////////////////////////////////////////////////////////////

  StatusCode METNetSig::rebuildMET( const std::string& metKey,
                                 xAOD::Type::ObjectType metType,
                                 xAOD::MissingETContainer* metCont,
                                 const xAOD::IParticleContainer* collection,
                                 xAOD::MissingETAssociationHelper& helper,
                                 MissingETBase::UsageHandler::Policy objScale ) const
  {
      // If the MET term already exists in the container, skip rebuilding to avoid duplicates
      if ( (*metCont)[metKey] != nullptr ) {
        ATH_MSG_DEBUG("MET term '" << metKey << "' already exists in the container — skipping rebuild in METNetSig.");
        return StatusCode::SUCCESS;
      }

      return m_metmaker_tight->rebuildMET( metKey, metType, metCont, collection, helper, objScale );
  }

  StatusCode METNetSig::rebuildJetMET( const std::string&,
                                    const std::string& softClusKey,
                                    const std::string& softTrkKey,
                                    xAOD::MissingETContainer* metCont,
                                    const xAOD::JetContainer* jets,
                                    const xAOD::MissingETContainer* metCoreCont,
                                    xAOD::MissingETAssociationHelper& helper,
                                    bool ) const {

    // Retrieving the event information (Needed for METSig, so we do it early)
    SG::ReadHandle<xAOD::EventInfo> ei(m_eventInfoKey);

    float avgmu = ei->averageInteractionsPerCrossing();
    float actmu = ei->actualInteractionsPerCrossing();

    //////////////////// BUILDING MET WPS /////////////////////

    // Creating the new containers for all the WPs
    auto metCont_loose = std::make_unique<xAOD::MissingETContainer>();
    auto metCont_looseAux = std::make_unique<xAOD::MissingETAuxContainer>();
    metCont_loose->setStore( metCont_looseAux.get() );

    auto metCont_tight = std::make_unique<xAOD::MissingETContainer>();
    auto metCont_tightAux = std::make_unique<xAOD::MissingETAuxContainer>();
    metCont_tight->setStore( metCont_tightAux.get() );

    auto metCont_tghtr = std::make_unique<xAOD::MissingETContainer>();
    auto metCont_tghtrAux = std::make_unique<xAOD::MissingETAuxContainer>();
    metCont_tghtr->setStore( metCont_tghtrAux.get() );

    auto metCont_tenac = std::make_unique<xAOD::MissingETContainer>();
    auto metCont_tenacAux = std::make_unique<xAOD::MissingETAuxContainer>();
    metCont_tenac->setStore( metCont_tenacAux.get() );

    // Copying the contents of the original container into each WP container
    // Avoids redundant calculation of identical terms.
    ATH_MSG_VERBOSE( "Making copies of base MET container" );
    ATH_CHECK( copyMETContainer( metCont_loose.get(), metCont ) );
    ATH_CHECK( copyMETContainer( metCont_tight.get(), metCont ) );
    ATH_CHECK( copyMETContainer( metCont_tghtr.get(), metCont ) );
    ATH_CHECK( copyMETContainer( metCont_tenac.get(), metCont ) );

    // Selected objects may differ between WPs due to jets, muon-in-jet treatment, etc. Need unique helpers, so copy current state.
    xAOD::MissingETAssociationHelper helper_loose(helper);
    xAOD::MissingETAssociationHelper helper_tight(helper);
    xAOD::MissingETAssociationHelper helper_tghtr(helper);
    xAOD::MissingETAssociationHelper helper_tenac(helper);

    // Building the jet and soft terms for each WP
    ATH_MSG_VERBOSE( "Building jet and soft terms for each MET WP" );
    ATH_CHECK( m_metmaker_loose->rebuildJetMET(   "RefJet", softClusKey, softTrkKey, metCont_loose.get(), jets, metCoreCont, helper_loose, true ) );
    ATH_CHECK( m_metmaker_tight->rebuildJetMET(   "RefJet", softClusKey, softTrkKey, metCont_tight.get(), jets, metCoreCont, helper_tight, true ) );
    ATH_CHECK( m_metmaker_tghtr->rebuildJetMET(   "RefJet", softClusKey, softTrkKey, metCont_tghtr.get(), jets, metCoreCont, helper_tghtr, true ) );
    ATH_CHECK( m_metmaker_tenac->rebuildJetMET(   "RefJet", softClusKey, softTrkKey, metCont_tenac.get(), jets, metCoreCont, helper_tenac, true ) );

    // Building and adding the "Final" MET object to each WP container
    ATH_MSG_VERBOSE( "Building the Final MET object for each MET WP" );
    ATH_CHECK( met::buildMETSum( "FinalTrk",  metCont_loose.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track)) );
    ATH_CHECK( met::buildMETSum( "FinalTrk",  metCont_tight.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track)) );
    ATH_CHECK( met::buildMETSum( "FinalTrk",  metCont_tghtr.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track)) );
    ATH_CHECK( met::buildMETSum( "FinalTrk",  metCont_tenac.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::Track)) );
    // CST uses the LCTopo source bitmask even if the clusters are actually at EM scale
    ATH_CHECK( met::buildMETSum( "FinalClus", metCont_loose.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::LCTopo)) );
    ATH_CHECK( met::buildMETSum( "FinalClus", metCont_tight.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::LCTopo)) );
    ATH_CHECK( met::buildMETSum( "FinalClus", metCont_tghtr.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::LCTopo)) );
    ATH_CHECK( met::buildMETSum( "FinalClus", metCont_tenac.get(), static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Signal::LCTopo)) );

    //////////////////// SETTING UP OUTPUT ////////////////////

    // Initialise two vectors: One for holding the network input values and another for the input names
    std::vector<std::string> input_names;
    std::vector<float> input_values;

    ATH_CHECK( addMETFinal( "Loose", metCont_loose.get(), input_names, input_values ) );

    // Tight gets all its hard terms included as well (same as all th others, only needed once)
    ATH_CHECK( addMETTerm( "Tight", *(metCont_tight->find(MissingETBase::Source::electron())), input_names, input_values ) );
    ATH_CHECK( addMETTerm( "Tight", *(metCont_tight->find(MissingETBase::Source::photon())),   input_names, input_values ) );
    ATH_CHECK( addMETTerm( "Tight", *(metCont_tight->find(MissingETBase::Source::tau())),      input_names, input_values ) );
    ATH_CHECK( addMETTerm( "Tight", *(metCont_tight->find(MissingETBase::Source::muon())),     input_names, input_values ) );
    ATH_CHECK( addMETFinal( "Tight", metCont_tight.get(), input_names, input_values ) );

    ATH_CHECK( addMETFinal( "Tighter", metCont_tghtr.get(), input_names, input_values ) );
    ATH_CHECK( addMETFinal( "Tenacious", metCont_tenac.get(), input_names, input_values ) );

    // Pileup information and tracking information (using the user provided primary vertex container name)
    SG::ReadHandle<xAOD::VertexContainer> pvtxs(m_pvContainerKey);
    if(!pvtxs.isValid()){
      ATH_MSG_ERROR("Could not retrieve primary vertex container!");
      return StatusCode::FAILURE;
    }
    int NVx_2Tracks = 0;
    int NVx_4Tracks = 0;
    int PV_NTracks = 0;
    int n_tracks = 0;
    for ( const xAOD::Vertex* vx : *pvtxs ) {
      n_tracks = vx->nTrackParticles();
      if ( n_tracks>=2 ) NVx_2Tracks++;
      if ( n_tracks>=4 ) NVx_4Tracks++;
      if ( vx->vertexType() == xAOD::VxType::PriVtx ) PV_NTracks = n_tracks;
    }
    ATH_CHECK( addInputValue( "ActualMu",    actmu,       input_names, input_values ) );
    ATH_CHECK( addInputValue( "AverageMu",   avgmu,       input_names, input_values ) );
    ATH_CHECK( addInputValue( "NPV_2Tracks", NVx_2Tracks, input_names, input_values ) );
    ATH_CHECK( addInputValue( "NPV_4Tracks", NVx_4Tracks, input_names, input_values ) );
    ATH_CHECK( addInputValue( "PV_NTracks",  PV_NTracks,  input_names, input_values ) );

    // Creating a dummy MET object on the output container to carry the NN features and decorate it with the two vectors
    ATH_MSG_VERBOSE( "Saving network inputs to base MET container" );
    xAOD::MissingET* net_inpts = nullptr;
      if ( (*metCont)["NetInputDummy"] != NULL ) {
        // If NetInputDummy already exists, erase it from the container so we recreate a clean object.
        ATH_MSG_DEBUG("NetInputDummy already present in container — erasing and recreating.");
        auto it = metCont->find("NetInputDummy");
        if ( it != metCont->end() ) metCont->erase(it);
      }
      ATH_CHECK( fillMET( net_inpts, metCont, "NetInputDummy", static_cast<MissingETBase::Types::bitmask_t>(MissingETBase::Source::Type::UnknownType) ) );
    dec_inputnames( *(*metCont)["NetInputDummy"] ) = input_names;
    dec_inputvalues( *(*metCont)["NetInputDummy"] ) = input_values;
    return StatusCode::SUCCESS;
  }

  StatusCode METNetSig::evaluateNNMETSig( xAOD::MissingETContainer* metCont,
                                    float& met_x, float& met_y, float& sigma_x, float& sigma_y) const
    {

      // First we check that the container has the correct decorations
      ATH_MSG_VERBOSE( "Checking MET container for network inputs" );
      if ( (*metCont)["NetInputDummy"] == NULL ) {
        ATH_MSG_ERROR( "Could not find the NetInputDummy MET object in the container. Did you run rebuildJetMET?" );
        return StatusCode::FAILURE;
      }


      // Pull out the vector from the met containter
      std::vector<float> tmp_inputs = acc_inputvalues(*(*metCont)["NetInputDummy"]);
      ATH_MSG_VERBOSE( "Loaded " << tmp_inputs.size() << " network inputs" );

      // Check that the list of inputs matches the network size
      if ( (int)tmp_inputs.size() != m_metnetsighandler->getReqSize() ) {
        ATH_MSG_ERROR( "The MET container provided " << tmp_inputs.size()
                        << " elements, but the ONNX network needs exactly " << m_metnetsighandler->getReqSize() << "!" );
        return StatusCode::FAILURE;
      }

      // Passing the inputs through the network
      std::vector<float> net_met = m_metnetsighandler->predict( tmp_inputs );

      // // Adding the network output to the Final Met container
      // xAOD::MissingET* metFinal = nullptr;
      // ATH_CHECK( fillMET( metFinal, metCont, totalName, MissingETBase::Source::total() ) );

      met_x = net_met[0];
      met_y = net_met[1];
      sigma_x = net_met[2];
      sigma_y = net_met[3];

      ATH_MSG_DEBUG( "Writing the Final Network MET: (" << met_x << ", " << met_y << ", " << sigma_x << ", " << sigma_y << ")" );

      return StatusCode::SUCCESS;
    }

  /////////////////////////////// PRIVATE METHODS ///////////////////////////////

  StatusCode METNetSig::addMETFinal( const std::string& WP_name,
                                  xAOD::MissingETContainer* met_container,
                                  std::vector<std::string>& name_vec,
                                  std::vector<float>& val_vec ) const
  {
    // Add the jet, soft, and final terms to the vector
    ATH_CHECK( addMETTerm( WP_name, (*met_container)["RefJet"], name_vec, val_vec ) );
    ATH_CHECK( addMETTerm( WP_name, (*met_container)["PVSoftTrk"], name_vec, val_vec ) );
    ATH_CHECK( addMETTerm( WP_name, (*met_container)["FinalTrk"], name_vec, val_vec ) );
    ATH_CHECK( addMETTerm( WP_name, (*met_container)["SoftClus"], name_vec, val_vec ) );
    ATH_CHECK( addMETTerm( WP_name, (*met_container)["FinalClus"], name_vec, val_vec ) );
    return StatusCode::SUCCESS;
  }

  StatusCode METNetSig::addMETTerm( const std::string &WP_name,
                                 xAOD::MissingET* met,
                                 std::vector<std::string>& name_vec,
                                 std::vector<float>& val_vec ) const
  {
    // Build the name of the term from the WP and the Object type from the container
    std::string tname = WP_name + "_" + acc_name(*met);

    // Add the MET components and SumET
    ATH_CHECK( addInputValue( tname+"_mpx",   met->mpx()/1000.,   name_vec, val_vec ) );
    ATH_CHECK( addInputValue( tname+"_mpy",   met->mpy()/1000.,   name_vec, val_vec ) );
    ATH_CHECK( addInputValue( tname+"_sumet", met->sumet()/1000., name_vec, val_vec ) );

    return StatusCode::SUCCESS;
  }

  StatusCode METNetSig::addInputValue( const std::string& var_name,
                                    float value,
                                    std::vector<std::string>& name_vec,
                                    std::vector<float>& val_vec ) const
  {
    // This is the only place we push_back to either vector
    // Ensuring that they are always consistant with each other!!
    ATH_MSG_VERBOSE( "Adding variable: " << var_name << " = " << value << " to the network input vector" );
    name_vec.push_back( var_name );
    val_vec.push_back( value );
    return StatusCode::SUCCESS;
  }

  StatusCode METNetSig::copyMETContainer( xAOD::MissingETContainer* new_container,
                                       const xAOD::MissingETContainer* old_container) const
  {

    // Initiase variables used in the loop
    std::string name;
    xAOD::MissingET* blank_met;
    MissingETBase::Types::bitmask_t source;

    for ( const auto& old_met : *old_container ) {
      blank_met = nullptr;                                                  // Resetting the null pointer
      name = acc_name(*old_met);                                            // Getting the name from the original container
      source = acc_source(*old_met);                                        // Getting the particle type from the original container
      ATH_CHECK( fillMET( blank_met, new_container, name, source ) );       // Initialising the blank met with the new name in the new container
      *(*new_container)[name] = *(*old_container)[name];                    // Copying over the contents from one container to another

      // Also need to copy the original object links
      dec_constitObjLinks(*(*new_container)[name]) = acc_constitObjLinks(*(*old_container)[name]);
    }

    return StatusCode::SUCCESS;
  }

  /////////////////////////////// UNUSED METHODS ///////////////////////////////

  StatusCode METNetSig::rebuildMET( xAOD::MissingET*,
                                 const xAOD::IParticleContainer*,
                                 xAOD::MissingETAssociationHelper&,
                                 MissingETBase::UsageHandler::Policy ) const
  {
    ATH_MSG_ERROR( "METNetSig has not overloaded this version of rebuildMET from IMETMaker!\n"
                   "Please only use this method:\n"
                   "StatusCode METMaker::rebuildMET("
                      "const std::string& metKey,\n"
                      "xAOD::Type::ObjectType metType,\n"
                      "xAOD::MissingETContainer* metCont,\n"
                      "const xAOD::IParticleContainer* collection,\n"
                      "xAOD::MissingETAssociationHelper& helper,\n"
                      "MissingETBase::UsageHandler::Policy objScale\n"
                    ")" );
    return StatusCode::FAILURE;
  }

  StatusCode METNetSig::rebuildMET( xAOD::MissingET*,
                                 const xAOD::IParticleContainer*,
                                 xAOD::MissingETAssociationHelper&,
                                 MissingETBase::UsageHandler::Policy,
                                 bool,
                                 MissingETBase::UsageHandler::Policy ) const

  {
    ATH_MSG_ERROR( "METNetSig has not overloaded this version of rebuildMET from IMETMaker!\n"
                   "Please only use this method:\n"
                   "StatusCode METMaker::rebuildMET("
                      "const std::string& metKey,\n"
                      "xAOD::Type::ObjectType metType,\n"
                      "xAOD::MissingETContainer* metCont,\n"
                      "const xAOD::IParticleContainer* collection,\n"
                      "xAOD::MissingETAssociationHelper& helper,\n"
                      "MissingETBase::UsageHandler::Policy objScale\n"
                    ")" );
    return StatusCode::FAILURE;
  }

  StatusCode METNetSig::rebuildJetMET(const std::string& metJetKey,
                                   const std::string& /*metSoftKey*/,
                                   xAOD::MissingETContainer* metCont,
                                   const xAOD::JetContainer* jets,
                                   const xAOD::MissingETContainer* metCoreCont,
                                   xAOD::MissingETAssociationHelper& helper,
                                   bool doJetJVT) const
  {
    return rebuildJetMET(metJetKey, "SoftClus", "PVSoftTrk", metCont, jets, metCoreCont, helper, doJetJVT);
  }

  StatusCode METNetSig::rebuildJetMET( xAOD::MissingET*,
                                    const xAOD::JetContainer*,
                                    xAOD::MissingETAssociationHelper&,
                                    xAOD::MissingET*,
                                    const xAOD::MissingET*,
                                    xAOD::MissingET*,
                                    const xAOD::MissingET*,
                                    bool,
                                    bool,
                                    std::vector<const xAOD::IParticle*>* ) const
  {
    ATH_MSG_ERROR( "METNetSig has not overloaded this version of rebuildJetMET from IMETMaker!\n"
                   "Please use the method with the following arguments:\n"
                   "StatusCode METNetSig::rebuildJetMET(\n"
                      "const std::string& metJetKey,\n"
                      "const std::string& softClusKey,\n"
                      "const std::string& softTrkKey,\n"
                      "xAOD::MissingETContainer* metCont,\n"
                      "const xAOD::JetContainer* jets,\n"
                      "const xAOD::MissingETContainer* metCoreCont,\n"
                      "xAOD::MissingETAssociationHelper& helper,\n"
                      "bool doJetJVT\n"
                    ")" );
    return StatusCode::FAILURE;
  }

  StatusCode METNetSig::rebuildTrackMET( const std::string&,
                                      const std::string&,
                                      xAOD::MissingETContainer*,
                                      const xAOD::JetContainer*,
                                      const xAOD::MissingETContainer*,
                                      xAOD::MissingETAssociationHelper&,
                                      bool ) const
  {
    ATH_MSG_ERROR( "Please dont use the method METNetSig::rebuildTrackMET!\n"
                   "It has no use in METNetSig and is a holdover from it's inheritance from IMETMaker." );
    return StatusCode::FAILURE;
  }

  StatusCode METNetSig::rebuildTrackMET( xAOD::MissingET*,
                                      const xAOD::JetContainer*,
                                      xAOD::MissingETAssociationHelper&,
                                      xAOD::MissingET*,
                                      const xAOD::MissingET*,
                                      bool ) const
  {
    ATH_MSG_ERROR( "METNetSig does not support Track MET!" );
    return StatusCode::FAILURE;
  }

  StatusCode METNetSig::markInvisible( const xAOD::IParticleContainer*,
                                    xAOD::MissingETAssociationHelper&,
                                    xAOD::MissingETContainer* ) const
  {
    ATH_MSG_ERROR( "METNetSig does not support markInvisible!");
    return StatusCode::FAILURE;
  }

}