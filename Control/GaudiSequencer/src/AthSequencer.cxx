///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// AthSequencer.cxx
// Implementation file for class AthSequencer
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////

// AthSequencer class
// Implements:
// 1) Common functionality of IInterface
// 2) Default behaviour for the IAlgorithm

#include "AthSequencer.h"

#include "GaudiKernel/IAlgManager.h"
#include "GaudiKernel/ISvcLocator.h"
#include "AthenaBaseComps/DynamicDataHelper.h"


/**
 ** Constructor(s)
 **/
AthSequencer::AthSequencer( const std::string& name, 
                            ISvcLocator* pSvcLocator ):
  ::AthCommonDataStore<AthCommonMsg<Gaudi::Sequence>>   ( name, pSvcLocator ),
  m_clidSvc("ClassIDSvc/ClassIDSvc", name)
{
  m_names.declareUpdateHandler( &AthSequencer::membershipHandler, this );
}

/**
 ** Destructor
 **/
AthSequencer::~AthSequencer()
{}

StatusCode
AthSequencer::initialize()
{
  if (!decodeMemberNames().isSuccess()) {
    ATH_MSG_ERROR ("Unable to configure one or more sequencer members ");
    return StatusCode::FAILURE;
  }

  Ath::DynamicDataHelper dynamic_data_helper;
  StatusCode sc(StatusCode::SUCCESS);
  // Loop over all sub-algorithms
  for (Gaudi::Algorithm* theAlgorithm : *subAlgorithms()) {
    if (!theAlgorithm->sysInitialize( ).isSuccess()) {
      ATH_MSG_ERROR ("Unable to initialize Algorithm "
                     << theAlgorithm->type() << "/" << theAlgorithm->name());
      sc= StatusCode::FAILURE;
    }
    else if (m_runPostInitialize) {
       // visit all algorithms and its tools to gather their input, output handles and dynamic data consumers
       dynamic_data_helper.gatherDataHandlesAndDynamicConsumers(this->name(),theAlgorithm);
    }
  }
  if (sc.isSuccess() && m_runPostInitialize) {
     ATH_MSG_DEBUG("Allow dynamic data consumers to update their data dependencies.");
     if (!m_undeclaredOutputData.empty()) {
        ATH_CHECK( m_clidSvc.retrieve() );
        ATH_CHECK( dynamic_data_helper.addExtraDependencies(*m_clidSvc, m_undeclaredOutputData.value(), msg() ) );
     }
     dynamic_data_helper.updateDataNeeds(m_maxPass, msg());
  }
  return sc;
}

StatusCode
AthSequencer::reinitialize()
{
  StatusCode sc(StatusCode::SUCCESS);
  // Bypass the loop if this sequencer is disabled
  if ( isEnabled( ) ) {
    
    // Loop over all members calling their reinitialize functions
    // if they are not disabled.
    for (Gaudi::Algorithm* theAlgorithm : *subAlgorithms()) {
      if ( theAlgorithm->isEnabled( ) ) {
        if (theAlgorithm->sysReinitialize( ).isFailure()) {
          ATH_MSG_ERROR ("Unable to reinitialize Algorithm "
                         << theAlgorithm->type () << "/"
                         << theAlgorithm->name());
          sc = StatusCode::FAILURE;
        }
      }
    }
  }
  return sc;
}

StatusCode
AthSequencer::execute( const EventContext& ctx ) const
{
  StatusCode sc = StatusCode::SUCCESS;
  bool seqPass = !m_modeOR;

  ATH_MSG_DEBUG ("Executing " << name() << "...");

  auto state = execState( ctx );
  
  // Bypass the loop if this sequencer is disabled or has already been executed
  if ( isEnabled( ) && state.state() != AlgExecState::State::Done ) {

    // Prevent multiple executions of this sequencer for the current event
    state.setState( AlgExecState::State::Executing );

    // Loop over all algorithms calling their execute functions if they
    // are (a) not disabled, and (b) aren't already executed. Note that
    // in the latter case the filter state is still examined. Terminate
    // the loop if an algorithm indicates that it's filter didn't pass.
    const std::vector<Gaudi::Algorithm*>* subAlgms = subAlgorithms( );
    for (auto theAlgorithm : *subAlgms) {
      if ( theAlgorithm->isEnabled( ) ) {
        if ( theAlgorithm->execState(ctx).state() == AlgExecState::State::None ) {
          sc = executeAlgorithm (theAlgorithm, ctx);
        }
	
        if ( sc.isSuccess() ) {
	  
          if ( !m_ignoreFilter ) {
            // Take the filter passed status of this algorithm as my own status
            const bool passed = theAlgorithm->execState( ctx ).filterPassed();
            if ( m_invert ) {
              state.setFilterPassed( !passed );
            } else {
              state.setFilterPassed( passed );
            }
            
            // The behaviour when the filter fails depends on the 
            // StopOverride property.
            // The default action is to stop processing, but this default can be
            // overridden by setting the "StopOverride" property to false.
            if ( m_modeOR ? passed : !passed ) {
              seqPass = passed;
              if ( !m_stopOverride ) break;
            }
          }
        } else {
          ATH_MSG_INFO ("execute of [" << theAlgorithm->name() << "] did NOT succeed");
          break;
        }
      }
    }
  }

  if ( !m_ignoreFilter && !m_names.empty() ) {
    if ( m_invert ) {
      state.setFilterPassed( !seqPass );
    } else {
      state.setFilterPassed( seqPass );
    }
  }

  state.setState( AlgExecState::State::Done );

  return sc;
}


StatusCode AthSequencer::executeAlgorithm (Gaudi::Algorithm* theAlgorithm,
                                           const EventContext& ctx) const
{
  // Call the sysExecute() of the method the algorithm
  return theAlgorithm->sysExecute( ctx );
}

StatusCode
AthSequencer::start()
{
  StatusCode sc(StatusCode::SUCCESS);
  // Loop over all sub-algorithms
  for (Gaudi::Algorithm* theAlgorithm : *subAlgorithms()) {
    if (!theAlgorithm->sysStart( ).isSuccess()) {
      ATH_MSG_ERROR ("Unable to start Algorithm "
                     << theAlgorithm->type () << "/"
                     << theAlgorithm->name());
      sc = StatusCode::FAILURE;
    }
  }
  return sc;
}

StatusCode
AthSequencer::stop()
{
  StatusCode sc(StatusCode::SUCCESS);
  // Loop over all sub-algorithms if they are not disabled.
  for (Gaudi::Algorithm* theAlgorithm : *subAlgorithms()) {
    if (theAlgorithm->sysStop( ).isFailure()) {
      ATH_MSG_ERROR ("Unable to stop Algorithm "
                     << theAlgorithm->type () << "/"
                     << theAlgorithm->name());
      sc = StatusCode::FAILURE;
    }
  }
  return sc;
}

void
AthSequencer::resetExecuted( const EventContext& ctx ) const
{
  execState(ctx).reset();
  
  // Loop over all members calling their resetExecuted functions
  // if they are not disabled.
  for (Gaudi::Algorithm* theAlgorithm : *subAlgorithms()) {
    theAlgorithm->execState(ctx).reset();
  }
}

bool
AthSequencer::isStopOverride( ) const
{
  return m_stopOverride.value( );
}

StatusCode
AthSequencer::append( Gaudi::Algorithm* pAlgorithm )
{
  return append( pAlgorithm, subAlgorithms( ) );
}

StatusCode
AthSequencer::createAndAppend( const std::string& type,
                               const std::string& name,
                               Gaudi::Algorithm*& pAlgorithm )
{
  return createAndAppend( type, name, pAlgorithm, subAlgorithms( ) );
}

StatusCode
AthSequencer::remove( Gaudi::Algorithm* pAlgorithm )
{
  return remove (pAlgorithm->name());
}

StatusCode
AthSequencer::remove( const std::string& algname )
{
  return remove( algname, subAlgorithms( ) );
}

StatusCode
AthSequencer::decodeMemberNames( )
{
  // Decode the membership list
  return decodeNames( m_names, subAlgorithms() );
}

void
AthSequencer::membershipHandler( Gaudi::Details::PropertyBase& /* theProp */ )
{
  if ( isInitialized() ) decodeMemberNames().ignore();
}

/**
 ** Protected Member Functions
 **/

StatusCode
AthSequencer::append( Gaudi::Algorithm* pAlgorithm,
                      std::vector<Gaudi::Algorithm*>* theAlgs )
{
  bool all_good = true;
  // Check that the specified algorithm doesn't already exist 
  // in the membership list
  for (Gaudi::Algorithm* theAlgorithm : *theAlgs) {
    if ( theAlgorithm == pAlgorithm ) {
      all_good = false;
      break;
    }
  }
  if ( all_good ) {
    theAlgs->push_back( pAlgorithm );
    pAlgorithm->addRef();
  }
  return all_good ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

StatusCode 
AthSequencer::createAndAppend( const std::string& type,
                               const std::string& algName,
                               Gaudi::Algorithm*& pAlgorithm,
                               std::vector<Gaudi::Algorithm*>* theAlgs )
{
  SmartIF<IAlgManager> theAlgMgr(Gaudi::svcLocator()->as<IAlgManager>());
  IAlgorithm* tmp = nullptr;

  ATH_CHECK( theAlgMgr->createAlgorithm( type, algName, tmp ) );
  pAlgorithm = dynamic_cast<Gaudi::Algorithm*>(tmp);
  theAlgs->push_back( pAlgorithm );

  return StatusCode::SUCCESS;
}

StatusCode
AthSequencer::decodeNames( Gaudi::Property<std::vector<std::string>>& theNames,
                           std::vector<Gaudi::Algorithm*>* theAlgs )
{
  SmartIF<IAlgManager> theAlgMgr(Gaudi::svcLocator()->as<IAlgManager>());

  // Clear the existing list of algorithms
  theAlgs->clear( );

  // Build the list of member algorithms from the contents of the
  // theNames list.
  StatusCode result = StatusCode::SUCCESS;
  for (const std::string& name : theNames.value()) {

    // Parse the name for a syntax of the form <type>/<name>
    Gaudi::Utils::TypeNameString tn(name);

    // Check whether the supplied name corresponds to an existing
    // Algorithm object.
    SmartIF<IAlgorithm>& theIAlg = theAlgMgr->algorithm(tn.name(), /*createIf*/false);
    Gaudi::Algorithm* theAlgorithm = nullptr;
    if ( theIAlg ) {
      theAlgorithm = dynamic_cast<Gaudi::Algorithm*>(theIAlg.get());
      if ( theAlgorithm ) {
        // The specified Algorithm already exists -
        // just append it to the membership list.
        if ( append(theAlgorithm, theAlgs).isSuccess( ) ) {
          ATH_MSG_DEBUG (tn.name() << " already exists - appended to member list");
        } else {
          ATH_MSG_WARNING (tn.name() << " already exists - append failed!!!");
          result = StatusCode::FAILURE;
        }
      }
      else {
        ATH_MSG_WARNING (tn.name() << " is not an Algorithm - Failed dynamic cast");
        result = StatusCode::FAILURE;
      }
    } else {
      // The specified name doesn't exist -
      // create a new object of the specified type and append it to
      // the membership list.
      if ( createAndAppend(tn.type(), tn.name(), theAlgorithm, theAlgs).isSuccess( ) ) {
        ATH_MSG_DEBUG (tn.name() << " doesn't exist - created and appended to member list");
      } else {
        ATH_MSG_WARNING (tn.name() << " doesn't exist - creation failed!!!");
        result = StatusCode::FAILURE;
      }
    }
  } //> loop over names

  // Print membership list
  if (msgLvl(MSG::DEBUG)) {
    if ( result.isSuccess() && !theAlgs->empty() ) {

      msg(MSG::DEBUG) << "Member list: ";
      bool first = true;
      for (Gaudi::Algorithm* alg : *theAlgs) {
        if (first)
          first = false;
        else
          msg() << ", ";
        if ( alg->name() == System::typeinfoName(typeid(*alg)))
          msg() << alg->name();
        else
          msg() << System::typeinfoName(typeid(*alg)) << "/" << alg->name();
      }
      msg(MSG::DEBUG) << endmsg;
    }
  }
  return result;
}

StatusCode
AthSequencer::remove( const std::string& /*algname*/,
                      std::vector<Gaudi::Algorithm*>* /*theAlgs*/ )
{
  ATH_MSG_ERROR ("AthSequencer::remove( ) is not supported");
  return StatusCode::FAILURE;
}
