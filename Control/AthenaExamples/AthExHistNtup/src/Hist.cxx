///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Hist.cxx 
// Implementation file for class AthEx::Hist
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// AthExHistNtup includes
#include "Hist.h"

// ROOT includes
#include "TH1F.h"

using namespace AthEx;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
Hist::Hist( const std::string& name, 
            ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator ),
  m_hist (0)
{ }

// Destructor
///////////////
Hist::~Hist()
{}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode Hist::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_CHECK( m_histSvc.retrieve() );
  ATH_CHECK( m_evt.initialize() );

  // register our histogram with the svc
  m_hist = new TH1F("h1", "histogram title", 100,0.,100.);
  if (!m_histSvc->regHist("/stat/simple1D/h1", m_hist).isSuccess()) {
    ATH_MSG_ERROR("could not register histogram [h1]");
    delete m_hist; m_hist = 0;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode Hist::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode Hist::execute()
{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  // get event data...
  SG::ReadHandle<xAOD::EventInfo> evt( m_evt );
  if (!evt.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve EventInfo obj");
    return StatusCode::FAILURE;
  } 
  
  int event = evt->eventNumber();
  ATH_MSG_INFO("   EventInfo:  r: " << event << " e: " << evt->eventNumber() );

  // fill the histogram
  m_hist->Fill( float(event), 1.);

  return StatusCode::SUCCESS;
}
