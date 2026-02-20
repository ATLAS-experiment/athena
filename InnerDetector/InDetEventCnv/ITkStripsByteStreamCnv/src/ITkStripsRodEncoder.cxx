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
  chunk(std::bitset<128> b){
    static constexpr std::bitset<128> mask64(~0ULL);
    static constexpr unsigned int shift{n * 64};
    return ((b >>shift) & mask64).to_ullong();
  }
  std::vector<uint8_t>
  split32bitWord(uint32_t word){
    std::vector<uint8_t> out;
    out.push_back(word>>24);
    out.push_back(word>>16);
    out.push_back(word>>8);
    out.push_back(word);
    return out;
  }
  int
  rodLinkFromOnlineID(const ITkStripOnlineId onlineID){
    const uint32_t fibre{onlineID.fibre()};
    const int formatter{static_cast<int>((fibre/12) & 0x7)};
    const int linkNum{static_cast<int>((fibre - (formatter*12)) & 0xF)};
    const int rodLink{(formatter << 4) | linkNum};
    return rodLink;
  }
  uint32_t
  hccKey(int barrelEC, uint8_t side, uint8_t disk, uint8_t phi_mod, int eta_mod, uint8_t eta_group){
    
    uint32_t hcckey = 0, mask = 1;
    uint8_t  sideAC = 0;
    bool hasTwoHccs = false;
    
    //Set barrel/endcap/side bit
    if(barrelEC == 0){
      hcckey = mask << 23;
      if(eta_mod>0) sideAC=1;
    } else if(barrelEC > 0) sideAC=1;

    //Set Side-A/C bit
    if(sideAC==1) hcckey = hcckey | (mask << 20);
    
    //Set the layer/disk bits
    hcckey = hcckey | (disk<<17);
    
    //Set the inner/outer side bit
    if(side==1) hcckey = hcckey | (mask << 16);

    //Set the petal bit (0 for barrel)
    if(barrelEC!=0) hcckey = hcckey | (mask << 15);
    
    //Set the phi module number
    hcckey = hcckey | (phi_mod<<8);

    //Set the HCC number    
    if(disk<2 && barrelEC==0) {
      hasTwoHccs = true;
    }else if(eta_group!=2 && barrelEC!=0){
      hasTwoHccs = true;
    }
        
    eta_mod = std::abs(eta_mod);
    //This is to get the right Hcc number (1 or 2) per module algorithmically could try to improve this in the future
    if ( hasTwoHccs && ( ((eta_mod-1)/2.0 >= 2*eta_group+1  && barrelEC==0) || ((((eta_mod/2.0 >= 2*eta_group+1) && eta_mod<=7) || ((eta_mod/2.0 >= 2*eta_group) && (eta_mod>=10 && eta_mod<=13))) && barrelEC!=0))){
       hcckey = hcckey | (mask << 7);
    }
    
    barrelEC = (barrelEC == 0) ? 1 : 0;
      
    //Set the eta group number    
    hcckey = hcckey | (eta_group);
    
    return hcckey << 8;
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

  std::unordered_map<uint32_t, std::vector<std::bitset<256>>> allStripData;
  
  for (const auto& rdo : vecRDOs) {
    int barrelEC = getBarrelEC(rdo);
    int eta_mod = getEtaModule(rdo);    
    uint8_t side = getSide(rdo);    
    uint8_t disk = getDiskLayer(rdo);
    uint8_t phi_mod = getPhiModule(rdo);
    uint16_t strip_max = getStripMax(rdo);
    
    if (strip_max == 0xFFFF){
      //To do: Implement workaround
      const Identifier rdoID{rdo->identify()};
      ATH_MSG_WARNING("Negative maximum number of strips found " << m_itkStripsID->strip_max(rdoID));
      continue; 
    }

    uint8_t sideAC = 0;
    if((barrelEC == 0 && eta_mod > 0) || barrelEC > 0) sideAC = 1;

    uint8_t eta_group=0;

    if (barrelEC == 0) {      
      if(disk < 2){
        eta_group = static_cast<uint8_t>(std::floor((std::abs(eta_mod)-1) / 4));
      }else{
        eta_group = static_cast<uint8_t>(std::floor((std::abs(eta_mod)-1) / 2));     
      }
    }else {
      if(eta_mod>=0 && eta_mod<=9) eta_group = static_cast<uint8_t>(std::floor(eta_mod / 4));
      else if(eta_mod>=10 && eta_mod<=13) eta_group = static_cast<uint8_t>(std::floor((eta_mod+2)/4));
      else if(eta_mod>13) eta_group = static_cast<uint8_t>(std::floor((eta_mod-6)/2));      
    }
    

    uint8_t chips_per_module = (strip_max + 1) / 128;
    uint32_t key = hccKey(barrelEC, side, disk, phi_mod, eta_mod, eta_group);
    
    ATH_MSG_DEBUG("barrel: "<< barrelEC<<" sideAC: " << (uint32_t)sideAC << " disk: "<<(uint32_t)disk << " side: " << (uint32_t)side <<" phi_mod: "<<(uint32_t)phi_mod << " eta_mod: " << eta_mod << " eta group: "<<(uint32_t)eta_group << " chips per module: " << (uint32_t)chips_per_module);
    
    ATH_MSG_DEBUG("key: " << std::bitset<32>(key));
    auto& StripData = allStripData[key];

    if (StripData.empty()) {
      StripData.resize(chips_per_module);
    }
     
    //Populate the bitset for each chip with active strips
    int strip = getStrip(rdo);
    int chip = static_cast<int>(std::floor(strip / 128));
    int strip_position = strip % 128;
    int strip_logical_channel = 2*strip_position + (eta_mod & 1);

    ATH_MSG_DEBUG("strip N: "<< strip << " chip n: " << chip << " Strip position: " << strip_position << " Strip position logical: " << strip_logical_channel);
    StripData[chip].set(strip_logical_channel);
  }

  std::vector<uint8_t> vec8Data;
  uint32_t vectorSize = 0;
  std::vector<uint16_t> clusters;
  
  ATH_MSG_DEBUG("All strip data size: " << allStripData.size());

  //Iterate over processed strip data and find clusters
  for (const auto& [key, StripData] : allStripData) {
    
    uint16_t ichannel = 0;
    uint16_t size = 1;
    int ptype = 1;
    bool keyRecorded = false;
    clusters.clear();
    
    ATH_MSG_DEBUG("key is: " << std::bitset<32>(key) << " StripData size: " << StripData.size());
    
    for (size_t i = 0; i < StripData.size(); ++i) {
      
      std::bitset<256> hits = StripData[i];
      
      if(hits==0){
        ++ichannel;
        continue;
      }

      //Use clusterFinder to extract clusters from the bitset
      clusters = clusterFinder(hits);
      
      if(clusters.empty()){
        ++ichannel;
        continue;
      } 
      uint32_t hccKey = (keyRecorded) ? 0 : key;        
      encodeData(clusters, ichannel, vec8Data, ptype, m_l0tag , m_bcid, hccKey, size);
      keyRecorded = true;      
      ++ichannel;++size;
    }
    vec8Data.push_back(0xed);
    vec8Data.push_back(0x6f);
    ATH_MSG_DEBUG("Add 16-0s: " << size % 2 << " " << vec8Data.size());

    if(size % 2 == 0){
      vec8Data.push_back(0);
      vec8Data.push_back(0);
    }
    uint32_t packetLenght = (vec8Data.size()-vectorSize)+4;
    ATH_MSG_DEBUG("Packet Lenght: " << (uint32_t)packetLenght << " " << std::bitset<32>(packetLenght));

    std::vector<uint8_t> pktlenght = split32bitWord(packetLenght);

    ATH_MSG_DEBUG("PktLenght: " << pktlenght.size() << " vec8Data size: " << vec8Data.size());

    uint32_t offset = vectorSize + 4;
    vec8Data.insert(vec8Data.begin()+offset, pktlenght.begin(), pktlenght.end());
    vectorSize   = vec8Data.size();
    
    ATH_MSG_DEBUG("vec8Data size: " << vec8Data.size() << " size: " << size-1);
  }

  //Update BCID and L0Tag counters
  m_bcid = (m_bcid + 1) & 0x7F;
  m_l0tag = (m_l0tag + 1) & 0x7F;

  ATH_MSG_DEBUG("vec8Data size: " << vec8Data.size());

  packFragments(vec8Data,vec32Data);
  for(auto &word: vec32Data){
    ATH_MSG_DEBUG("32-bit word: " << std::bitset<32>(word));
  }
  return;
}

void
ITkStripsRodEncoder::encodeData(const std::vector<uint16_t>& clusters, const uint16_t ichannel,std::vector<uint8_t>& data_encode,
                                int ptyp, uint8_t l0tag, uint8_t bc_count, uint32_t hccKey, uint16_t& size) const {


  if(hccKey!=0){
    ATH_MSG_DEBUG("hccKey: " << std::bitset<32>(hccKey));
    ATH_MSG_DEBUG("hccKey 8-bit word-4: "<<std::bitset<8>(hccKey));        
    ATH_MSG_DEBUG("hccKey 8-bit word-3: "<<std::bitset<8>(hccKey>>24));
    ATH_MSG_DEBUG("hccKey 8-bit word-2: "<<std::bitset<8>(hccKey>>16));    
    ATH_MSG_DEBUG("hccKey 8-bit word-1: "<<std::bitset<8>(hccKey>>8));

    data_encode.push_back(hccKey);    
    data_encode.push_back(hccKey>>24);
    data_encode.push_back(hccKey>>16);
    data_encode.push_back(hccKey>>8);

    uint16_t header = getHeaderPhysicsPacket(ptyp, l0tag, bc_count);
    ATH_MSG_DEBUG("header: " << std::bitset<16>(header));
    data_encode.push_back((header>>8) & 0xff);
    data_encode.push_back(header & 0xff);
  }  

  for(size_t idx=0;auto &cluster : clusters){
    if(cluster == 0x3fe) continue;
    if(idx!=0) size++;
    
    // cluster bits:
    // "0" + 4-bit channel number + 11-bit cluster dropping the last cluster bit    
    uint16_t clusterbits = ((ichannel & 0xf) << 11) | (cluster & 0x7ff);
    ATH_MSG_DEBUG("Clusters: " << idx << ": " << std::bitset<16>(clusterbits) << " size: " << size << " ichannel: " << ichannel);    
    data_encode.push_back((clusterbits>>8) & 0xff);
    data_encode.push_back(clusterbits & 0xff);
    idx++;    
  }
  
  return;
}


std::vector<uint16_t>
ITkStripsRodEncoder::clusterFinder(const std::bitset<256>& inputData, const uint8_t maxCluster) const {

  std::vector<uint16_t> clusters;
  
  // Split into far (odd) and near (even) strips
  std::bitset<128> dataEven;
  std::bitset<128> dataOdd;
  for(int i=0; i<128; ++i){
    dataEven[i] = inputData[2*i];
    dataOdd[i] = inputData[2*i+1];
  }

  // Split the 128-bit Even and Odd data into four 64-bit chunks for processing
  uint64_t d0l = chunk<0>(dataEven);
  uint64_t d0h = chunk<1>(dataEven);

  uint64_t d1l = chunk<0>(dataOdd);
  uint64_t d1h = chunk<1>(dataOdd);

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
  uint8_t hit_mask = 0;

  if (hits_low64){
    hit_addr = __builtin_ctzll(hits_low64);
  } else if (hits_high64){
    hit_addr = __builtin_ctzll(hits_high64) + 64;
  }

  hit_mask = getBit_128b(hit_addr+1, hits_high64, hits_low64) << 2
    | getBit_128b(hit_addr+2, hits_high64, hits_low64) << 1
    | getBit_128b(hit_addr+3, hits_high64, hits_low64);

  for (int i=0; i<4; ++i)
    setBit_128b(hit_addr+i, 0, hits_high64, hits_low64);

  if (hit_addr == 128) {
    return 0x3ff;
  } else {
    hit_addr += isSecondRow<<7;
    return hit_addr << 3 | hit_mask;
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
ITkStripsRodEncoder::getSide(const SCT_RDORawData* rdo) const {
  const Identifier rdoID{rdo->identify()};
  int itkSide{m_itkStripsID->side(rdoID)};
  return itkSide;
}

int
ITkStripsRodEncoder::getBarrelEC(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  return m_itkStripsID->barrel_ec(rdoID);
}

uint8_t
ITkStripsRodEncoder::getDiskLayer(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  return m_itkStripsID->layer_disk(rdoID);
}

uint8_t
ITkStripsRodEncoder::getPhiModule(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  return m_itkStripsID->phi_module(rdoID);
}

int
ITkStripsRodEncoder::getEtaModule(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  return m_itkStripsID->eta_module(rdoID);
}

uint16_t
ITkStripsRodEncoder::getStripMax(const SCT_RDORawData* rdo) const{
  const Identifier rdoID{rdo->identify()};
  if(m_itkStripsID->strip_max(rdoID)<0) return 0xFFFF;
  return m_itkStripsID->strip_max(rdoID);
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
