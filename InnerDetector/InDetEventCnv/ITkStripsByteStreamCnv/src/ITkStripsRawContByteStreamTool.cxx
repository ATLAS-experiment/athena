/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripsRawContByteStreamTool.h"

#include "ByteStreamData/RawEvent.h" 
#include "ByteStreamCnvSvcBase/SrcIdMap.h" 
#include "eformat/SourceIdentifier.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ITkStripsByteStreamCnv/IITkStripsRodEncoder.h"

#include "ITkStripCabling/IITkStripCablingTool.h"


// Constructor 

ITkStripsRawContByteStreamTool::ITkStripsRawContByteStreamTool(const std::string& type, const std::string& name,
                                                   const IInterface* parent) :
  base_class(type, name, parent){
  //nop
}

// Initialize
StatusCode 
ITkStripsRawContByteStreamTool::initialize() {
  ATH_CHECK( m_byteStreamCnvSvc.retrieve() );
  // Retrieve ID mapping
  ATH_CHECK(m_cabling.retrieve());
  ATH_MSG_DEBUG("Retrieved service " << m_cabling);
  // Get the SCT Helper, valid also for ITk strips
  ATH_CHECK(detStore()->retrieve(m_itkStripsIDHelper, "SCT_ID"));
  return StatusCode::SUCCESS;
}

// Finalize

StatusCode 
ITkStripsRawContByteStreamTool::finalize() {
  return StatusCode::SUCCESS;
}

// Convert method

StatusCode 
ITkStripsRawContByteStreamTool::convert(const SCT_RDO_Container* itkStripsRDOCont) const{
  FullEventAssembler<SrcIdMap>* fullEventAssembler = nullptr;
  ATH_CHECK( m_byteStreamCnvSvc->getFullEventAssembler (fullEventAssembler,"SCTRawCont") );
  FullEventAssembler<SrcIdMap>::RODDATA* rod;
  // Set ROD Minor version
  fullEventAssembler->setRodMinorVersion(m_rodBlockVersion);
  ATH_MSG_DEBUG(" Setting Minor Version Number to " << m_rodBlockVersion);
  // Mapping between ROD IDs and the hits in that ROD
  std::map<uint32_t, std::vector<const SCT_RDORawData*>> rdoMap;

  // The following few lines are to make sure there is an entry in the rdoMap for 
  // every ROD, even if there are no hits in it for a particular event 
  // (as there might be ByteStream errors e.g. TimeOut errors).
  std::vector<std::uint32_t> listOfAllRODs;
  m_cabling->getAllRods(listOfAllRODs);
  for (std::uint32_t rod : listOfAllRODs) {
    rdoMap[rod].clear();
  }
  // Loop over the collections in the ITkStrips RDO container
  for (const auto* itkStripsRawColl : *itkStripsRDOCont) {
    if (itkStripsRawColl == nullptr) {
      ATH_MSG_WARNING("Null pointer to ITkStrips RDO collection.");
      continue;
    } else {
      // Collection ID
      Identifier idColl{itkStripsRawColl->identify()};
      IdentifierHash idCollHash{m_itkStripsIDHelper->wafer_hash(idColl)};
      uint32_t robid{m_cabling->getRobIdFromHash(idCollHash)};
      if (robid == 0) continue;
      // Building the ROD ID
      eformat::helper::SourceIdentifier srcIDROB{robid};
      eformat::helper::SourceIdentifier srcIDROD{srcIDROB.subdetector_id(), srcIDROB.module_id()};
      uint32_t rodid{srcIDROD.code()};
      // Loop over RDOs in the collection
      for (const auto* rdo : *itkStripsRawColl) {
        // Fill the ROD/RDO map
        rdoMap[rodid].push_back(rdo);
      }
    }
  }  // End loop over collections

  // Now encode data for each ROD in turn
  for (const auto& rodToRDOs : rdoMap) {
    rod = fullEventAssembler->getRodData(rodToRDOs.first); // Get ROD data address
    m_encoder->fillROD(*rod, rodToRDOs.first, rodToRDOs.second); // Encode ROD data
  }
  return StatusCode::SUCCESS;
}
