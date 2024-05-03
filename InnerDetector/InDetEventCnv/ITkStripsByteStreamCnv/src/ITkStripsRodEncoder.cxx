/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripsRodEncoder.h" 

#include "InDetRawData/SCT_RDORawData.h"
#include "InDetIdentifier/SCT_ID.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"

namespace { // Anonymous namespace
  int 
  rodLinkFromOnlineID(const SCT_OnlineId onlineID){
    const uint32_t fibre{onlineID.fibre()};
    const int formatter{static_cast<int>((fibre/12) & 0x7)};
    const int linkNum{static_cast<int>((fibre - (formatter*12)) & 0xF)};
    const int rodLink{(formatter << 4) | linkNum};
    return rodLink;
  } 
  bool 
  isOdd(const int someNumber){
    return static_cast<bool>(someNumber & 1);
  }
 
 
} // End of anonymous namespace

// Constructor

ITkStripsRodEncoder::ITkStripsRodEncoder(const std::string& type, const std::string& name, 
                               const IInterface* parent) : 
  base_class(type, name, parent){
  //nop
}

// Initialize

StatusCode 
ITkStripsRodEncoder::initialize() {
  ATH_MSG_DEBUG("ITkStripsRodEncoder::initialize()");
  
  // Retrieve cabling tool
  ATH_CHECK(m_cabling.retrieve());
  ATH_MSG_DEBUG("Retrieved tool " << m_cabling);
  ATH_CHECK(detStore()->retrieve(m_itkStripsID, "SCT_ID"));
  const InDetDD::SCT_DetectorManager* itkStripsDetManager{nullptr};
  ATH_CHECK(detStore()->retrieve(itkStripsDetManager, "SCT"));
  const InDetDD::SiDetectorElementCollection* sctDetElementColl{itkStripsDetManager->getDetectorElementCollection()};
  for (const InDetDD::SiDetectorElement* sctDetElement : *sctDetElementColl) {
    if (sctDetElement->swapPhiReadoutDirection()) {
      m_swapModuleID.insert(sctDetElement->identify());
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode ITkStripsRodEncoder::finalize() {
  return StatusCode::SUCCESS;
}

void 
ITkStripsRodEncoder::fillROD(std::vector<uint32_t>& /*vec32Data*/, const uint32_t& /*robID*/, 
                             const std::vector<const SCT_RDORawData*>& /*vecRDOs*/) const { 
  //code to be filled here
  return;
}

//NOTE: For ITkStrips there is not 'group size' but a mask instead
void 
ITkStripsRodEncoder::encodeData(const std::vector<int>& /*vecTimeBins*/, std::vector<uint16_t>& /*vec16Words*/, 
                                const SCT_RDORawData* /*rdo*/, const int& /*groupSize*/, const int& /*strip*/) const {
  ///code to be filled here
  return;
}

//not sure whether this will be needed for ITkStrips
void 
ITkStripsRodEncoder::packFragments(std::vector<uint16_t>& vec16Words, 
                                   std::vector<uint32_t>& vec32Words) const {
  int num16Words{static_cast<int>(vec16Words.size())};
  if (isOdd(num16Words)) {
    // Just add an additional 16bit words to make even size v16 to in the 32 bits word 0x40004000
    vec16Words.push_back(0x4000);
    num16Words++;
  }
  // Now merge 2 consecutive 16 bit words in 32 bit words
  const unsigned short int numWords{2};
  const unsigned short int position[numWords]{0, 16};
  unsigned short int arr16Words[numWords]{0, 0};
  for (int i{0}; i<num16Words; i += numWords) {
    arr16Words[i%numWords]     = vec16Words[i+1];
    arr16Words[(i+1)%numWords] = vec16Words[i];
    const uint32_t uint32Word{set32Bits(arr16Words, position, numWords)};
    vec32Words.push_back(uint32Word);
#ifdef ITkStripsDEBUG
    ATH_MSG_INFO("SCT encoder -> PackFragments: Output rod 0x"<<std::hex<<uint32Word<<std::dec);
#endif
  }
  return;
}


uint32_t 
ITkStripsRodEncoder::set32Bits(const unsigned short int* arr16Words, 
                                   const unsigned short int* position, 
                                   const unsigned short int& numWords) const {
  uint32_t uint32Word{0};
  uint32_t pos{0};
  uint32_t uint16Word{0};
  for (uint16_t i{0}; i<numWords; i++) {
    uint16Word = static_cast<uint32_t>(*(arr16Words+i));
    pos = static_cast<uint32_t>(*(position+i));
    uint32Word |= (uint16Word<<pos);
  } 
  return uint32Word;
}

// Get RDO info functions
int 
ITkStripsRodEncoder::getStrip(const SCT_RDORawData* rdo) const {
  const Identifier rdoID{rdo->identify()};
  return m_itkStripsID->strip(rdoID);
}

Identifier 
ITkStripsRodEncoder::offlineID(const SCT_RDORawData* rdo) const {
  const Identifier rdoId{rdo->identify()};
  return m_itkStripsID->wafer_id(rdoId);
}

uint32_t 
ITkStripsRodEncoder::onlineID(const SCT_RDORawData* rdo) const {
  const Identifier waferID{offlineID(rdo)};
  const IdentifierHash offlineIDHash{m_itkStripsID->wafer_hash(waferID)};
  return static_cast<uint32_t>(m_cabling->getOnlineIdFromHash(offlineIDHash));
}

int 
ITkStripsRodEncoder::getRODLink(const SCT_RDORawData* rdo) const {
  return rodLinkFromOnlineID(onlineID(rdo));
}

int 
ITkStripsRodEncoder::side(const SCT_RDORawData* rdo) const {
  const Identifier rdoID{rdo->identify()};
  int itkSide{m_itkStripsID->side(rdoID)};
  return itkSide;
}



//the following may be needed for ITkStrips, but must have different implementation
uint16_t 
ITkStripsRodEncoder::getHeaderUsingRDO(const SCT_RDORawData* rdo) const {
  const int rodLink{getRODLink(rdo)};
  const uint16_t linkHeader{static_cast<uint16_t>(0x2000 | (m_condensed.value() << 8) | rodLink)};
  return linkHeader;
}

uint16_t 
ITkStripsRodEncoder::getHeaderUsingHash(const IdentifierHash& linkHash, const int& errorWord) const {
  const int rodLink{rodLinkFromOnlineID(m_cabling->getOnlineIdFromHash(linkHash))};
  const uint16_t linkHeader{static_cast<uint16_t>(0x2000 | errorWord | (m_condensed.value() << 8) | rodLink)};
  return linkHeader;
}

uint16_t 
ITkStripsRodEncoder::getTrailer(const int& errorWord) const {
  const uint16_t linkTrailer{static_cast<uint16_t>(0x4000 | errorWord)};
  return linkTrailer;
}

