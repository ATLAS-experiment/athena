///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Ntup.cxx 
// Implementation file for class Ntup
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 

// AthExHistNtup includes
#include "Ntup.h"

// ROOT includes
#include "TTree.h"

using namespace AthEx;

/////////////////////////////////////////////////////////////////// 
// Public methods: 
/////////////////////////////////////////////////////////////////// 

// Constructors
////////////////
Ntup::Ntup( const std::string& name, 
            ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator ),
  m_size(0), 
  m_run(0),
  m_event(0),
  m_ntuple(0)
{ }

// Destructor
///////////////
Ntup::~Ntup()
{}

// Athena Algorithm's Hooks
////////////////////////////
StatusCode Ntup::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_CHECK( m_ntSvc.retrieve() );
  ATH_CHECK( m_evt.initialize() );
   
  // register our n-tuple with the svc
  m_ntuple = new TTree("tree1", "tree title");
  if (!m_ntSvc->regTree("/rec/trees/tree1", m_ntuple).isSuccess()) {
    ATH_MSG_ERROR("could not register tree [tree1]");
    delete m_ntuple; m_ntuple = 0;
    return StatusCode::FAILURE;
  }

  // book branches
  m_ntuple->Branch("size", &m_size, "size/i");
  m_ntuple->Branch("run", &m_run, "run/i");
  m_ntuple->Branch("rundata", &m_rundata);

  return StatusCode::SUCCESS;
}

StatusCode Ntup::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

StatusCode Ntup::execute()
{  
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  // clear data...
  m_rundata.clear();
  m_size = 0;
  m_event = 0;
  m_run = 0;

    // get event data...
  SG::ReadHandle<xAOD::EventInfo> evt( m_evt );
  if (!evt.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve EventInfo obj");
    return StatusCode::FAILURE;
  }

  m_event= evt->eventNumber();
  m_size = 2 + (m_event % 3);
  m_run = evt->runNumber();

  m_rundata.resize(m_size);
  m_rundata[0] = m_run;
  m_rundata[1] = m_event;
  for (unsigned int i=2; i < m_size; ++i) {
    m_rundata[i] = i;
  }

  // fill n-tuple
  if (m_ntuple->Fill() == -1) {
    ATH_MSG_WARNING("problems writing n-tuple");
  }

  ATH_MSG_INFO("run: [" << m_run << "]" << endmsg <<
               "size: [" << m_size << "]" << endmsg <<
               "rundata-sz: [" << m_rundata.size() << "]");

  return StatusCode::SUCCESS;
}
