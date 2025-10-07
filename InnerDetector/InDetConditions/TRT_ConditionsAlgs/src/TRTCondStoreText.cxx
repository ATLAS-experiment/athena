/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "TRTCondStoreText.h"

#include "TRT_ConditionsData/BasicRtRelation.h"
#include "TRT_ConditionsData/DinesRtRelation.h"
#include "TRT_ConditionsData/BinnedRtRelation.h"
#include "TRT_ConditionsData/RtRelationFactory.h"
#include "InDetIdentifier/TRT_ID.h"

#include <fstream>
#include <iostream>
#include <string>


/**  @file TRTCondStoreText.cxx
 *   Algorithm for reading TRT calibration constants from text file and store them in a pool and cool file
 *
 *
 * @author Peter Hansen <phansen@nbi.dk>
**/
TRTCondStoreText::TRTCondStoreText(const std::string& name, ISvcLocator* pSvcLocator):AthAlgorithm   (name, pSvcLocator){}

StatusCode TRTCondStoreText::initialize()
{

    // Get TRT ID helper
    ATH_CHECK(detStore()->retrieve(m_trtid, "TRT_ID"));

    // Format of input text file
    int format = 0;
    if (m_par_caltextfile != ""){
        ATH_MSG_INFO(" input text file supplied " << m_par_caltextfile);
        if (StatusCode::SUCCESS != checkTextFile(m_par_caltextfile, format)){
            ATH_MSG_FATAL("Could not read format of text file" << m_par_caltextfile);
            return StatusCode::FAILURE;
        }
        ATH_MSG_INFO(" Found format " << format << " in text file " << m_par_caltextfile);
    }
    else{
        ATH_MSG_FATAL(" No input text file supplied. Nothing can be done. ");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO(" Read calibration constants from text file " << m_par_caltextfile);
    if (StatusCode::SUCCESS != readTextFile(m_par_caltextfile, format)){
        ATH_MSG_FATAL("Could not read calibration objects from text file " << m_par_caltextfile);
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode TRTCondStoreText::execute(){

  StatusCode sc = StatusCode::SUCCESS;
  return sc;
}

StatusCode TRTCondStoreText::finalize() {

  StatusCode sc = StatusCode::SUCCESS;
  return sc;
}

StatusCode TRTCondStoreText::checkTextFile(const std::string& filename, int& format ) 
{

  StatusCode sc=StatusCode::SUCCESS ;
  std::ifstream infile(filename) ;
  if(!infile) {
    ATH_MSG_ERROR( "Cannot find input file " << filename ) ;
    sc=StatusCode::FAILURE;
  } else {
    // read the format tag. if none, default to 0
    format = 0 ;
    char line[512] ;
    infile.getline(line,512) ;
    std::string linestring(line) ;
    size_t pos = linestring.find("Fileformat") ;
    if(pos != std::string::npos) {
      sscanf(line,"# Fileformat=%d",&format) ;
    } else {
      ATH_MSG_WARNING( "Input file has no Fileformat identifier. Assuming format=0.");
      // 'rewind' the file

      infile.close() ;
      infile.open(filename) ;
    }
  }
  infile.close() ;
  return sc ;
}

StatusCode TRTCondStoreText::readTextFile(const std::string& filename, int& format ) 
{

  StatusCode sc=StatusCode::SUCCESS ;
  std::ifstream infile(filename) ;
  if(!infile) {
    ATH_MSG_ERROR( "Cannot find input file " << filename ) ;
  } else {
    // read the format tag. if none, default to 0
    format = 0 ;
    char line[512] ;
    infile.getline(line,512) ;
    std::string linestring(line) ;
    size_t pos = linestring.find("Fileformat") ;
    if(pos != std::string::npos) {
      sscanf(line,"# Fileformat=%d",&format) ;
    } else {
      ATH_MSG_WARNING( "Input file has no Fileformat identifier. Assuming format=1");
      // 'rewind' the file

      infile.close() ;
      infile.open(filename) ;
    }
    ATH_MSG_INFO( "Reading calibration data from text file " << filename << " format " << format ) ;
    switch(format) {
    case 1: sc=readTextFile_Format1(infile) ; break ;
    case 2: sc=readTextFile_Format2(infile) ; break ;
    case 3: sc=readTextFile_Format3(infile) ; break ;
    default :  sc=readTextFile_Format1(infile) ; break ;
    }
  }
  infile.close() ;

  return sc ;
}


StatusCode TRTCondStoreText::readTextFile_Format1(std::istream& infile) 
{


  enum ReadMode { ReadingRtRelation, ReadingStrawT0, ReadingGarbage } ;
  ReadMode readmode =ReadingGarbage ;

  // The OutputConditionAlg only works with old-style conditions access, so create raw pointers here.

  StrawT0Container*  t0Container = new TRTCond::StrawT0MultChanContainer() ;
  RtRelationContainer* rtContainer = new TRTCond::RtRelationMultChanContainer() ; 
 
  char line[512] ;
  int nrtrelations(0), nstrawt0(0) ;
  while( infile.getline(line,512) ) {
    if(line[0] == '#') {
      // line with tag
      std::string linestring(line) ;
      if(linestring.find("RtRelation") != std::string::npos) {
	readmode = ReadingRtRelation ;
        ATH_MSG_INFO(" Found line with Rt tag ");

      } else if (linestring.find("StrawT0") != std::string::npos) { 
	readmode = ReadingStrawT0 ;
        ATH_MSG_INFO(" Found line with T0 tag ");

      }else readmode = ReadingGarbage ;
    } else if( readmode != ReadingGarbage) {
      std::istringstream is(line) ;
      // read the id
      TRTCond::ExpandedIdentifier id ;
      is >> id ;
      // read the semicolon that end the id
      char dummy ;
      is >> dummy ;
     
      // read the object
      if( readmode == ReadingRtRelation ) {


 	TRTCond::RtRelation* rt = TRTCond::RtRelationFactory::readFromFile(is) ;
 	// coverity[tainted_data]
  rtContainer->set( id,rt); 
	delete rt ;
	++nrtrelations ;
      } else if( readmode == ReadingStrawT0 ) {

	float t0(0), t0err(0) ;
	is >> t0 >> t0err;
	
	//skip straws with t0=0. The
	if(t0==0) continue;
	      // coverity[tainted_data]
        t0Container->setT0( id, t0, t0err );
        //debug
        //id.print();
	//std::cout << " t0 " <<  t0  << " t0err " << t0err << std::endl;
	++nstrawt0 ;
      }
    }
  }

  // Check that containers were filled

  size_t t0footprint = t0Container->footprint()  ;
  size_t rtfootprint = rtContainer->footprint()  ;

  
  ATH_MSG_INFO( "read " << nstrawt0 << " t0 and " << nrtrelations << " rt from file. " 
	<< " t0/rt footprints " << t0footprint << " / " << rtfootprint  ) ;


  //Record the containers the old way for the OutputConditionsAlg

  if(t0Container->initialize().isFailure()) ATH_MSG_WARNING("Could not initialize T0 Container for key " << m_par_t0containerkey);
  ATH_MSG_INFO(" Recording T0 container ");
  if( (detStore()->record(t0Container,m_par_t0containerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record T0 container for key " << m_par_t0containerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(t0Container,m_par_t0containerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for StrawT0Container " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded T0 Container " << m_par_t0containerkey << " with DetStore ");
    }
  }


  ATH_MSG_INFO(" Recording RT container ");
  if( (detStore()->record(rtContainer,m_par_rtcontainerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record RT container for key " << m_par_rtcontainerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(rtContainer,m_par_rtcontainerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for RtRelationContainer " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded RT Container " << m_par_rtcontainerkey << " with DetStore ");
    }
  }


  return StatusCode::SUCCESS ;
}

StatusCode TRTCondStoreText::readTextFile_Format2(std::istream& infile) 
{


  enum ReadMode { ReadingRtRelation, ReadingErrors, ReadingStrawT0, ReadingGarbage } ;
  ReadMode readmode =ReadingGarbage ;
  char line[512] ;
  int nrtrelations(0), nerrors(0), nstrawt0(0) ;

  // The OutputConditionAlg only works with old-style access, so create a raw pointer.
  StrawT0Container*  t0Container = new TRTCond::StrawT0MultChanContainer() ; 
  RtRelationContainer* rtContainer = new TRTCond::RtRelationMultChanContainer() ; 
  RtRelationContainer* errContainer = new TRTCond::RtRelationMultChanContainer() ; 


  while( infile.getline(line,512) ) {
    if(line[0] == '#') {
      // line with tag
      std::string linestring(line) ;
      if(     linestring.find("RtRelation") != std::string::npos) {
	readmode = ReadingRtRelation ;
        ATH_MSG_INFO(" Found line with Rt tag ");
        rtContainer->clear() ;
      } else if(linestring.find("StrawT0") != std::string::npos) {
	readmode = ReadingStrawT0 ;
        ATH_MSG_INFO(" Found line with T0 tag ");
        t0Container->clear() ;
      } else if(linestring.find("RtErrors") != std::string::npos) {
	readmode = ReadingErrors ; 
        ATH_MSG_INFO(" Found line with Error tag ");
        errContainer->clear() ;
      } else { readmode = ReadingGarbage ; }
    } else if( readmode != ReadingGarbage) {
      std::istringstream is(line) ;
      // read the id
      TRTCond::ExpandedIdentifier id ;
      is >> id ;
      // read the semicolon that end the id
      char dummy ;
      is >> dummy ;
      // read the object
      if( readmode == ReadingRtRelation ) {

 	TRTCond::RtRelation* rt = TRTCond::RtRelationFactory::readFromFile(is) ;
 	// coverity[tainted_data]
  rtContainer->set( id,rt); 
	delete rt ;
	++nrtrelations ;

      } else if( readmode == ReadingErrors ) {

 	TRTCond::RtRelation* err = TRTCond::RtRelationFactory::readFromFile(is) ;
 	// coverity[tainted_data]
  errContainer->set( id,err);
	delete err ;
	++nerrors ;

      } else if( readmode == ReadingStrawT0 ) {

	float t0(0), t0err(0) ;
	is >> t0 >> t0err ;
	// coverity[tainted_data]
  t0Container->setT0( id, t0, t0err );

	++nstrawt0 ;
      }
    }
  }

  size_t t0footprint = t0Container->footprint()  ;
  size_t rtfootprint = rtContainer->footprint()  ;
  size_t errfootprint = errContainer->footprint()  ;
  
  ATH_MSG_INFO( "read " << nstrawt0 << " t0 and " << nerrors << " errors and " << nrtrelations << " rt relations " 
		<< " t0/rt/err footprints " << t0footprint << " / " << rtfootprint << " / " << errfootprint ) ;

  //Record the containers the old way for the OutputConditionsAlg
  if(t0Container->initialize().isFailure()) ATH_MSG_WARNING("Could not initialize T0 Container for key " << m_par_t0containerkey);
  ATH_MSG_INFO(" Recording T0 container ");
  if( (detStore()->record(t0Container,m_par_t0containerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record T0 container for key " << m_par_t0containerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(t0Container,m_par_t0containerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for StrawT0Container " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded T0 Container " << m_par_t0containerkey << " with DetStore ");
    }
  }


  ATH_MSG_INFO(" Recording RT container ");
  if( (detStore()->record(rtContainer,m_par_rtcontainerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record RT container for key " << m_par_rtcontainerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(rtContainer,m_par_rtcontainerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for RtRelationContainer " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded RT Container " << m_par_rtcontainerkey << " with DetStore ");
    }
  }

  ATH_MSG_INFO(" Recording Error container ");
  if( (detStore()->record(errContainer,m_par_errcontainerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record Error container for key " << m_par_errcontainerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(errContainer,m_par_errcontainerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for RtRelationContainer " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded Error Container " << m_par_errcontainerkey << " with DetStore ");
    }
  }

  return StatusCode::SUCCESS ;
}


StatusCode TRTCondStoreText::readTextFile_Format3(std::istream& infile)
{

  // The OutputConditionAlg only works with old-style access, so create a raw pointer.
  StrawT0Container*  t0Container = new TRTCond::StrawT0MultChanContainer() ; 
  RtRelationContainer* rtContainer = new TRTCond::RtRelationMultChanContainer() ; 
  RtRelationContainer* errContainer = new TRTCond::RtRelationMultChanContainer() ; 
  RtRelationContainer* slopeContainer = new TRTCond::RtRelationMultChanContainer() ; 


  enum ReadMode { ReadingRtRelation, ReadingErrors, ReadingSlopes, ReadingStrawT0, ReadingGarbage } ;
  ReadMode readmode =ReadingGarbage ;
  char line[512] ;
  int nrtrelations(0), nerrors(0), nslopes(0), nstrawt0(0) ;

  while( infile.getline(line,512) ) {
    if(line[0] == '#') {
      // line with tag
      std::string linestring(line) ;
      if(     linestring.find("RtRelation") != std::string::npos) {
        readmode = ReadingRtRelation ;
        rtContainer->clear() ;
        ATH_MSG_INFO(" Found line with Rt tag ");
      } else if(linestring.find("RtErrors") != std::string::npos) {
        readmode = ReadingErrors ;
        errContainer->clear() ;
        ATH_MSG_INFO(" Found line with Error tag ");
      } else if(linestring.find("RtSlopes") != std::string::npos) {
        readmode = ReadingSlopes ;
        slopeContainer->clear() ;
        ATH_MSG_INFO(" Found line with Slope tag ");
      } else if(linestring.find("StrawT0") != std::string::npos) {
        readmode = ReadingStrawT0 ;
        t0Container->clear() ;
        ATH_MSG_INFO(" Found line with T0 tag ");
      } else { readmode = ReadingGarbage ; }
    } else if( readmode != ReadingGarbage) {
      std::istringstream is(line) ;
      // read the id
      TRTCond::ExpandedIdentifier id ;
      is >> id ;
      // read the semicolon that end the id
      char dummy ;
      is >> dummy ;
      // read the object
      if( readmode == ReadingRtRelation ) {

        TRTCond::RtRelation* rt = TRTCond::RtRelationFactory::readFromFile(is) ;
        // coverity[tainted_data]
        rtContainer->set( id,rt); 
        delete rt ;
        ++nrtrelations ;

      } else if( readmode == ReadingErrors ) {

        TRTCond::RtRelation* err = TRTCond::RtRelationFactory::readFromFile(is) ;
        // coverity[tainted_data]
        errContainer->set( id,err);
        delete err ;
        ++nerrors ;

      } else if( readmode == ReadingSlopes ) {

        TRTCond::RtRelation* slope = TRTCond::RtRelationFactory::readFromFile(is) ;
        // coverity[tainted_data]
        slopeContainer->set( id,slope);
        delete slope ;
        ++nslopes ;
	
      } else if( readmode == ReadingStrawT0 ) {

        float t0(0), t0err(0) ;
        is >> t0 >> t0err ;
        // coverity[tainted_data]
        t0Container->setT0( id, t0, t0err );
        ++nstrawt0 ;
      }
    }
  }

  size_t t0footprint = t0Container->footprint()  ;
  size_t rtfootprint = rtContainer->footprint()  ;
  size_t errfootprint = errContainer->footprint()  ;
  size_t slopefootprint = slopeContainer->footprint()  ;
  
  ATH_MSG_INFO( "read " << nstrawt0 << " t0 and " << nerrors << " errors and " << nrtrelations << " rt relations and " << nslopes << " error slopes " 
		<< " t0/rt/err/slope footprints " << t0footprint << " / " << rtfootprint << " / " << errfootprint << " / " << slopefootprint) ;


  //Record the containers the old way for the OutputConditionsAlg
  if(t0Container->initialize().isFailure()) ATH_MSG_WARNING("Could not initialize T0 Container for key " << m_par_t0containerkey);
  ATH_MSG_INFO(" Recording T0 container ");
  if( (detStore()->record(t0Container,m_par_t0containerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record T0 container for key " << m_par_t0containerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(t0Container,m_par_t0containerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for StrawT0Container " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded T0 Container " << m_par_t0containerkey << " with DetStore ");
    }
  }


  ATH_MSG_INFO(" Recording RT container ");
  if( (detStore()->record(rtContainer,m_par_rtcontainerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record RT container for key " << m_par_rtcontainerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(rtContainer,m_par_rtcontainerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for RtRelationContainer " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded RT Container " << m_par_rtcontainerkey << " with DetStore ");
    }
  }

  ATH_MSG_INFO(" Recording Error container ");
  if( (detStore()->record(errContainer,m_par_errcontainerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record Error container for key " << m_par_errcontainerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(errContainer,m_par_errcontainerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for RtRelationContainer " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded Error Container " << m_par_errcontainerkey << " with DetStore ");
    }
  }

  ATH_MSG_INFO(" Recording Slope container ");
  if( (detStore()->record(slopeContainer,m_par_slopecontainerkey))!=StatusCode::SUCCESS ) {
     ATH_MSG_ERROR("Could not record Slope container for key " << m_par_slopecontainerkey << " with DetStore ");
     return StatusCode::FAILURE;
  } else {
    if(StatusCode::SUCCESS!=detStore()->retrieve(slopeContainer,m_par_slopecontainerkey)) {
      ATH_MSG_FATAL(  "Could not retrieve data handle for RtRelationContainer " );
      return StatusCode::FAILURE ;
    } else { 
     ATH_MSG_INFO("Successfully recorded Slope Container " << m_par_slopecontainerkey << " with DetStore ");
    }
  }


  return StatusCode::SUCCESS ;
}


TRTCond::ExpandedIdentifier TRTCondStoreText::trtcondid( const Identifier& id, int level) const
{
  return TRTCond::ExpandedIdentifier( m_trtid->barrel_ec(id),m_trtid->layer_or_wheel(id),
				      m_trtid->phi_module(id),m_trtid->straw_layer(id),
				      m_trtid->straw(id),level ) ;
}





