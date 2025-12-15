/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**   
 *   @file ITkStripCablingAlg.cxx
 *
 *   @brief Fills an ITkStrip cabling object from a plain text source (based on ITkPixelCabling package)
 *
 *   @author Edson Carquin
 *   @date September 2024
 */

//package includes
#include "ITkStripCablingAlg.h"

//indet includes
#include "InDetIdentifier/SCT_ID.h"

//Athena includes
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "PathResolver/PathResolver.h"

// Gaudi include
#include "GaudiKernel/EventIDRange.h"

//STL
#include <iostream>
#include <fstream>



// Constructor
ITkStripCablingAlg::ITkStripCablingAlg(const std::string& name, ISvcLocator* pSvcLocator):
  AthCondAlgorithm(name, pSvcLocator)  
{}

//
StatusCode
ITkStripCablingAlg::initialize() {
  m_configFilePath = PathResolverFindCalibFile(m_source);
  if(m_configFilePath.empty()){
    ATH_MSG_ERROR("Failed to find Calibration file: " << m_source);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Reading ITk Strip cabling file from " << m_configFilePath);
  // ITkStripID
  ATH_CHECK(detStore()->retrieve(m_idHelper, "SCT_ID"));
  // Write Cond Handle
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}


//
StatusCode
ITkStripCablingAlg::execute(const EventContext& ctx) const {
  // Write Cond Handle
  SG::WriteCondHandle<ITkStripCablingData> writeHandle = SG::makeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                  << ". In theory this should not be called, but may happen"
                  << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<ITkStripCablingData> pCabling = std::make_unique<ITkStripCablingData>();
  auto inputFile = std::ifstream(m_configFilePath);
  if (not inputFile.good()){
    ATH_MSG_ERROR("The itk cabling file "<<m_configFilePath<<" could not be opened.");
    return StatusCode::FAILURE;
  }
  inputFile>>*pCabling;
  ATH_MSG_DEBUG("Cabling file size: " << pCabling->size());
  const int numEntries = pCabling->size();
  ATH_MSG_DEBUG(numEntries << " entries were made to the identifier map.");

  // Define validity of the output cond object and record it
  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0, 0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDRange rangeW{start, stop};
  if (writeHandle.record(rangeW, std::move(pCabling)).isFailure()) {
    ATH_MSG_FATAL("Could not record ITkStripCablingData " << writeHandle.key() 
                  << " with EventRange " << rangeW
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_VERBOSE("recorded new conditions data object " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
  return (numEntries==0) ? (StatusCode::FAILURE) : (StatusCode::SUCCESS);
}

