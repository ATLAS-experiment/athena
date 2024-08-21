/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "BunchCrossingIntensityCondTest.h"

  
StatusCode BunchCrossingIntensityCondTest::initialize() {
  ATH_MSG_INFO( "Initializing..." );
     
  ATH_CHECK( m_inputKey.initialize() );
  
  if (m_fileName.size()>0) {
    m_fileOut.open(m_fileName);
    if (m_fileOut.is_open()) {
      ATH_MSG_INFO("Writing to file " << m_fileName);
    }
    else {
      msg(MSG::ERROR) << "Failed to open file " << m_fileName << endmsg;
      return StatusCode::FAILURE;
    }
  }
  else
    ATH_MSG_INFO("Writing to stdout");
  
  return StatusCode::SUCCESS;
}


StatusCode BunchCrossingIntensityCondTest::execute() {

  // Retrieve the object holding the BCID of the current event:
  const EventContext& ctx = Gaudi::Hive::currentContext();


  std::ostream& out = m_fileOut.good() ? m_fileOut : std::cout;

  SG::ReadCondHandle<BunchCrossingIntensityCondData> readHdl(m_inputKey);
  const BunchCrossingIntensityCondData* bccd=*readHdl;

  out << "\nTimestamp:" << ctx.eventID().time_stamp() << " ns:" << ctx.eventID().time_stamp_ns_offset() << std::endl; 

  for (unsigned bcid=0;bcid<4;++bcid) {
    
      printInfo(bccd,bcid,out,0);
      printInfo(bccd,bcid,out,1);
    
  }

  return StatusCode::SUCCESS;
}


void BunchCrossingIntensityCondTest::printInfo(const BunchCrossingIntensityCondData* bccd, unsigned int bcid, std::ostream& out , int channel ) {

  out << "BCID " << bcid<< " LB "<<bccd->GetRunLB();
  out << "Beam1IntensityBCID="<< bccd->GetBeam1IntensityBCID(bcid,channel)  << ", Beam2IntensityBCID=" <<bccd->GetBeam2IntensityBCID(bcid,channel);
  out << std::endl;

}
