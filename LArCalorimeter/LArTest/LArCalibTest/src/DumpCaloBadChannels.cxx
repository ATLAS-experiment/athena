/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArCalibTest/DumpCaloBadChannels.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/StoreGateSvc.h"
#include <iostream>
#include <fstream>

DumpCaloBadChannels::DumpCaloBadChannels(const std::string & name, 
                                         ISvcLocator * pSvcLocator) : 
  AthAlgorithm(name,pSvcLocator),
  m_caloBCT("CaloBadChanTool"),
  m_cellID(0)

{
  declareProperty("FileName",m_fileName="");
}


DumpCaloBadChannels::~DumpCaloBadChannels() {}

                            
StatusCode DumpCaloBadChannels::initialize() {
  ATH_MSG_INFO ( "in Intialize()" );
  ATH_CHECK( detStore()->retrieve(m_cellID,"CaloCell_ID") );
  ATH_CHECK( m_caloBCT.retrieve() );
  return StatusCode::SUCCESS;
}




StatusCode  DumpCaloBadChannels::finalize() {
  ATH_MSG_INFO ( "in finalize()" );
  std::ostream *out = &(std::cout); 
  std::ofstream outfile;
  if (m_fileName.size()) {
    outfile.open(m_fileName.c_str(),std::ios::out);
    if (outfile.is_open()) {
      ATH_MSG_INFO ( "Writing to file " << m_fileName );
      out = &outfile;
    }
    else
      ATH_MSG_ERROR ( "Failed to open file " << m_fileName );
  }

  const EventContext& ctx = Gaudi::Hive::currentContext();
  for (const Identifier& id : m_cellID->cell_range()) {
    CaloBadChannel bc=m_caloBCT ->caloStatus(ctx, id);
    (*out) << "Chan: 0x" << std::hex << id.get_compact() << ": " << bc.packedData() << std::dec << std::endl;
  }


  if (outfile.is_open())
      outfile.close();
  return StatusCode::SUCCESS;
}
StatusCode  DumpCaloBadChannels::execute(const EventContext& /*ctx*/) { 
  return StatusCode::SUCCESS;
}


