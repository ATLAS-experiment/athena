/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
  template<unsigned int n>
  unsigned long long
  chunk(std::bitset<256> b){
    static constexpr std::bitset<256> mask64(~0ULL);
    static constexpr unsigned int shift{n * 64};
    return ((b >>shift) & mask64).to_ullong();
  }
  int
  rodLinkFromOnlineID(const ITkStripOnlineId onlineID){
    const uint32_t fibre{onlineID.fibre()};
    const int formatter{static_cast<int>((fibre/12) & 0x7)};
    const int linkNum{static_cast<int>((fibre - (formatter*12)) & 0xF)};
    const int rodLink{(formatter << 4) | linkNum};
    return rodLink;
  }
  uint64_t
  geometryKey(int barrel, int disk, int phi_mod, int eta_group){
    return (static_cast<uint64_t>(barrel) << 48) |
           (static_cast<uint64_t>(disk) << 32) |
           (static_cast<uint64_t>(phi_mod) << 16) |
           static_cast<uint64_t>(eta_group);
  }

} // End of anonymous namespace

// Initialize
StatusCode
ITkStripsRodEncoder::initialize() {
  ATH_MSG_DEBUG("ITkStripsRodEncoder::initialize()");

  // Retrieve cabling tool
  ATH_CHECK(m_cabling.retrieve());
  ATH_MSG_DEBUG("Retrieved tool " << m_cabling);
  ATH_CHECK(detStore()->retrieve(m_itkStripsID, "SCT_ID"));
  const InDetDD::SCT_DetectorManager* itkStripsDetManager{nullptr};
  ATH_CHECK(detStore()->retrieve(itkStripsDetManager, "ITkStrip"));
  const InDetDD::SiDetectorElementCollection* sctDetElementColl{itkStripsDetManager->getDetectorElementCollection()};
  for (const InDetDD::SiDetectorElement* sctDetElement : *sctDetElementColl) {
    if (sctDetElement->swapPhiReadoutDirection()) {
      m_swapModuleID.insert(sctDetElement->identify());
    }
  }
  ATH_MSG_DEBUG("Initialization was successful");
  return StatusCode::SUCCESS;
}



void
ITkStripsRodEncoder::fillROD(std::vector<uint32_t>& vec32Data, const uint32_t& /*robID*/,
                             const std::vector<const SCT_RDORawData*>& vecRDOs) const {
  //code to be filled here

  std::unordered_map<uint64_t, std::vector<std::bitset<256>>> allStripData;
  
  for (const auto& rdo : vecRDOs) {
    int barrel = getBarrelEC(rdo);
    int disk = getDiskLayer(rdo);
    int phi_mod = getPhiModule(rdo);
    int eta_mod = getEtaModule(rdo);
    int strip_max = getStripMax(rdo);
    
    if (strip_max < 0){
      continue; 
    }
    int eta_group;

    if (barrel == 0) {
      eta_group = static_cast<int>(std::floor((eta_mod+1) / 2)); //To work with barrel
    }else {
      eta_group = static_cast<int>(std::floor(eta_mod / 2)); //To work with endcap
    }

    int chips_per_module = (strip_max + 1) / 128;
    uint64_t key = geometryKey(barrel, disk, phi_mod, eta_group);
    auto& StripData = allStripData[key];

    if (StripData.empty()) {
      StripData.resize(chips_per_module);
    }
     
    //Populate the bitset for each chip with active strips
    int strip = getStrip(rdo);
    int chip = static_cast<int>(std::floor(strip / 128));
    int strip_position = strip % 128;
    int strip_logical_channel = 2*strip_position + (eta_mod & 1);
    StripData[chip].set(strip_logical_channel);
  }

  std::vector<uint8_t> vec8Data;
  vec8Data.reserve(10);

  //Iterate over processed strip data and find clusters
  for (const auto& [key, StripData] : allStripData) {
    int ptype = 1;
    for (size_t i = 0; i < StripData.size(); ++i) {
      if (StripData[i].any()) {
              std::bitset<256> hits = StripData[i];
        //Use clusterFinder to extract clusters from the bitset
        std::vector<uint16_t> clusters = clusterFinder(hits);
        encodeData(clusters, vec8Data, ptype, m_l0tag , m_bcid);
      }
    }
    //Update BCID and L0Tag counters
    m_bcid = (m_bcid + 1) & 0x7F;
    m_l0tag = (m_l0tag + 1) & 0x7F;
  }
  packFragments(vec8Data,vec32Data);
  return;
}

void
ITkStripsRodEncoder::encodeData(const std::vector<uint16_t>& clusters, std::vector<uint8_t>& data_encode,
                                int ptyp, uint8_t l0tag, uint8_t bc_count) const {

  uint16_t header = getHeaderPhysicsPacket(ptyp, l0tag, bc_count);

  data_encode.push_back((header>>8) & 0xff);
  data_encode.push_back(header & 0xff);

  size_t cluster_count = 0;
  for ( uint16_t cluster : clusters) {
    if (cluster_count == 4){
      // Max number of clusters per package 4 
      break;
    }

    uint16_t clusterbits = cluster & 0x7ff;
    data_encode.push_back((clusterbits>>8) & 0xff);
    data_encode.push_back(clusterbits & 0xff);
    cluster_count++;
  }
  
  while (cluster_count < 4) {
        data_encode.push_back(0x7F);  // Cluster empty (0x7FF in 12 bits)
        data_encode.push_back(0xFF);
        cluster_count++;
    }

  return;
}

std::vector<uint16_t>
ITkStripsRodEncoder::clusterFinder(const std::bitset<256>& inputData, const uint8_t maxCluster) const {

  std::vector<uint16_t> clusters;

  // Split the 256-bit input data into four 64-bit chunks for processing
  uint64_t d0l = chunk<0>(inputData);
  uint64_t d0h = chunk<1>(inputData);

  uint64_t d1l = chunk<2>(inputData);
  uint64_t d1h = chunk<3>(inputData);

  while (d0l or d0h or d1l or d1h){
    if (clusters.size() > maxCluster) break;

    uint16_t cluster1 = clusterFinder_sub(d1h, d1l, true);
    if (cluster1 != 0x3ff) // No cluster was found
      clusters.push_back(cluster1);

    if (clusters.size() > maxCluster) break;

    uint16_t cluster0 = clusterFinder_sub(d0h, d0l, false);
    if (cluster0 != 0x3ff) // No cluster was found
      clusters.push_back(cluster0);
  }

  if (clusters.empty()) {
    clusters.push_back(0x3fe);
  } else {
    clusters.back() |=1 << 11;
  }

  return clusters;
}

inline bool
ITkStripsRodEncoder::getBit_128b(uint8_t bit_addr, uint64_t data_high64, uint64_t data_low64) const {
  if (bit_addr > 127) return false;

  return bit_addr<64 ? data_low64>>bit_addr & 1 : data_high64>>(bit_addr-64) & 1;
}

inline void
ITkStripsRodEncoder::setBit_128b(uint8_t bit_addr, bool value,  uint64_t& data_high64, uint64_t& data_low64) const {
  if (bit_addr < 64) {
    data_low64 = (data_low64 & ~(1ULL << bit_addr)) | ((uint64_t)value << bit_addr);
  } else if (bit_addr < 128) {
    data_high64 =
      (data_high64 & ~(1ULL << (bit_addr-64))) | ((uint64_t)value << (bit_addr-64));
  }
}


uint16_t
ITkStripsRodEncoder::clusterFinder_sub(uint64_t& hits_high64, uint64_t& hits_low64, bool isSecondRow) const {
  uint8_t hit_addr = 128;
  uint8_t hitpat_next3 = 0;

  if (hits_low64){
    hit_addr = __builtin_ctzll(hits_low64);
  } else if (hits_high64){
    hit_addr = __builtin_ctzll(hits_high64) + 64;
  }

  hitpat_next3 = getBit_128b(hit_addr+1, hits_high64, hits_low64) << 2
    | getBit_128b(hit_addr+2, hits_high64, hits_low64) << 1
    | getBit_128b(hit_addr+3, hits_high64, hits_low64);

  for (int i=0; i<4; ++i)
    setBit_128b(hit_addr+i, 0, hits_high64, hits_low64);

  if (hit_addr == 128) {
    return 0x3ff;
  } else {
    hit_addr += isSecondRow<<7;
    return hit_addr << 3 | hitpat_next3;
  }
}

void 
ITkStripsRodEncoder::packFragments(std::vector<uint8_t>& vec8Words, std::vector<uint32_t>& vec32Words) const {
  int num8Words{static_cast<int>(vec8Words.size())};
  if (num8Words % 4 != 0) {
    // Just add additional 8-bit words to make the size a multiple of 4
    while (num8Words % 4 != 0) {
      vec8Words.push_back(0x40); // Padding byte
      num8Words++;
    }
  }  
  // Now merge 4 consecutive 8-bit words into 32-bit words
  const unsigned short int numWords{4};
  const unsigned short int position[numWords]{0, 8, 16, 24};
  unsigned short int arr8Words[numWords]{0, 0, 0, 0};
  
  for (int i{0}; i<num8Words; i += numWords) {
    for (int j{0}; j<numWords; j++) {
      arr8Words[j] = vec8Words[i + j];
    }
    const uint32_t uint32Word{set32Bits(arr8Words, position, numWords)};
    vec32Words.push_back(uint32Word);
  }
  return;
}

// set32Bits function
// This function combines four 8-bit words into a 32-bit word
uint32_t ITkStripsRodEncoder::set32Bits(const unsigned short int* arr8Words, const unsigned short int* position, const unsigned short int& numWords) const 
{
  uint32_t uint32Word{0};
  uint32_t pos{0};
  uint32_t uint8Word{0}; 
  for (uint16_t i{0}; i<numWords; i++) {
    uint8Word = static_cast<uint32_t>(*(arr8Words + i));
    pos = static_cast<uint32_t>(*(position + i));
    uint32Word |= (uint8Word << pos); // Shift the 8-bit word to its correct position and merge
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

int
ITkStripsRodEncoder::getBarrelEC(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  int itkBarrel{m_itkStripsID->barrel_ec(rdoID)};
  return itkBarrel;
}

int
ITkStripsRodEncoder::getDiskLayer(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  int itkDiskLayer{m_itkStripsID->layer_disk(rdoID)};
  return itkDiskLayer;
}

int
ITkStripsRodEncoder::getPhiModule(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  int itkPhiModule{m_itkStripsID->phi_module(rdoID)};
  return itkPhiModule;
}

int
ITkStripsRodEncoder::getEtaModule(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  int itkEtaModule{m_itkStripsID->eta_module(rdoID)};
  return itkEtaModule;
}

int
ITkStripsRodEncoder::getStripMax(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  int itkStripMax{m_itkStripsID->strip_max(rdoID)};
  return itkStripMax;
}

bool
ITkStripsRodEncoder::getParity_8bits(uint8_t val) const{
  val ^= val >> 4;
  val ^= val >> 2;
  val ^= val >> 1;
  return val&1;
}

uint16_t
ITkStripsRodEncoder::getHeaderPhysicsPacket(int typ, uint8_t l0tag, uint8_t bc_cout) const {
  uint8_t bcid_low = bc_cout & 0x7; // BCID[2:0]
  bool bc_parity = getParity_8bits(bc_cout);
  //TYPE (4 bits) + FlagBit (1 bit) + L0tag (7 bits) + BCID (3 bits) + Parity (1 bit)
  const uint16_t Header{static_cast<uint16_t>(((uint8_t)typ << 12) | (0x1 << 11) | (l0tag & 0x7f) << 4 | (bcid_low) << 1 | bc_parity)};
  return Header;
  
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
