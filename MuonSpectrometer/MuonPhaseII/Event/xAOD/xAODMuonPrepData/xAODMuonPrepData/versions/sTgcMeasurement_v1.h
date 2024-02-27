/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCMEASUREMENT_V1_H
#define XAODMUONPREPDATA_VERSION_STGCMEASUREMENT_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"
#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonPrepRawData/sTgcPrepData.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {


class sTgcMeasurement_v1 : public UncalibratedMeasurement_v1 {

 public:
  /// Default constructor
  sTgcMeasurement_v1() = default;
  /// Virtual destructor
  virtual ~sTgcMeasurement_v1() = default;

  /// Returns the type of the Tgc strip as a simple enumeration
  xAOD::UncalibMeasType type() const override final {
    return xAOD::UncalibMeasType::sTgcStripType;
  }
  using sTgcChannelTypes = sTgcIdHelper::sTgcChannelTypes;
  /// Returns the channel type of the measurement (Pad/Wire/Strip)
  virtual sTgcChannelTypes channelType() const = 0;
  /// Pad measurements have 2 dimensions. Strips & Wires have only 1
  unsigned int numDimensions() const override final {       
      return channelType() == sTgcChannelTypes::Pad ? 2 : 1; 
  }
  /** @brief Returns the hash of the measurement channel w.r.t ReadoutElement*/
  IdentifierHash measurementHash() const;

  /** @brief Which algorithm produced the Measurement object*/
  using Author = Muon::sTgcPrepData::Author;
  Author author() const;

  /** @brief In which gasGap is the Measurement */
  uint8_t gasGap() const;
  /** @brief Channel number of the Measurement */
  uint16_t channelNumber() const;
 
  /** @brief: Collected charge on the wire */
  int charge() const;
  /** @brief: Calibrated time of the wire measurement */
  short int time() const;

  /** @brief Retrieve the associated sTgcReadoutElement. 
      If the element has not been set before, it's tried to load it on the fly. 
      Exceptions are thrown if that fails as well */
  const MuonGMR4::sTgcReadoutElement* readoutElement() const;

  
  /** @brief set the pointer to the sTgcReadoutElement */
  void setReadoutElement(const MuonGMR4::sTgcReadoutElement* readoutEle);
  /** @brief Set the author of the producing algorithm */
  void setAuthor(Author a);
  /** @brief Set the associated gas gap of the measurement */
  void setGasGap(uint8_t gap);
  /** @brief Set the channel number of the measurement */
  void setChannelNumber(uint16_t channel);
  /** @brief: Set the calibrated time of the wire measurement */
  void setTime(short int t);
  /** @brief: Set the collected charge on the wire */
  void setCharge(int q);

    private:
#ifdef __CLING__
    /// Down cast the memory of the readoutElement cache if the object is stored to disk 
    ///  to arrive at the same memory layout between Athena & CLING
    char m_readoutEle[sizeof(CxxUtils::CachedValue<const MuonGMR4::sTgcReadoutElement*>)]{};
#else
    CxxUtils::CachedValue<const MuonGMR4::sTgcReadoutElement*> m_readoutEle{};
#endif
};

}  // namespace xAOD

#include "AthContainers/DataVector.h"
DATAVECTOR_BASE(xAOD::sTgcMeasurement_v1, xAOD::UncalibratedMeasurement_v1);
#endif