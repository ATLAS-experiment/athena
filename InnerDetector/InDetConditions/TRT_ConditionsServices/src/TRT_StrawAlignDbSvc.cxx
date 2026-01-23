/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file TRT_StrawAlignDbSvc.cxx
 *  @brief AlgTool to manage TRT Conditions data
 *  @author Peter Hansen <phansen@nbi.dk>
 **/

#include "TRT_StrawAlignDbSvc.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "StoreGate/StoreGateSvc.h"

#include "GaudiKernel/IToolSvc.h"
#include "RegistrationServices/IIOVRegistrationSvc.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"

#include "Identifier/Identifier.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"

ATLAS_NO_CHECK_FILE_THREAD_SAFETY; // This class uses const_cast, regFcn (callback) and DataHandle. Legacy code

TRT_StrawAlignDbSvc::TRT_StrawAlignDbSvc( const std::string& name,
					  ISvcLocator* pSvcLocator )
  : base_class(name,pSvcLocator),
    m_detStore("DetectorStore",name),
    m_par_dxcontainerkey("/TRT/Calib/DX"),
    m_par_strawtextfile(""),
    m_par_forcecallback(false),
    m_trtid(nullptr),
    m_trtman(nullptr),
    m_streamer("AthenaOutputStreamTool/CondStream1")
{
  declareProperty("StrawTextFile",m_par_strawtextfile);
  declareProperty("ForceCallback",m_par_forcecallback);
  declareProperty("StreamTool",m_streamer);
  declareProperty("DetectorStore",m_detStore);
}


TRT_StrawAlignDbSvc::~TRT_StrawAlignDbSvc()
= default;


StatusCode TRT_StrawAlignDbSvc::initialize() 
{
  ATH_MSG_INFO ("TRT_StrawAlignDbSvc initialize method called");

  ATH_CHECK( m_detStore->retrieve(m_trtid,"TRT_ID") );
  ATH_CHECK( m_detStore->retrieve(m_trtman,"TRT") );
  

  bool dxcontainerexists = m_detStore->StoreGateSvc::contains<StrawDxContainer>(m_par_dxcontainerkey) ;
  
  if( dxcontainerexists ) {
    ATH_MSG_INFO (" dx container exists - reg callback ");
    if( (m_detStore->regFcn(&TRT_StrawAlignDbSvc::IOVCallBack,this,m_dxcontainer,m_par_dxcontainerkey)).isFailure()) 
      ATH_MSG_ERROR ("Could not register IOV callback for key: " << m_par_dxcontainerkey);
    
  } else {
    
    // create, record and update data handle
    ATH_MSG_INFO ("Creating new dx container");
    auto dxcontainer = std::make_unique<StrawDxContainer>();

    // reading from file 
    if( !m_par_strawtextfile.empty() ) {
      ATH_CHECK( this->readTextFile(dxcontainer.get(),
                                    m_par_strawtextfile) );
    }

    ATH_CHECK( m_detStore->record(std::move(dxcontainer), m_par_dxcontainerkey) );
  }

  return StatusCode::SUCCESS;
}


StatusCode TRT_StrawAlignDbSvc::finalize()
{
  ATH_MSG_INFO ("TRT_StrawAlignDbSvc finalize method called");
  return StatusCode::SUCCESS;
}

StatusCode TRT_StrawAlignDbSvc::writeTextFile(const std::string& filename) const
{
  ATH_MSG_INFO (" Write straw alignment data to text file " << filename);
  std::ofstream outfile(filename.c_str());


  // first reduce the container as much as possible
  // getDxContainer()->crunch() ;

  StrawDxContainer::FlatContainer packedstrawdata ;
  const StrawDxContainer* dxcontainer = getConstDxContainer();
  dxcontainer->getall( packedstrawdata ) ;
  
  // now, we store the entries
  ATH_MSG_INFO ("Number of entries in flatcontainer: " 
                << packedstrawdata.size());
  
  for(auto & it : packedstrawdata) {
    const TRTCond::ExpandedIdentifier& calid = it.first ;
    // get the end point corrections. if not the right type, store zeros.
    float dx1=dxcontainer->getDx1(calid) ; 
    float dx2=dxcontainer->getDx2(calid) ; 
    float dxerr=dxcontainer->getDxErr(calid) ;
    outfile << calid << " " 
	    << std::setprecision(5)
	    << std::setw(12) << dx1 << " " 
	    << std::setw(12) << dx2 << " " 
	    << std::setw(12) << dxerr << std::endl ; 
  }
  outfile.close() ;
  return StatusCode::SUCCESS ;
}
 
 

StatusCode TRT_StrawAlignDbSvc::readTextFile(const std::string& filename)
{
  return readTextFile( getDxContainer(), filename );
}


StatusCode TRT_StrawAlignDbSvc::readTextFile(StrawDxContainer* dxcontainer,
                                             const std::string& filename) 
{
  ATH_MSG_INFO ("Reading straw alignment data from text file " << filename);

  if(!dxcontainer) {
    ATH_MSG_WARNING (" Could not find the container ");
    return StatusCode::FAILURE;
  }
  dxcontainer->clear() ;
  std::ifstream infile(filename.c_str()) ;
  TRTCond::ExpandedIdentifier id ;
  float dx1,dx2,dxerr;
  int nentries(0) ;
  while ((infile >> id >> dx1 >> dx2 >> dxerr ) ) {
    setDx(id,dx1,dx2,dxerr) ;
    ATH_MSG_DEBUG (" read from file: dx1 " << dx1 << " dx2 " << dx2 << " dxerr " << dxerr);
    ++nentries ;
  }
  size_t dxfootprint = dxcontainer->footprint()  ;
  ATH_MSG_INFO (" read " << nentries << " from file. ");
  ATH_MSG_INFO (" dx footprints " << dxfootprint);
  ATH_MSG_INFO (" (no compression) ");

  // force a call back in the geometry
  (const_cast<InDetDD::TRT_DetectorManager*>(m_trtman))->align().ignore() ;

  return StatusCode::SUCCESS ;
}


StatusCode TRT_StrawAlignDbSvc::streamOutObjects() const
{
  ATH_MSG_INFO ("entering streamOutObjects ");
  
  // Get Output Stream tool for writing
  ATH_CHECK( m_streamer.retrieve() );
  
  IAthenaOutputStreamTool*  streamer=const_cast<IAthenaOutputStreamTool*>(&(*m_streamer));
    
  ATH_CHECK( streamer->connectOutput() );
  
  IAthenaOutputStreamTool::TypeKeyPairs typeKeys;
  typeKeys.push_back( IAthenaOutputStreamTool::TypeKeyPair(StrawDxContainer::classname(),m_par_dxcontainerkey)) ;
  getDxContainer()->crunch() ;
  
  ATH_CHECK( streamer->streamObjects(typeKeys) );
  ATH_CHECK( streamer->commitOutput() );
  
  ATH_MSG_INFO ("   Streamed out and committed "  << typeKeys.size() << " objects ");
  return StatusCode::SUCCESS;
}


StatusCode TRT_StrawAlignDbSvc::registerObjects(std::string tag, int run1, int event1, int run2, int event2) const 
{
  ATH_MSG_INFO ("registerObjects with IOV ");
  ATH_MSG_INFO ("Run/evt1 [" << run1 << "," << event1 << "]");
  ATH_MSG_INFO ("Run/evt2 [" << run2 << "," << event2 << "]");
  
  // get pointer to registration svc
  SmartIF<IIOVRegistrationSvc> regsvc{service("IOVRegistrationSvc")};
  ATH_CHECK( regsvc.isValid() );
  
  if (StatusCode::SUCCESS==regsvc->registerIOV(StrawDxContainer::classname(),
					       m_par_dxcontainerkey,tag,run1,run2,event1,event2))
    ATH_MSG_INFO ("Registered StrawDxContainer object with key " << m_par_dxcontainerkey);
  else 
    ATH_MSG_ERROR ("Could not register StrawDxContainer object with key " << m_par_dxcontainerkey);
  
  return( StatusCode::SUCCESS);
}


StatusCode TRT_StrawAlignDbSvc::IOVCallBack(IOVSVC_CALLBACK_ARGS_P(I,keys))
{
  for (std::list<std::string>::const_iterator 
	 itr=keys.begin(); itr!=keys.end(); ++itr) 
    ATH_MSG_INFO (" IOVCALLBACK for key " << *itr << " number " << I);
  
  // if constants need to be read from textfile, we use the call back routine to refill the IOV objects
  if(!m_par_strawtextfile.empty()) return readTextFile( getDxContainer(), m_par_strawtextfile ) ;
  
  return StatusCode::SUCCESS;
}

