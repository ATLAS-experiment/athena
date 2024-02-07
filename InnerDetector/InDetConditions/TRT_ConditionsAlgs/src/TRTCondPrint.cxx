/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <fstream>
#include <iostream>
#include <string>
#include "TRT_ConditionsAlgs/TRTCondPrint.h"
#include "TRT_ConditionsData/BasicRtRelation.h"
#include "TRT_ConditionsData/DinesRtRelation.h"
#include "TRT_ConditionsData/BinnedRtRelation.h"
#include "TRT_ConditionsData/RtRelationFactory.h"


/**  @file TRTCondPrint.cxx
 *
 *   If an output text file is specified, TRT calibration constants in memory are dumped
 *
 * @author Peter Hansen <phansen@nbi.dk>
**/
TRTCondPrint::TRTCondPrint(const std::string& name, ISvcLocator* pSvcLocator)
  :AthAlgorithm   (name, pSvcLocator),
   m_TRTCalDbTool("TRT_CalDbTool",this),
   m_setup(false),
   m_par_caloutputfile(""), // must be either nothing, caliboutput.txt or erroroutput.txt
   m_trtid(0),
   m_detstore("DetectorStore",name)

{
  // declare algorithm parameters
  declareProperty("TRTCalDbTool",m_TRTCalDbTool);
  declareProperty("DetectorStore",m_detstore);
  declareProperty("CalibOutputFile",m_par_caloutputfile);
}

TRTCondPrint::~TRTCondPrint(void)
{}

StatusCode TRTCondPrint::initialize() {

  //
  // Get ID helper
  ATH_CHECK( detStore()->retrieve(m_trtid,"TRT_ID") );

  //
  // Get TRT_CalDbTool
  ATH_CHECK( m_TRTCalDbTool.retrieve() );

  
  return StatusCode::SUCCESS;
}

StatusCode TRTCondPrint::execute(){

  StatusCode sc = StatusCode::SUCCESS;
  //
  // at first event:
  if (!m_setup) {

    //Write text file with same payload as input, as a check
    if (!m_par_caloutputfile.empty()) {
      ATH_MSG_INFO( " Writing calibration constants to text file " << m_par_caloutputfile);
      std::ofstream outfile(m_par_caloutputfile.c_str());
      if(m_par_caloutputfile=="caliboutput.txt") {
         sc = writeCalibTextFile(outfile);
      }else if(m_par_caloutputfile=="erroroutput.txt") {
         sc = writeErrorTextFile(outfile);
      } else {
        ATH_MSG_INFO( " You must use either caliboutput.txt or erroroutput.txt as output file name " );
      }
      outfile.close();
    }

    ATH_MSG_INFO( " Spot check of the T0s in memory ");
    for(std::vector<Identifier>::const_iterator it=m_trtid->straw_layer_begin();it!=m_trtid->straw_layer_end();++it) {

         Identifier id = m_trtid->straw_id(*it,0);
         int bec = m_trtid->barrel_ec(id);
         int lay = m_trtid->layer_or_wheel(id);
         int phi = m_trtid->phi_module(id);
         int slay = m_trtid->straw_layer(id);
      if(phi%10 == 0 && slay%10 == 0) {
        ATH_MSG_INFO( " bec " << bec << " lay " << lay << " phi " << phi << " slay " << slay << " t0 : " << m_TRTCalDbTool->getT0(id));
      }
    }


    m_setup=true;

  }
  
  return sc;
}

StatusCode TRTCondPrint::finalize() {

  StatusCode sc = StatusCode::SUCCESS;
  return sc;
}

StatusCode TRTCondPrint::writeCalibTextFile(std::ostream& outfile) const
{


  // first store rtrelations
  const RtRelationContainer* rtContainer = m_TRTCalDbTool->getRtContainer() ;
  if(!rtContainer) {
    ATH_MSG_ERROR(" TT container could not be retrieved");
    return StatusCode::FAILURE;
  }


  outfile << "# Rtrelation" << std::endl ;
  RtRelationContainer::FlatContainer rtrelations ;
  rtContainer->getall( rtrelations ) ;
  for( RtRelationContainer::FlatContainer::iterator it = rtrelations.begin() ;
       it != rtrelations.end(); ++it) {
    // write the identifier
    outfile << it->first << " : " ;
    // write the rt-relation via the factory
    TRTCond::RtRelationFactory::writeToFile(outfile,**(it->second)) ;
    outfile << std::endl ;
  }

  // now store the t0s
  const StrawT0Container* t0Container = m_TRTCalDbTool->getT0Container() ;
  if(!t0Container) {
    ATH_MSG_ERROR(" T0 container could not be retrieved");
    return StatusCode::FAILURE;
  }

  outfile << "# StrawT0" << std::endl ;
  StrawT0Container::FlatContainer packedstrawdata ;
  t0Container->getall( packedstrawdata ) ;
  ATH_MSG_INFO(" Flat T0 container retrieved with size " << packedstrawdata.size());

  float t0(0), t0err(0);
  for( TRTCond::StrawT0Container::FlatContainer::iterator it = packedstrawdata.begin() ;
       it != packedstrawdata.end(); ++it) {
    TRTCond::ExpandedIdentifier calid = it->first ;
    t0Container->unpack(calid,*it->second,t0,t0err) ;
    outfile << calid << " : " << t0 << " " << t0err << std::endl ;
  }

    
  return StatusCode::SUCCESS ;
}


StatusCode TRTCondPrint::writeErrorTextFile(std::ostream& outfile) const
{
  const RtRelationContainer* errContainer = m_TRTCalDbTool->getErrContainer() ;
  const RtRelationContainer* slopeContainer = m_TRTCalDbTool->getSlopeContainer() ;


  // then store errors2d
  outfile << "# RtErrors" << std::endl ;
  RtRelationContainer::FlatContainer errors ;
  errContainer->getall( errors ) ;
  for( RtRelationContainer::FlatContainer::iterator it = errors.begin() ;
       it != errors.end(); ++it) {
    // write the identifier
    outfile << it->first << " : " ;
    // write the errors via the factory
    TRTCond::RtRelationFactory::writeToFile(outfile,**(it->second)) ;
    outfile << std::endl ;
  }

  // then store slopes
  outfile << "# RtSlopes" << std::endl ;
  RtRelationContainer::FlatContainer slopes ;
  slopeContainer->getall( slopes ) ;
  for( RtRelationContainer::FlatContainer::iterator it = slopes.begin() ;
       it != slopes.end(); ++it) {
    // write the identifier
    outfile << it->first << " : " ;
    // write the slopes via the factory
    TRTCond::RtRelationFactory::writeToFile(outfile,**(it->second)) ;
    outfile << std::endl ;
  }

  return StatusCode::SUCCESS ;
}


TRTCond::ExpandedIdentifier TRTCondPrint::trtcondid( const Identifier& id, int level) const
{
  return TRTCond::ExpandedIdentifier( m_trtid->barrel_ec(id),m_trtid->layer_or_wheel(id),
				      m_trtid->phi_module(id),m_trtid->straw_layer(id),
				      m_trtid->straw(id),level ) ;
}





