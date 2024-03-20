/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MM_RDO_Decoder.h"

using namespace Muon;

MM_RDO_Decoder::MM_RDO_Decoder(const std::string& type, const std::string& name,const IInterface* parent) :
  AthAlgTool(type,name,parent),
  m_mmIdHelper(nullptr)
{  
  declareInterface< Muon::IMM_RDO_Decoder  >( this );
}

StatusCode MM_RDO_Decoder::initialize() {
   
  ATH_CHECK(detStore()->retrieve(m_mmIdHelper, "MMIDHELPER"));
  ATH_CHECK(m_calibTool.retrieve());

  return StatusCode::SUCCESS;
}


std::unique_ptr<MmDigit> Muon::MM_RDO_Decoder::getDigit(const EventContext& ctx,
                                                        const Muon::MM_RawData* data) const {

  // unit conversion
  const Identifier Id = data->identify(); 
  int tdo             = data->time();
  int pdo             = data->charge();
  uint16_t relBcid    = data->relBcid();
  // MM_RawData has time and charge in counts, need physical units
  float charge{0}, time{0.};
  m_calibTool->tdoToTime  (ctx, data->timeAndChargeInCounts(), tdo, Id, time  , relBcid); 
  m_calibTool->pdoToCharge(ctx, data->timeAndChargeInCounts(), pdo, Id, charge         ); 
  
  
  // MM_RawData is built using only the first 4 values. The others are now simply filled proper objects. 
  return std::make_unique<MmDigit>(Id, time, charge);
}
