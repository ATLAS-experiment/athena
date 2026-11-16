/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Gaudi Includes
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/SmartDataPtr.h"

#include "DetDescrCnvSvc/DetDescrAddress.h"

//TBDetDescr includes
#include "TBDetDescr/TBElementContainer.h"
#include "TBDetDescr/TBDetDescrManager.h"
#include "TBDetDescrLoader.h"

// Constructor & destructor
TBDetDescrLoader::TBDetDescrLoader(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
}

TBDetDescrLoader::~TBDetDescrLoader()
{
}

// Alg standard interface function
StatusCode TBDetDescrLoader::initialize() 
{ 
  StatusCode sc;

  if (m_readAction != 0) {

    ATH_MSG_INFO( "Creating TBDetDescrManager in DetectorStore" );

    m_TBDDM = new TBDetDescrManager();

    // somebody else should provide coordinates for all envelopes
    // we can't do anything here 
    // so this initialization is not needed,
    // it is kept here as an example
    std::vector<TBElement> vec;
    vec.resize(1); // it will contain just one "Unknown" element

    // initialize the manager with detectors we just created
    m_TBDDM->initialize(vec);
    m_TBDDM->print(msg());
    
    if (m_readAction < 0) {
      m_readAction = 0;
      sc=detStore()->record(m_TBDDM, m_TBDetDescrManager.value(), true);
    } else {
        sc=detStore()->record(m_TBDDM, m_TBDetDescrManager.value(), false);
    }
    
    if(sc.isFailure()) {
      ATH_MSG_ERROR( "failed to register TBDetDescrManager in Detector Store" );
      m_readAction = 0;
      ATH_MSG_ERROR( "resetting m_readAction to 0" );
      delete m_TBDDM;
      // do not exit wih FAILURE here
      // probably manager is created already
      // and we just want to use it
      // return sc;
    }
  } else {

    CxxUtils::RefCountedPtr<DetDescrAddress> addr
      (new DetDescrAddress(
                           ClassID_traits<TBDetDescrManager>::ID(),
                           m_TBDetDescrManager, m_TBDetDescrManager));

    sc = detStore()->recordAddress(addr);
    if (sc != StatusCode::SUCCESS) {
      ATH_MSG_INFO( "unable to fill address for " << (*addr->par())
                    << " with CLID " << addr->clID()
                    << " and storage type " << addr->svcType()
                    << " to detector store " );
    }
    else {
      unsigned int st = addr->svcType();
      ATH_MSG_INFO( "filling address for " << (*addr->par())
                    << " with CLID " << addr->clID()
                    << " and storage type " << st
                    << " to detector store " );
    }
  }
  
  if (m_writeAction != 0) {

    ATH_MSG_INFO( "Retrieving TBDetDescrManager from DetectorStore" );
    
    sc = detStore()->retrieve(c_TBDDM);
    if (sc.isFailure()) {
      ATH_MSG_ERROR( "Unable to retrieve TBDetDescrManager from DetectorStore" );
      m_writeAction = 0;
      ATH_MSG_ERROR( "resetting m_writeAction to 0" );
      return StatusCode::RECOVERABLE;
    }
    c_TBDDM->print(msg());
  }

  ATH_MSG_INFO( "initialization completed" );
  return StatusCode::SUCCESS;
}

StatusCode TBDetDescrLoader::execute(const EventContext& /*ctx*/) {
  StatusCode sc;

  TBElementContainer* pTBElementContainer;

  switch (m_readAction.value()) {

  case 0: // do nothing
    ATH_MSG_DEBUG( "readAction: do nothing" );

    // write to StoreGate only if we are not reading from it!

    switch (m_writeAction.value()) {

    case 0: // do nothing
      ATH_MSG_DEBUG( "writeAction: do nothing" );
      break;

    case 1: // write once
      m_writeAction = 0; // don't do anything anymore
                         // but without break proceed to next case
      /* FALLTHROUGH */

    case 2: // write always

      ATH_MSG_DEBUG( "Action: write TBDetDescr to StoreGate" );
      
      pTBElementContainer = new TBElementContainer();

      c_TBDDM->getAll(pTBElementContainer);
      c_TBDDM->print(msg());
  
      sc=evtStore()->record(pTBElementContainer, m_TBElementContainer.value(), false);
      if(sc.isFailure()) {
        ATH_MSG_FATAL( "failed to register the TBElementContainer" );
        return StatusCode::FAILURE;
      }
      break;

    default:
      break;
    }

    break;

  case 1: // read once
    m_readAction = 0; // don't do anything anymore
                      // but without break proceed to next case
    /* FALLTHROUGH */

  case 2: // read always

    ATH_MSG_DEBUG( "Action: read TBDetDescr from StoreGate" );
    {
      const TBElementContainer* pContainer = nullptr;
      ATH_CHECK( evtStore()->retrieve(pContainer,m_TBElementContainer.value()) );
      m_TBDDM->initialize(pContainer);
      m_TBDDM->print(msg());
    }
    break;
      
  default:
    break;

  }
  
  return StatusCode::SUCCESS;
}

StatusCode 
TBDetDescrLoader::finalize() {
  ATH_MSG_INFO( "finalized successfully" );
  return StatusCode::SUCCESS;
}

