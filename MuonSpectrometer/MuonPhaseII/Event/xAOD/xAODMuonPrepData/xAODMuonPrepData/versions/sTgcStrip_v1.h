/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCSTRIP_V1_H
#define XAODMUONPREPDATA_VERSION_STGCSTRIP_V1_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"

#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include "xAODMeasurementBase/versions/UncalibratedMeasurement_v1.h"

namespace xAOD {


class sTgcStrip_v1 : public UncalibratedMeasurement_v1 {

 public:
  /// Default constructor
  sTgcStrip_v1() = default;
  /// Virtual destructor
  virtual ~sTgcStrip_v1() = default;

  /// Returns the type of the Tgc strip as a simple enumeration
  xAOD::UncalibMeasType type() const override final {
    return xAOD::UncalibMeasType::sTgcStripType;
  }
  unsigned int numDimensions() const override final { return 1; }

  /** @brief Returns the hash of the measurement channel (tube (x) layer) */
  IdentifierHash measurementHash() const;

  /** @brief Returns the bcBitMap of this PRD
    bit2 for Previous BC, bit1 for Current BC, bit0 for Next BC */
  uint16_t bcBitMap() const;

  /** @brief Returns the time  (ns). */
  uint16_t time() const;

  /** @brief Returns the charge*/
  uint32_t charge() const;

  void setBcBitMap(uint16_t);
  void setTime(uint16_t value);
  void setCharge(uint32_t value);

  
  /** @brief set the pointer to the MdtReadoutElement */
  void setReadoutElement(const MuonGMR4::sTgcReadoutElement* readoutEle);
  /** @brief Retrieve the associated MdtReadoutElement. 
      If the element has not been set before, it's tried to load it on the fly. 
      Exceptions are thrown if that fails as well */
  const MuonGMR4::sTgcReadoutElement* readoutElement() const;

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
DATAVECTOR_BASE(xAOD::sTgcStrip_v1, xAOD::UncalibratedMeasurement_v1);
#endif