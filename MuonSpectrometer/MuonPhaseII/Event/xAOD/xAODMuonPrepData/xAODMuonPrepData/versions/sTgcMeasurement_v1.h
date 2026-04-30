/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONPREPDATA_VERSION_STGCMEASUREMENT_V1_H
#define XAODMUONPREPDATA_VERSION_STGCMEASUREMENT_V1_H



#include "xAODMuonPrepData/versions/MuonMeasurement_v1.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"

#include "MuonIdHelpers/sTgcIdHelper.h"
#include "MuonPrepRawData/sTgcPrepData.h"



namespace xAOD {


class sTgcMeasurement_v1 : public MuonMeasurement_v1 {

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
  unsigned numDimensions() const override final {       
      return channelType() == sTgcChannelTypes::Pad ? 2 : 1; 
  }

  std::uint8_t measuresPhi() const override final { return channelType() != sTgcChannelTypes::Strip; }
  /** @brief Returns the hash of the measurement channel w.r.t ReadoutElement*/
  IdentifierHash measurementHash() const override final;
  /** @brief Returns the hash of the associated gasGap layer */
  IdentifierHash layerHash() const override final;
  /** @brief Returns the local measurement position as 3-vector */
  Amg::Vector3D localMeasurementPos() const override final;
  /** @brief Which algorithm produced the Measurement object*/
  using Author = ::Muon::sTgcPrepData::Author;
  Author author() const;

  /** @brief In which gasGap is the Measurement */
  std::uint8_t gasGap() const;
  /** @brief Channel number of the Measurement */
  std::uint16_t channelNumber() const;
 
  /** @brief: Collected charge on the wire */
  int charge() const;
  /** @brief: Calibrated time of the wire measurement */
  short int time() const;

  /** @brief Retrieve the associated sTgcReadoutElement. 
      If the element has not been set before, it's tried to load it on the fly. 
      Exceptions are thrown if that fails as well */
  const MuonGMR4::sTgcReadoutElement* readoutElement() const override final;

  
  /** @brief set the pointer to the sTgcReadoutElement */
  void setReadoutElement(const MuonGMR4::sTgcReadoutElement* readoutEle);
  /** @brief Set the author of the producing algorithm */
  void setAuthor(Author a);
  /** @brief Set the associated gas gap of the measurement */
  void setGasGap(std::uint8_t gap);
  /** @brief Set the channel number of the measurement */
  void setChannelNumber(std::uint16_t channel);
  /** @brief: Set the calibrated time of the wire measurement */
  void setTime(short int t);
  /** @brief: Set the collected charge on the wire */
  void setCharge(int q);

};

}  // namespace xAOD

#endif