/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RawData/HGTD_ALTIROC_RDO.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 *
 * @brief Second version of RDO with ALTIROC output
*/

#ifndef HGTD_RAWDATA_HGTD_ALTIROC_RDO_H
#define HGTD_RAWDATA_HGTD_ALTIROC_RDO_H

#include "Identifier/Identifiable.h"

#include "Identifier/Identifier.h"

#include <cstdint>

class HGTD_ALTIROC_RDO : public Identifiable {

public:
  /**
   * @brief Default constructor should NOT be used, needed for pool I/O.
   */
  HGTD_ALTIROC_RDO() = default;
  HGTD_ALTIROC_RDO(const HGTD_ALTIROC_RDO&) = default;
  HGTD_ALTIROC_RDO(HGTD_ALTIROC_RDO&&) = default;
  HGTD_ALTIROC_RDO& operator=(const HGTD_ALTIROC_RDO&) = default;
  HGTD_ALTIROC_RDO& operator=(HGTD_ALTIROC_RDO&&) = default;

  // Destructor:
  virtual ~HGTD_ALTIROC_RDO() = default;

  /**
   * @brief   Constructor with parameters
   *
   * @param [in] rdo_id Offline compact identifier of the readout channel.
   * @param [in] word ALTIROC output word
   */
  HGTD_ALTIROC_RDO(const Identifier rdo_id, const uint64_t word);

  virtual Identifier identify() const;
  virtual uint16_t getToT() const;
  virtual uint8_t getToA() const;
  virtual uint16_t getBCID() const;
  virtual uint8_t getL1ID() const;
  virtual uint8_t getCRC() const;
  virtual uint64_t getWord() const;

private:
  /** @brief Offline ID of the readout channel.
   */
  Identifier m_rdo_id;

  /** @brief ALTIROC RAW output.
   */
  uint64_t m_word{};
};

inline Identifier HGTD_ALTIROC_RDO::identify() const { return m_rdo_id; }

// decode BCID information (10 bits)
inline uint16_t HGTD_ALTIROC_RDO::getBCID() const { return ((m_word>>30) & 0x3FF); }

// decode L1ID information (6 bits)
inline uint8_t HGTD_ALTIROC_RDO::getL1ID() const { return ((m_word>>24) & 0x3F); }

// decode TOT information (9 bits)
inline uint16_t HGTD_ALTIROC_RDO::getToT() const { return ((m_word>>15) & 0x1FF); }

// decode TOA information (7 bits)
inline uint8_t HGTD_ALTIROC_RDO::getToA() const { return ((m_word>>8) & 0x7F); }

// decode CRC information (8 bits)
inline uint8_t HGTD_ALTIROC_RDO::getCRC() const { return ((m_word) & 0xFF); }

// Return raw altiroc word
inline uint64_t HGTD_ALTIROC_RDO::getWord() const { return m_word; }

#endif // HGTD_RAWDATA_HGTD_ALTIROC_RDORAWDATA_H
