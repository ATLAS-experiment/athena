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
					                                    SCT_RDO_Container& rdoIDCont,
                                                                       IDCInDetBSErrContainer& errorsIDC,
                                                                       DataPool<SCT3_RawData>* dataItemsPool,
                                                                           const EventContext& ctx,
                                                            const std::vector<IdentifierHash>* vecHash) const
{
  SCT_RodDecoderErrorsHelper errs = errorsIDC; // on destruction will fill the IDC

  const uint32_t robID=robFrag.rod_source_id();

  StatusCode sc = StatusCode::SUCCESS;

  SharedData data;
  data.reset();

  CacheHelper cache; // For the trigger
  cache.vecHash = vecHash;  

  OFFLINE_FRAGMENTS_NAMESPACE::PointerType vecROBData;
  const unsigned long int vecROBDataSize{robFrag.rod_ndata()};
  if (vecROBDataSize >   robFrag.payload_size_word()) {
     ATH_MSG_WARNING("The ROB data does not seem to fit in the payload. Rejecting fragment (ndata size  " << vecROBDataSize << " !< payload size " << robFrag.payload_size_word()
                     << " header size: " <<  robFrag.rod_header_size_word()
                     << " trailer size: " << robFrag.rod_trailer_size_word()
                     << " fragment size: " << robFrag.rod_fragment_size_word()
                     << ")");
     return StatusCode::RECOVERABLE;
  }  
  robFrag.rod_data(vecROBData);
  
  ATH_MSG_DEBUG("vecROBDataSize: " << vecROBDataSize);
  
  // Loop over header, hit element, flagged ABCD error, raw data, trailer words
  for (uint32_t i=0; i<vecROBDataSize; i++) {
    // The data is 16-bits wide packed to a 32-bit word (rob_it1). So we unpack it here.
    ATH_MSG_DEBUG("ROB: "<< std::bitset<32>(vecROBData[i]));
  }
  
  const uint8_t* vecROBData_8bits = reinterpret_cast<const uint8_t*>(vecROBData);
  const size_t total_bytes = vecROBDataSize * sizeof(uint32_t);

  uint32_t nclusters = 0;
  uint8_t isHCCHeader   = 0;
  int nPacket       = 0;
  uint16_t hccword1 = 0;
  uint8_t word8     = 0;
  uint8_t HccHeadFound = 0;
  uint16_t packetSize = 0;
  bool noClusterTag = false;
  
  for (size_t i=0; i < total_bytes; i+=2) {
    //Read Header
    bool hasError   = false;
    bool breakNow   = false;
    uint16_t psize=packetSize/4;    
    uint16_t word16 = (vecROBData_8bits[i] << 8) | vecROBData_8bits[i + 1];
    ATH_MSG_DEBUG(" 16-bit word: " << std::bitset<16>(word16) << " " << (uint32_t)i << " " << total_bytes );
    
    if(i==total_bytes-2) ATH_MSG_DEBUG("nClusters found (this): " << (uint32_t)nclusters);
    
    ATH_MSG_DEBUG("Check: " << nPacket << " Packets: " << packetSize/4 << " isHCCHeader: " << (uint32_t)isHCCHeader << " " << nPacket);
    
    if(psize != 0 && nPacket != 0){
      ATH_MSG_DEBUG("check: " << (nPacket+1) % (packetSize/2));
      if((nPacket+1) % (packetSize/2)==0) nPacket=-1;
    }

    if(word16 == 0 && noClusterTag){
      ATH_MSG_DEBUG("Skip empty end of packet 16-bit word: ");
      nPacket++;
    }else if((vecROBData_8bits[i] == 0 || isHCCHeader == 1) && HccHeadFound == 0 && nPacket<2){
      //HCC header found
      ATH_MSG_DEBUG("HCC header found, will decode the next three 8-bit words: " <<(uint32_t)isHCCHeader);
      if(isHCCHeader == 0){
        hccword1 = (vecROBData_8bits[i + 1] << 8) | vecROBData_8bits[i+2];
        ATH_MSG_DEBUG("HCC header: "<<std::bitset<16>(hccword1));        
      } else if(isHCCHeader == 1){
        word8 = vecROBData_8bits[i + 1];
        ATH_MSG_DEBUG("HCC header: "<<std::bitset<8>(word8));        
      }      
      ATH_CHECK(processHccHeader(hccword1, word8, isHCCHeader, robID, data, rdoIDCont, dataItemsPool, cache, errs, hasError, breakNow,ctx));
      if(isHCCHeader==0) HccHeadFound = 1;
      nPacket++;
      noClusterTag=false;
    }else if((HccHeadFound==1 && vecROBData_8bits[i] == 0) || HccHeadFound==2){      
      if(HccHeadFound==1) HccHeadFound=2;
      else if(HccHeadFound==2){
        packetSize = word16;
        ATH_MSG_DEBUG("Packet size is: " << packetSize );
        HccHeadFound=0;
      }
      nPacket++;
    }else if(((vecROBData_8bits[i] & 0xF8) == 0x18) && (((nPacket+1) % (packetSize/2)) == 5)){
      nPacket++;      
      ATH_MSG_DEBUG("Header found: " << std::bitset<16>(word16));
      nclusters=0;      
      ATH_CHECK(processHeader(word16, robID, data, rdoIDCont, dataItemsPool, cache, errs, hasError, breakNow,ctx));
    }else{
      nPacket++;
      if(word16 == 0xed6f){
        ATH_MSG_DEBUG("No more clusters found");
        noClusterTag=true;
        continue;
      }else{
        nclusters+=1;
        ATH_CHECK(processHits(word16, robID, data, rdoIDCont, dataItemsPool, cache, errs, hasError, ctx));
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


StatusCode ITkStripsRodDecoder::processHccHeader(const uint16_t hccword1,
                                                 const uint8_t word8,
                                                 uint8_t &isHCCHeader,
                                                 const uint32_t /*robID*/,
                                                 SharedData& /*data*/,
                                                 SCT_RDO_Container& /*rdoIDCont*/,
                                                 DataPool<SCT3_RawData>* /*dataItemsPool*/,
                                                 CacheHelper& /*cache*/,
                                                 SCT_RodDecoderErrorsHelper& /*errs*/,
                                                 bool& /*hasError*/,
                                                 bool& /*breakNow*/,
                                                 const EventContext& /*ctx*/) const
{

  StatusCode sc{StatusCode::SUCCESS};
  
  if(isHCCHeader == 1){
    isHCCHeader = 0;
    ATH_MSG_DEBUG("Decoding HCC bits");
    /*24 bits in total
    bits from 24-8 are in hccword1
    Last 8bits are in word8
    */
    uint8_t barrel = ((hccword1 >> 8) & 0x80);
    ATH_MSG_DEBUG("is barrel: " << std::bitset<16>(hccword1 >> 8) << " " << std::bitset<8>(barrel));
    uint8_t side   = ((hccword1 >> 5) & 0x80);
    ATH_MSG_DEBUG("side     : " << std::bitset<16>(hccword1 >> 5) << " " << std::bitset<8>(side));
    uint8_t disk   = ((hccword1 >> 9) & 0x7);
    ATH_MSG_DEBUG("disk     : " << std::bitset<16>((hccword1 >> 9) & 0x7) << " " << std::bitset<8>(disk));
    uint8_t inout  = ((hccword1 >> 8) & 0x1);
    ATH_MSG_DEBUG("inout    : " << std::bitset<16>((hccword1 >> 8) & 0x1) << " " << std::bitset<8>(inout));
    uint8_t petal  = ((hccword1 >> 7) & 0x1);
    ATH_MSG_DEBUG("petal    : " << std::bitset<16>((hccword1 >> 7) & 0x1) << " " << std::bitset<8>(petal));        
    uint8_t phimod = hccword1 & 0x7F;
    ATH_MSG_DEBUG("phimod   : " << std::bitset<16>(hccword1 & 0x7F) << " " << std::bitset<8>(phimod));
    uint8_t hccnum = word8 & 0x80;
    ATH_MSG_DEBUG("hccnum   : " << std::bitset<16>(word8 & 0x80) << " " << std::bitset<8>(hccnum));
    uint8_t etamod = word8 & 0x3F;
    ATH_MSG_DEBUG("etamod   : " << std::bitset<16>(word8 & 0x3F) << " " << std::bitset<8>(etamod));        

    bool isbarrel = (barrel != 0x0);
    bool issideA  = (side   != 0x0);
    bool isinout  = (inout  == 0x1);
    bool ispetal  = (petal  == 0x1);
    uint8_t hccN  = (hccnum == 0x80) ? 2 : 1;


    ATH_MSG_DEBUG("isBarrel: " << isbarrel << " isSideA: " << issideA << " disk: " << (uint32_t)disk);
    ATH_MSG_DEBUG("isInOut: " << isinout << " isPetal: " << ispetal << " phimod: " << (uint32_t)phimod);
    ATH_MSG_DEBUG("HCCNum: " << (uint32_t)hccN << " etamod: " << (uint32_t)etamod);

    ATH_MSG_DEBUG("hccheader " << isbarrel << " " << issideA << " " << (uint32_t)disk << " " << (uint32_t)inout << " " << (uint32_t)phimod << " " << (uint32_t)etamod << " " << (uint32_t)hccN );
  }  
  else isHCCHeader++;

  return sc;
}


StatusCode ITkStripsRodDecoder::processHeader(const uint16_t word16,
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
  StatusCode sc = StatusCode::SUCCESS;

  data.foundHeader = true;
  m_headNumber++;

  uint8_t type      = (word16 >> 11) & 0x1F;
  // Useful information
  uint8_t l0tag     = (word16 >> 7)  & 0xF;
  uint8_t bcid_low  = (word16 >> 4)  & 0x7;
  uint8_t bcid_xor  = word16 & 0xF;

  ATH_MSG_DEBUG("l0tag: " << (uint32_t)l0tag << " bcid_low: " << (uint32_t)bcid_low << " bcid_xor: " << (uint32_t)bcid_xor << " type: " << (uint32_t)type << " word16: " << std::bitset<16>(word16));
  
  if (type == 0x03) {  // PR Header
    bool breakNow{false};
    ATH_MSG_DEBUG("PR Packet Found");
    if (hasError) sc = StatusCode::RECOVERABLE;
    if (breakNow) return sc;
  } else {
    ATH_MSG_WARNING("Unexpected packet type (not PR): 0x" << std::hex << int(type));
    return sc;
  }        

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
  const int rodlinkNumber{static_cast<int>(word16 & 0x7F)};

  // This is the real calculation for the offline
  data.linkNumber = (((rodlinkNumber >>4)&0x7)*12+(rodlinkNumber &0xF));
  const uint32_t onlineID{(robID & 0xFFFFFF) | (data.linkNumber << 24)};
  ATH_MSG_DEBUG("OnlineID: " << (uint32_t)onlineID << " Link number: " << data.linkNumber);
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
       ATH_MSG_DEBUG("setCollectionCall");
       data.setCollection(m_itkStripsID, hash, rdoIDCont, dataItemsPool, errs);
    }
    else {
       std::stringstream msg;
       msg <<std::hex << onlineID;
       ATH_MSG_WARNING("Rob fragment (rob=" << robID << ") with invalid onlineID  " << msg.str() << " -> " << hash  << ".");
    }
  }
  if (!hasError and not hash.is_valid())  {
    std::stringstream msg;
    msg <<std::hex << onlineID;
    ATH_MSG_WARNING("Rob fragment (rob=" << robID << ") with invalid onlineID  " << msg.str() << " -> " << hash  << ".");
    hasError = true;
  }

  data.condensedMode = static_cast<bool>(word16 & 0x100);

  return sc;
}

StatusCode ITkStripsRodDecoder::processHits(const uint16_t word16,
                                            const uint32_t /*robID*/,
                                            SharedData& /*data*/,
                                            SCT_RDO_Container& /*rdoIDCont*/,
                                            DataPool<SCT3_RawData>* /*dataItemsPool*/,
                                            CacheHelper& /*cache*/,
                                            SCT_RodDecoderErrorsHelper& /*errs*/,
                                            bool& /*hasError*/,
                                            const EventContext& /*ctx*/) const
{
  StatusCode sc = StatusCode::SUCCESS;

  uint8_t stripNumber = 0;
  uint8_t address  = (word16 >> 3) & 0xFF;

  ATH_MSG_DEBUG("Cluster address: " << std::bitset<8>(address) << " " << (uint32_t)address);

  stripNumber = (address >= 128) ? 2*(address-128)+1 : 2*address;          

  //Get the next three strips in the cluster
  uint8_t firsthit = (word16 & 0x4);
  uint8_t secondhit   = (word16 & 0x2);
  uint8_t thirdhit   = (word16 & 0x1);        
  ATH_MSG_DEBUG("First hit: " << std::bitset<16>(firsthit));
  ATH_MSG_DEBUG("Second hit: " << std::bitset<16>(secondhit));
  ATH_MSG_DEBUG("Third hit: " << std::bitset<16>(thirdhit));
  uint8_t stripN1 = 0,stripN2 = 0,stripN3=0;
  uint8_t addr    = 0;
  if(firsthit != 0x0) {
    addr = address+1;
    stripN1 = (addr >= 128) ? 2*(addr-128)+1 : 2*addr;
    ATH_MSG_DEBUG("Hits: " << (uint32_t)(stripN1));          
  }
  if(secondhit != 0x0) {
    addr = address+2;
    stripN2 = (addr >= 128) ? 2*(addr-128)+1 : 2*addr;          
    ATH_MSG_DEBUG("Hits: " << (uint32_t)(stripN2));
  }
  if(thirdhit != 0x0) {
    addr = address+3;
    stripN3 = (addr >= 128) ? 2*(addr-128)+1 : 2*addr;          
    ATH_MSG_DEBUG("Hits: " << (uint32_t)(stripN3));
  }

  uint8_t nchip    = (word16 >> 11) & 0xF;

  ATH_MSG_DEBUG("Chip number: " << (int)nchip << " Strip Number: " << (uint32_t)stripNumber);
  ATH_MSG_DEBUG("abcclusters " << (uint32_t)nchip << " " << (uint32_t)stripNumber << " " << (uint32_t)(stripN1) << " " << (uint32_t)(stripN2) << " " << (uint32_t)(stripN3)); 
  
  return sc;
}

