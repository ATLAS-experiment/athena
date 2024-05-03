/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
 
#ifndef ITkStripsRawDataByteStreamCnv_ITkStripsRodEncoder_h
#define ITkStripsRawDataByteStreamCnv_ITkStripsRodEncoder_h

#include "AthenaBaseComps/AthAlgTool.h"
#include "ITkStripsByteStreamCnv/IITkStripsRodEncoder.h"

#include "InDetRawData/SCT_RDO_Container.h"
#include "SCT_Cabling/ISCT_CablingTool.h"
#include "GaudiKernel/ToolHandle.h"

#include <set>


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
 
  /** Constructor */
  ITkStripsRodEncoder(const std::string& type, const std::string& name, const IInterface* parent);

  /** Destructor */
  virtual ~ITkStripsRodEncoder() = default;

  /** Initialize */
  virtual StatusCode initialize() override;

  /** Finalize */
  virtual StatusCode finalize() override;

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
  void encodeData(const std::vector<int>& vecTimeBins, std::vector<uint16_t>& vec16Words, 
                  const SCT_RDORawData* rdo, const int& groupSize, const int& strip) const;
  
  /**
   * @brief Method to pack vector of 16 bit words intto a vector of 32 bit words.
   *
   * Method us used by private method encodeData(...).
   *
   * @param vec16Words Vector containing 16 bit words.
   * @param vec32Words Vector for 32 bit words to be packed.
   */
  void packFragments(std::vector<uint16_t>& vec16Words, std::vector<uint32_t>& vec32Words) const;

  /**
   * @breif Method to set pairs of 16 bit words to a 32 bit word.
   *
   * Function used by the packFragments(...) method.
   *
   * @param arr16Words Pointer to array containing a pair of 16 bit words.
   * @param position Pointer to an array that gives the 32 bit starting positions of the 16 bit words and corresponding to arr16Words.
   * @param numWords Number of word to be set to a 32 bit word.
   */
  uint32_t set32Bits(const unsigned short int* arr16Words, 
                     const unsigned short int* position, 
                     const unsigned short int& numWords) const;

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

  /** Get the 16-bit word for a header for a hit. */
  uint16_t getHeaderUsingRDO(const SCT_RDORawData* rdo) const;
  
  /** Get the 16-bit word for a header for a link with a ByteStream error. */
  uint16_t getHeaderUsingHash(const IdentifierHash& linkHash, const int& errorWord) const;
  
  /** Get the 16-bit word for a trailer, with or without ByteStream errors. */
  uint16_t getTrailer(const int& errorWord) const;

 
  
  /** Providing mappings of online and offline identifiers and also serial numbers. */
  ToolHandle<ISCT_CablingTool> m_cabling{this, 
                                         "SCT_CablingTool", 
                                         "SCT_CablingTool", 
                                         "Tool to retrieve ITkStrips Cabling"};

  /** Identifier helper class for the ITkStrips subdetector that creates compact Identifier objects and 
      IdentifierHash or hash IDs. Also allows decoding of these IDs. */
  const SCT_ID* m_itkStripsID{nullptr};

  /** Example Boolean used to determine decoding mode, maybe unused finally */
  BooleanProperty m_condensed{this, "CondensedMode", false, "Condensed mode (true) or Expanded mode (false)"};

  /** Swap Module identifier, set by SCTRawContByteStreamTool. */
  std::set<Identifier> m_swapModuleID{};
};

#endif // SCT_RAWDATABYTESTREAMCNV_SCT_RODENCODER_H