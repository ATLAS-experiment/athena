/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**   
 *   @file ITkPixelCablingAlg.cxx
 *
 *   @brief Fills an ITkPixel cabling object from a plain text source
 *
 *   @author Shaun Roe
 *   @date June 2024
 */

//package includes
#include "ITkPixelCablingAlg.h"

//indet includes
#include "InDetIdentifier/PixelID.h"

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
ITkPixelCablingAlg::ITkPixelCablingAlg(const std::string& name, ISvcLocator* pSvcLocator):
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

//
StatusCode
ITkPixelCablingAlg::initialize() {
  m_source = PathResolver::find_file(m_source.value(), "DATAPATH");
  if (m_source.empty()) {
    ATH_MSG_FATAL("The ITkPixel data file for cabling, " << m_source.value() << ", was not found.");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Reading cabling from " << m_source.value());
  // ITkPixelID
  ATH_CHECK(detStore()->retrieve(m_idHelper, "PixelID"));
  // Write Cond Handle
  ATH_CHECK(m_writeKey.initialize());

  return StatusCode::SUCCESS;
}


//
StatusCode
ITkPixelCablingAlg::execute(const EventContext& ctx) const {
  // Write Cond Handle
  SG::WriteCondHandle<ITkPixelCablingData> writeHandle = SG::makeHandle(m_writeKey, ctx);
  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << writeHandle.fullKey() << " is already valid."
                  << ". In theory this should not be called, but may happen"
                  << " if multiple concurrent events are being processed out of order.");
    return StatusCode::SUCCESS;
  }

  // Construct the output Cond Object and fill it in
  std::unique_ptr<ITkPixelCablingData> pCabling = std::make_unique<ITkPixelCablingData>();
  auto inputFile = std::ifstream(m_source.value());
  if (not inputFile.good()){
    ATH_MSG_ERROR("The itk cabling file "<<m_source.value()<<" could not be opened.");
    return StatusCode::FAILURE;
  }
  inputFile>>*pCabling;
  const int numEntries = pCabling->size();
  ATH_MSG_DEBUG(numEntries << " entries were made to the identifier map.");

  // Define validity of the output cond object and record it
  const EventIDBase start{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, 0, 0, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDBase stop{EventIDBase::UNDEFNUM, EventIDBase::UNDEFEVT, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM-1, EventIDBase::UNDEFNUM, EventIDBase::UNDEFNUM};
  const EventIDRange rangeW{start, stop};
  if (writeHandle.record(rangeW, std::move(pCabling)).isFailure()) {
    ATH_MSG_FATAL("Could not record ITkPixelCablingData " << writeHandle.key() 
                  << " with EventRange " << rangeW
                  << " into Conditions Store");
    return StatusCode::FAILURE;
  }
  ATH_MSG_VERBOSE("recorded new conditions data object " << writeHandle.key() << " with range " << rangeW << " into Conditions Store");
  return (numEntries==0) ? (StatusCode::FAILURE) : (StatusCode::SUCCESS);
}

