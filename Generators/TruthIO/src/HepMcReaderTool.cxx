/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////// 
// HepMcReaderTool.cxx 
// Implementation file for class HepMcReaderTool
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 


// STL includes
#include <algorithm>
#include <cctype>


//HepMC includes
#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/IO_GenEvent.h"
#include "HepMC3/ReaderFactory.h"

// McParticleTools includes
#include "HepMcReaderTool.h"

static const char * const s_protocolSep = ":";

/// Constructors
////////////////
HepMcReaderTool::HepMcReaderTool( const std::string& type, const std::string& name, const IInterface* parent ) : 
  base_class( type, name, parent )
{
  //
  // Property declaration
  // 

  declareProperty( "Input", m_ioFrontendURL = "auto:hepmc.genevent.txt", "Name of the front-end we'll use to read in the HepMC::GenEvent.\nEx: ascii:hepmc.genevent.txt" );
  m_ioFrontendURL.declareUpdateHandler( &HepMcReaderTool::setupFrontend, this );
  declareProperty( "McEventsOutput", m_mcEventsOutputName = "GEN_EVENT", "Output location of the McEventCollection to read out" );
}

/// Destructor
///////////////
HepMcReaderTool::~HepMcReaderTool()
{ 
  ATH_MSG_DEBUG("Calling destructor");
}

/// Athena Algorithm's Hooks
////////////////////////////
StatusCode HepMcReaderTool::initialize()
{
  ATH_MSG_INFO("Initializing " << name() << "...");
  // Get pointer to StoreGateSvc and cache it :
  if ( !evtStore().retrieve().isSuccess() ) {
    ATH_MSG_ERROR("Unable to retrieve pointer to StoreGateSvc");
    return StatusCode::FAILURE;
  }

  // setup frontend
  if ( !m_ioFrontend ) {
    setupFrontend(m_ioFrontendURL);
  }

  return StatusCode::SUCCESS;
}

StatusCode HepMcReaderTool::finalize()
{
  ATH_MSG_INFO("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode HepMcReaderTool::execute()
{
  // create a new McEventCollection and put it into StoreGate
  McEventCollection * mcEvts = new McEventCollection;
  if ( evtStore()->record( mcEvts, m_mcEventsOutputName ).isFailure() ) {
    ATH_MSG_ERROR("Could not record a McEventCollection at ["<< m_mcEventsOutputName << "] !!");
    return StatusCode::FAILURE;
  }
  
  if ( evtStore()->setConst( mcEvts ).isFailure() ) {
    ATH_MSG_WARNING("Could not setConst McEventCollection at ["<< m_mcEventsOutputName << "] !!");
  }

  HepMC::GenEvent * evt = new HepMC::GenEvent;
  mcEvts->push_back(evt);

  return read(evt);
}

StatusCode HepMcReaderTool::read( HepMC::GenEvent* evt )
{
  m_ioFrontend->read_event(*evt);
  return StatusCode::SUCCESS;
}

void HepMcReaderTool::setupFrontend( Gaudi::Details::PropertyBase& /*prop*/ )
{
  // defaults
  std::string protocol = "auto";
  std::string fileName = "hepmc.genevent.txt";

  // reset internal state
  m_ioFrontend = nullptr;

  // caching URL
  const std::string& url = m_ioFrontendURL.value();
  
  std::string::size_type protocolPos = url.find(s_protocolSep);

  if ( std::string::npos != protocolPos ) {
    protocol = url.substr( 0, protocolPos );
    fileName = url.substr( protocolPos + 1, std::string::npos );
  } else {
    fileName = url;
  }

  // get the protocol name in lower cases
  std::transform( protocol.begin(), protocol.end(), protocol.begin(), [](unsigned char c){ return std::tolower(c); } );

  if ( "auto" == protocol ) {
    m_ioFrontend = HepMC3::deduce_reader( fileName.c_str());
  } else if ( "ascii" == protocol ) {
    m_ioFrontend = std::make_shared<HepMC3::ReaderAsciiHepMC2>( fileName.c_str());
  } else {
    msg(MSG::WARNING) << "UNKNOWN protocol [" << protocol << "] !!" << endmsg<< "Will use [ascii] instead..."<< endmsg;
    protocol = "ascii";
    m_ioFrontend = std::make_shared<HepMC3::ReaderAsciiHepMC2>( fileName.c_str());
  }    
  ATH_MSG_DEBUG("Using protocol [" << protocol << "] and write to ["<< fileName << "]");
}
