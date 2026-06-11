/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "BunchCrossingAverageCondTest.h"

  
StatusCode BunchCrossingAverageCondTest::initialize() {
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

StatusCode BunchCrossingAverageCondTest::execute(const EventContext& ctx) {

  // Retrieve the object holding the BCID of the current event:

  std::ostream& out = m_fileOut.good() ? m_fileOut : std::cout;
  SG::ReadCondHandle<BunchCrossingAverageCondData> readHdl(m_inputKey, ctx);
  const BunchCrossingAverageCondData* bccd=*readHdl;
  out << "\nTimestamp:" << ctx.eventID().time_stamp() << " ns:" << ctx.eventID().time_stamp_ns_offset() << std::endl; 
  for (unsigned channel=0;channel<2;++channel) 
      printInfo(bccd,channel,out);

  return StatusCode::SUCCESS;
}

void BunchCrossingAverageCondTest::printInfo(const BunchCrossingAverageCondData* bccd, unsigned int channel, std::ostream& out  ) {

  out << "channel " << channel;
  out << " GetBeam1Intensity " << bccd->GetBeam1Intensity(channel)
  << " GetBeam2Intensity " << bccd->GetBeam2Intensity(channel)
  << " GetBeam1IntensityAll " << bccd->GetBeam1IntensityAll(channel)
  << " GetBeam2IntensityAll " << bccd->GetBeam2IntensityAll(channel)
  << " GetBeam1IntensitySTD " << bccd->GetBeam1IntensitySTD(channel)
  << " GetBeam2IntensitySTD " << bccd->GetBeam2IntensitySTD(channel)
  << " GetBeam1IntensityAllSTD " << bccd->GetBeam1IntensityAllSTD(channel)
  << " GetBeam2IntensityAllSTD " << bccd->GetBeam2IntensityAllSTD(channel);
  out << std::endl;
  
}
