/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
 
#ifndef ITkStripsRawDataByteStreamCnv_ITkStripsRodEncoder_h
#define ITkStripsRawDataByteStreamCnv_ITkStripsRodEncoder_h

#include "AthenaBaseComps/AthAlgTool.h"
#include "ITkStripsByteStreamCnv/IITkStripsRodEncoder.h"

#include "ITkStripCabling/IITkStripCablingTool.h"
#include "GaudiKernel/ToolHandle.h"

#include <set>
#include <atomic>

class SCT_ID;
class SCT_RDORawData;
class Identifier;
class IdentifierHash;

/** 
 * @class ITkStripsRodEncoder
 *
 * @brief Athena Algorithm Tool that provides conversion from ITkStrips RDO to ROD format Bytestream.
 *
 *
 */
class 
ITkStripsRodEncoder : public extends<AthAlgTool, IITkStripsRodEncoder>{ 
 public:
 
  using base_class::base_class;
  
  /** Destructor */
  virtual ~ITkStripsRodEncoder() = default;

  /** Initialize */
  virtual StatusCode initialize() override;

  /**
   * @brief Main Convert method
   *
   * Converts SCT RDO to a vector of 32 bit words. Starts by retrieving and collecting
   * errors, then loops through RDO's and decode them to 16 bit words, and then
   * finally it packs the 16 bit word into 32 bit words vector.
   *
   * @param vec32Data Vector of 32 bit words to be filled with encoded RDOs from the SCT.
   * @param robID ID of the current readout buffer (ROB).
   * @param vecRDOs Vector containing the RDOs to be coverted to vector of 32 bit words.
   */
  virtual void fillROD(std::vector<uint32_t>& vec32Data, const uint32_t& robID, 
                       const std::vector<const SCT_RDORawData*>& vecRDOs) const override;
  

 private:
  
  /**
   * @brief Method to encode RDO data to vector of 16 bin words.
   *
   * Methods used by main convert methods fillROD(...).
   *
   * @param vecTimeBins Vector of time bins for RDOs.
   * @param vec16Words Vector of 16 bit words to filled from encoded RDO data.
   * @param rdo RDO raw data object to be encoded.
   * @param groupSize Group size info from the RDO.
   * @param strip Strip number info from the RDO.
   */
  
  mutable std::atomic<uint8_t> m_bcid = 0;
  mutable std::atomic<uint8_t> m_l0tag = 0;
  
  void encodeData(const std::vector<uint16_t>& clusters, const uint16_t ichannel, std::vector<uint8_t>& data_encode,
                                int typ, uint8_t l0tag, uint8_t bc_count) const;
  
  /**
   * @brief Method to pack vector of 8 bit words intto a vector of 32 bit words.
   *
   * Method us used by private method encodeData(...).
   *
   * @param vec8Words Vector containing 8 bit words.
   * @param vec32Words Vector for 32 bit words to be packed.
   */
  void packFragments(std::vector<uint8_t>& vec8Words, std::vector<uint32_t>& vec32Words) const;

  /**
   * @breif Method to set pairs of 8 bit words to a 32 bit word.
   *
   * Function used by the packFragments(...) method.
   *
   * @param arr8Words Pointer to array containing a pair of 8 bit words.
   * @param position Pointer to an array that gives the 32 bit starting positions of the 8 bit words and corresponding to arr8Words.
   * @param numWords Number of word to be set to a 32 bit word.
   */
  
  std::vector<uint16_t> clusterFinder(const std::bitset<256>& inputData, const uint8_t maxCluster=63) const;
  uint16_t clusterFinder_sub(uint64_t& hits_high64, uint64_t& hits_low64, bool isSecondRow) const;
  
  uint32_t set32Bits(const unsigned short int* arr8Words, 
                     const unsigned short int* position, 
                     const unsigned short int& numWords) const;

  bool getBit_128b(uint8_t bit_addr, uint64_t data_high64, uint64_t data_low64) const;
  void setBit_128b(uint8_t bit_addr, bool value, uint64_t& data_high64, uint64_t& data_low64) const;
  bool getParity_8bits(uint8_t val) const;
  
  /** Get the barrel/endcape info from the RDO. */
  int getBarrelEC(const SCT_RDORawData* rdo) const;

  /** Get disk/layer info from the RDO. */
  int getDiskLayer(const SCT_RDORawData* rdo) const;

  /** Get the phi value info from the RDO. */
  int getPhiModule(const SCT_RDORawData* rdo) const;

  /** Get the eta value info from the RDO. */
  int getEtaModule(const SCT_RDORawData* rdo) const;

  /** Get the maxumum strip value info from the RDO. */
  int getStripMax(const SCT_RDORawData* rdo) const;

  /** Get the side info from the RDO. */
  int side(const SCT_RDORawData* rdo) const;
  
  /** Get the time bin info from the RDO. */
  int getTimeBin(const SCT_RDORawData* rdo) const;
  
  /** Get the strip number info from the RDO. */
  int getStrip(const SCT_RDORawData* rdo) const;
 
  /** Get the offline Identifier from the RDO.  */
  Identifier offlineID(const SCT_RDORawData* rdo) const;

  /** Get the online Identifier from the RDO. */
  uint32_t onlineID(const SCT_RDORawData* rdo) const;

  /** Get the ROD link number info in the RDO header data. */
  int getRODLink(const SCT_RDORawData* rdo) const;

  /** Get the 16-bit word for a header with Type (PR or LP), L0Tag event and BCID */
  uint16_t getHeaderPhysicsPacket(int typ, uint8_t l0tag, uint8_t bc_count) const;

  /** Get the 16-bit word for a header for a hit. */
  uint16_t getHeaderUsingRDO(const SCT_RDORawData* rdo) const;
  
  /** Get the 16-bit word for a header for a link with a ByteStream error. */
  uint16_t getHeaderUsingHash(const IdentifierHash& linkHash, const int& errorWord) const;
  
  /** Get the 16-bit word for a trailer, with or without ByteStream errors. */
  uint16_t getTrailer(const int& errorWord) const;
  
  /** Providing mappings of online and offline identifiers and also serial numbers. */
  ToolHandle<IITkStripCablingTool> m_cabling{this, 
                                         "ITkStripCablingTool", 
                                         "ITkStripCablingTool", 
                                         "Tool to retrieve ITkStrips Cabling"};

  /** Identifier helper class for the ITkStrips subdetector that creates compact Identifier objects and 
      IdentifierHash or hash IDs. Also allows decoding of these IDs. */
  const SCT_ID* m_itkStripsID{nullptr};

  /** Example Boolean used to determine decoding mode, maybe unused finally */
  BooleanProperty m_condensed{this, "CondensedMode", false, "Condensed mode (true) or Expanded mode (false)"};

  /** Swap Module identifier, set by SCTRawContByteStreamTool. */
  std::set<Identifier> m_swapModuleID{};
};

#endif // ITKSTRIP_RAWDATABYTESTREAMCNV_ITKSTRIP_RODENCODER_H
