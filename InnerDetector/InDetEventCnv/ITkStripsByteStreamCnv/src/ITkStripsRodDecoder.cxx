/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripsRodDecoder.h"

#include "ByteStreamData/RawEvent.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "Identifier/IdentifierHash.h"

#include <algorithm>
#include <array>
#include <utility>

// Initialize

StatusCode ITkStripsRodDecoder::initialize()
{

  // Retrieve cabling tool
  ATH_CHECK(m_cabling.retrieve());
  ATH_MSG_DEBUG("Retrieved tool " << m_cabling);
  ATH_CHECK(detStore()->retrieve(m_itkStripsID,"SCT_ID"));
  m_contextITk = m_itkStripsID->wafer_context();
  m_swapPhiReadoutDirection.resize(m_itkStripsID->wafer_hash_max(), false);
  ATH_CHECK(m_configTool.retrieve());
  const InDetDD::SCT_DetectorManager* itkStripsDetManager{nullptr};
  ATH_CHECK(detStore()->retrieve(itkStripsDetManager, "ITkStrip"));
  const InDetDD::SiDetectorElementCollection* sctDetElementColl{itkStripsDetManager->getDetectorElementCollection()};
  for (const InDetDD::SiDetectorElement* element: *sctDetElementColl) {
    if (element->swapPhiReadoutDirection()) {
      m_swapPhiReadoutDirection[element->identifyHash()] = true;
    }
  }
  ATH_MSG_DEBUG("Initialization was successful");
  return StatusCode::SUCCESS;
}

// fillCollection method
StatusCode ITkStripsRodDecoder::fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
					                                    SCT_RDO_Container& /*rdoIDCont*/,
                                              IDCInDetBSErrContainer& errorsIDC,
                                              DataPool<SCT3_RawData>* dataItemsPool,
                                              const EventContext& /*ctx*/,
                                              const std::vector<IdentifierHash>* vecHash) const
{
  SCT_RodDecoderErrorsHelper errs{errorsIDC}; // on destruction will fill the IDC

  StatusCode sc{StatusCode::SUCCESS};

  SharedData data;
  data.reset();

  CacheHelper cache; // For the trigger
  cache.vecHash = vecHash;

  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vecROBData;
  const unsigned long int vecROBDataSize{robFrag.rod_ndata()};

  robFrag.rod_data(vecROBData);
  // Loop over header, hit element, flagged ABCD error, raw data, trailer words
 
  const uint8_t* vecROBData_8bits = reinterpret_cast<const uint8_t*>(vecROBData);
  const size_t total_bytes = vecROBDataSize * sizeof(uint32_t);

  for (size_t i = 0; i + 9 < total_bytes; i += 10) { // 10 bytes per package
    //Read Header
    bool hasError{false};
    uint16_t header = (vecROBData_8bits[i] << 8) | vecROBData_8bits[i + 1];
  
    uint8_t type      = (header >> 11) & 0x1F; 

    // Useful information
    // uint8_t l0tag     = (header >> 7)  & 0xF;
    // uint8_t bcid_low  = (header >> 4)  & 0x7;
    // uint8_t bcid_xor  = header & 0xF;

    if (type == 0x03) {  // PR Header
      bool breakNow{false};
      ATH_MSG_DEBUG("PR Packet Found");
      if (hasError) sc = StatusCode::RECOVERABLE;
      if (breakNow) break;
    } else {
    ATH_MSG_WARNING("Unexpected packet type (not PR): 0x" << std::hex << int(type));
    continue;
    }
    for (int j = 0; j < 4; ++j) {
      uint16_t cluster = (vecROBData_8bits[i + 2 + j * 2] << 8) | vecROBData_8bits[i + 3 + j * 2];
      if (cluster == 0x7FFF){
        ATH_MSG_DEBUG("Cluster [" << j << "] empty ");
      }
    }
  }

  // Create the last RDO of the last link of the event
  if (data.isStripValid()) {
     if (not data.isSaved(false) and data.isOldStripValid()) {
        const int rdoMade{makeRDO(false, data, cache, dataItemsPool)};
        if (rdoMade == -1) {
           sc = StatusCode::RECOVERABLE;
           ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
        }
        else {
           data.setSaved(false, rdoMade);
        }
     }
  }

  for (auto& [hash, rdoColl] : data.rdoCollMap) {
    if (rdoColl==nullptr) continue; // nullptr means the collection is already filled.

    if (rdoColl->empty()) { // Empty collection is not filled.
      rdoColl.reset();
      errs.removeIfEmpty(hash); // To get the same result as before. Not sure whether we need this.
      continue;
    }

    ATH_CHECK(data.writeHandleMap[hash].addOrDelete(std::move(rdoColl)));
  }

  if (sc.isFailure()) ATH_MSG_DEBUG("One or more ByteStream errors found ");
  return sc;
}

// makeRDO method

int ITkStripsRodDecoder::makeRDO(const bool isOld,
                            SharedData& data,
                            CacheHelper& cache,
                            DataPool<SCT3_RawData>* dataItemsPool) const
{
  // If the link is already decoded, RDO will not be created.
  SCT_RDO_Collection* rdoColl{data.rdoCollMap[data.linkIDHash].get()};
  if (rdoColl==nullptr) return 0;

  int strip{isOld ? data.oldStrip : data.strip};
  if (((strip & 0x7F) + (data.groupSize-1) >= N_STRIPS_PER_CHIP) or (strip<0) or (strip>=N_STRIPS_PER_SIDE)) {
    ATH_MSG_WARNING("Cluster with " << data.groupSize << " strips, starting at strip " << strip
                    << " in collection " << data.linkIDHash << " out of range. Will not make RDO");
    return -1;
  }

  // This option is for the trigger, if there is a vecHash* given, test it!
  if (cache.vecHash) {
    if (data.linkIDHash == cache.skipHash) {
      ATH_MSG_VERBOSE("Collection for Hash not to be decoded, skip");
      return 0;
    }
    else if (data.linkIDHash != cache.lastHash) {
      cache.lastHash = data.linkIDHash;
      // Maybe the new hash is not in the list, so test it
      std::vector<IdentifierHash>::const_iterator hashIDIterator{find(cache.vecHash->begin(),
                                                                      cache.vecHash->end(),
                                                                      data.linkIDHash)};
      if (hashIDIterator == cache.vecHash->end()) {
        ATH_MSG_VERBOSE("Collection for Hash not to be decoded, skip");
        // Remember this one, so that we do not find(...) forever
        cache.skipHash = data.linkIDHash;
        return 0;
      }
    }
  }

  // See if strips go from 0 to N_STRIPS_PER_SIDE-1(=767) or vice versa
  if (m_swapPhiReadoutDirection[data.linkIDHash]) {
    strip = N_STRIPS_PER_SIDE-1 - strip;
    strip = strip-(data.groupSize-1);
  }

  // Get identifier from the hash, this is not nice
  const Identifier digitID{m_itkStripsID->strip_id(data.collID, strip)};
  if (not m_itkStripsID->is_sct(digitID)) {
    ATH_MSG_WARNING("Cluster with invalid Identifier. Will not make RDO");
    return -1;
  }

  const unsigned int rawDataWord{static_cast<unsigned int>(data.groupSize | (strip << 11) | (data.timeBin <<22) | (data.errors << 25))};

  ATH_MSG_DEBUG("Output Raw Data " << std::hex << " Coll " << data.collID.getString()
                << ":-> " << m_itkStripsID->print_to_string(digitID) << std::dec);

  // Now the Collection is there for sure. Create RDO and push it into Collection.
  m_nRDOs++;

  if(dataItemsPool){
    SCT3_RawData* sct_rdo = dataItemsPool->nextElementPtr();
    (*sct_rdo) = SCT3_RawData(digitID, rawDataWord, &(data.errorHit));
    rdoColl->push_back(sct_rdo);
  }else{
    rdoColl->push_back(std::make_unique<SCT3_RawData>(digitID, rawDataWord, &(data.errorHit)));
  }
  return 1;
}

// addRODError method

StatusCode ITkStripsRodDecoder::addRODError(uint32_t /*rodID*/, SCT_ByteStreamErrors::ErrorType /*error*/,
                                       SCT_RodDecoderErrorsHelper& /*errs*/,
                                       const std::unordered_set<IdentifierHash>* /*foundHashes*/) const
{
  return StatusCode::SUCCESS;
}

// addSingleError method

StatusCode ITkStripsRodDecoder::addSingleError(const IdentifierHash& /*hashID*/,
                                          SCT_ByteStreamErrors::ErrorType /*error*/,
                                          SCT_RodDecoderErrorsHelper& /*errs*/) const
{
  return StatusCode::SUCCESS;
}

StatusCode ITkStripsRodDecoder::processHeader(const uint16_t inData,
                                         const uint32_t robID,
                                         SharedData& data,
                                         SCT_RDO_Container& rdoIDCont,
                                         DataPool<SCT3_RawData>* dataItemsPool,
                                         CacheHelper& cache,
                                         SCT_RodDecoderErrorsHelper& errs,
                                         bool& hasError,
                                         bool& breakNow,
                                         const EventContext& ctx) const
{
  StatusCode sc{StatusCode::SUCCESS};

  data.foundHeader = true;
  m_headNumber++;

  // Create the last RDO of the previous link if any
  if (data.isStripValid()) {
     if (not data.isSaved(false) and data.isOldStripValid()) {

        const int rdoMade{makeRDO(false, data, cache, dataItemsPool)};
        if (rdoMade == -1) {
           hasError = true;
           ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
        }
        else {
           data.setSaved(false, rdoMade);
        }
     }
  }

  // Everything is set to default for a new hunt of RDO
  data.reset();

  // Link Number (or stream) in the ROD fragment
  const int rodlinkNumber{static_cast<int>(inData & 0x7F)};

  // This is the real calculation for the offline
  data.linkNumber = (((rodlinkNumber >>4)&0x7)*12+(rodlinkNumber &0xF));
  const uint32_t onlineID{(robID & 0xFFFFFF) | (data.linkNumber << 24)};
  IdentifierHash hash;
  if ((onlineID ==0) or (data.linkNumber > 95)) {
    ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::ByteStreamParseError, errs));
    hasError = true;
    ATH_MSG_DEBUG("Header: xxx Link number out of range (skipping following data)"
                  << std::dec << data.linkNumber);
    breakNow = true;
    return sc;
  }
  else {
    hash = m_cabling->getHashFromOnlineId(onlineID, ctx);
    if (hash.is_valid()) {
       data.setCollection(m_itkStripsID, hash, rdoIDCont, dataItemsPool, errs);
    }
    else {
       std::stringstream msg;
       msg <<std::hex << onlineID;
       ATH_MSG_WARNING("Rob fragment (rob=" << robID << ") with invalid onlineID  " << msg.str() << " -> " << hash  << ".");
    }
  }
  // Look for masked off links - bit 7
  if ((inData >> 7) & 0x1) {
    ATH_MSG_DEBUG("Masked link " << onlineID << " " << data.linkIDHash);
    ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::MaskedLink, errs));
    hasError = true;
  }
  if (inData & 0x800) {
    ATH_MSG_DEBUG("    Header: xxx TimeOut Error " << data.linkIDHash);
    m_headErrorTimeout++;
    ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::TimeOutError, errs));
    hasError = true;
  }
  
  if (inData & 0x1000) {
    ATH_MSG_DEBUG("    Header: xxx Preamble Error " << data.linkIDHash);
    m_headErrorPreamble++;
    ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::PreambleError, errs));
    hasError = true;
  }

  if (inData & 0x400) {
    ATH_MSG_DEBUG("    Header: xxx LVL1 ID Error " << data.linkIDHash);
    m_headErrorLvl1ID++;
    ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::LVL1IDError, errs));
    hasError = true;
  }
  
  if (inData & 0x200) {
    ATH_MSG_DEBUG("    Header: xxx BCID Error " << data.linkIDHash);
    m_headErrorBCID++;
    ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::BCIDError, errs));
    hasError = true;
  }
  
  if ((inData & 0xF) > 11) {
    ATH_MSG_DEBUG("    Header: xxx Error in formatter " << data.linkIDHash);
    m_headErrorFormatter++;
    ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::FormatterError, errs));
    hasError = true;
  }
  if (!hasError and not hash.is_valid())  {
    std::stringstream msg;
    msg <<std::hex << onlineID;
    ATH_MSG_WARNING("Rob fragment (rob=" << robID << ") with invalid onlineID  " << msg.str() << " -> " << hash  << ".");
    hasError = true;
  }

  data.condensedMode = static_cast<bool>(inData & 0x100);

  return sc;
}

StatusCode ITkStripsRodDecoder::processRawData(const uint16_t inData,
                                          const uint32_t robID,
                                          SharedData& data,
                                          SCT_RodDecoderErrorsHelper& errs,
                                          bool& hasError) const
{
  StatusCode sc{StatusCode::SUCCESS};

  if (not data.foundHeader) {
    ATH_MSG_WARNING(" Missing link header in ROD " << std::hex << robID << std::dec);
    data.foundMissingLinkHeaderError = true;
    m_numMissingLinkHeader++;
    hasError = true;
    return sc;
  }

  ATH_MSG_DEBUG(" xxx Raw Data Mode " << std::hex << inData << std::dec << ": Config Data Mode ");
  // Too many errors in the BS for the ROD to decode the data
  m_configDataBit++;
  ATH_CHECK(addSingleError(data.linkIDHash, SCT_ByteStreamErrors::RawError, errs));
  hasError = true;

  return sc;
}

