/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArFCalDegTestAlg.h"
#include <sstream>


/** 
 * @brief Standard Gaudi initialize method.
 */
StatusCode LArFCalDegTestAlg::initialize()
{

  ATH_CHECK( m_factorsKey.initialize() );

  return StatusCode::SUCCESS;
}


/** 
 * @brief Standard Gaudi execute method.
 */
StatusCode LArFCalDegTestAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("LArFCalDegTestAlg::execute");
  SG::ReadCondHandle<LArFcalDegFactors> fcalHdl{m_factorsKey, ctx};
  ATH_MSG_DEBUG("handle received");
 
  if(!fcalHdl.isValid()) {
     ATH_MSG_ERROR("Invalid read handle to FcalDegFactors with name: " << m_factorsKey.key());
     return StatusCode::FAILURE;
  } else ATH_MSG_DEBUG("successfully got FcalDegFactors");

  const auto factors = fcalHdl->get();

  if(factors.size() != LArFcalDegFactors::nBins) {
     ATH_MSG_ERROR("Wrong size of FcalDegMap: " << factors.size());
     return StatusCode::FAILURE;
  }

  for(unsigned i=0; i<LArFcalDegFactors::nBins; ++i) {
     if(factors.at(i).size() != LArFcalDegFactors::nValues) {
        ATH_MSG_ERROR("Wrong vector length " << factors.at(i).size() << " for bin " << i);
        return StatusCode::FAILURE;
     } 
  }
  for(unsigned bin=0; bin<factors.size(); ++bin){
   std::ostringstream os;
   os<<bin;
   for(unsigned idx = 0; idx < LArFcalDegFactors::nValues; ++idx){
      os<<" "<<factors.at(bin)[idx];
   }
   ATH_MSG_DEBUG(os.str());
  }
   
  return StatusCode::SUCCESS;
}
